/*
 * Import poi data from osm data sets
 *
 * Copyright (C) 2016  Filip Krump <filip.krumpe@fmi.uni-stuttgart.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "osminputhelper.h"

#include <algorithm>

#include "osmpbf/filter.h"
#include "osmpbf/inode.h"
#include "osmpbf/irelation.h"
#include "osmpbf/iway.h"
#include "osmpbf/parsehelpers.h"

// ---- BoundingBox
void
osm_input::OsmInputHelper::BoundingBox::adapt(
  const osm_input::OsmPoi::Position& aPos)
{
  mMinLat = std::min(mMinLat, aPos.mLat);
  mMaxLat = std::max(mMinLat, aPos.mLat);

  mMinLon = std::min(mMinLon, aPos.mLon);
  mMaxLon = std::max(mMinLon, aPos.mLon);
}

void
osm_input::OsmInputHelper::BoundingBox::adapt(int32_t aLat, int32_t aLon)
{
  mMinLat = std::min(mMinLat, aLat);
  mMaxLat = std::max(mMinLat, aLat);

  mMinLon = std::min(mMinLon, aLon);
  mMaxLon = std::max(mMinLon, aLon);
}

// ---- OsmInputHelper
typedef std::vector<const osm_input::OsmPoi*> PoiSet;

// osm.pbf parsing
namespace osm_parsing {

struct SharedPOISet
{
  std::mutex lock;

  PoiSet* pois;

  SharedPOISet()
    : pois(new PoiSet()){};
};

struct BlockParser
{
  SharedPOISet* globalPois;
  PoiSet localPois;
  bool mIncludeSettlements;
  bool mIncludeGeneralPois;

  osmpbf::RCFilterPtr filter;

  BlockParser(SharedPOISet* aPoiGlobal, bool aSettlements, bool aGeneralPois)
    : globalPois(aPoiGlobal)
    , mIncludeSettlements(aSettlements)
    , mIncludeGeneralPois(aGeneralPois){};

  BlockParser(const BlockParser& aOther)
    : globalPois(aOther.globalPois)
    , mIncludeSettlements(aOther.mIncludeSettlements)
    , mIncludeGeneralPois(aOther.mIncludeGeneralPois){};

  void operator()(osmpbf::PrimitiveBlockInputAdaptor(&pbi))
  {
    // Filter to get all nodes that have an name and (amenity or place) tag
    osmpbf::OrTagFilter* orFilter = new osmpbf::OrTagFilter();
    orFilter->addChild(new osmpbf::KeyOnlyTagFilter("place"));
    orFilter->addChild(new osmpbf::KeyOnlyTagFilter("amenity"));

    osmpbf::AndTagFilter* andFilter = new osmpbf::AndTagFilter();
    andFilter->addChild(new osmpbf::KeyOnlyTagFilter("name"));
    andFilter->addChild(orFilter);

    filter.reset(andFilter);
    filter->assignInputAdaptor(&pbi);

    if (!(filter->rebuildCache())) {
      return;
    }

    localPois.clear();

    if (pbi.nodesSize() > 0) {

      for (osmpbf::INodeStream node = pbi.getNodeStream(); !node.isNull();
           node.next()) {
        if (filter->matches(node)) {
          osm_input::OsmPoi::Position pos((int32_t)node.lati(),
                                          (int32_t)node.lati());
          int64_t id = node.id();

          osm_input::OsmPoi* poi = new osm_input::OsmPoi(id, pos);
          for (uint32_t i = 0, s = node.tagsSize(); i < s; ++i) {
            std::string key = node.key(i);
            std::string value = node.value(i);
            if (key == "amenity" || key == "place" || key == "name" ||
                key == "population") {
              poi->addTag(key, value);
            }
          }

          poi->computeType(true);

          localPois.push_back(poi);
        }
      }
    }

    std::unique_lock<std::mutex> lck(globalPois->lock);
    globalPois->pois->insert(globalPois->pois->end(), localPois.begin(),
                             localPois.end());
  }
};
}

osm_input::OsmInputHelper::OsmInputHelper(std::string aPbfPath)
  : mPbfPath(std::string(aPbfPath))
{
}

osm_input::OsmInputHelper::~OsmInputHelper()
{
}

PoiSet
osm_input::OsmInputHelper::importPoiData(bool aIncludeSettlements,
                                         bool aIncludeGeneral)
{
  mPois.clear();

  printf("Trying to parse infile %s\n", mPbfPath.c_str());

  osmpbf::OSMFileIn osmFile(mPbfPath.c_str(), false);

  if (!osmFile.open()) {
    printf("Failed to open infile %s\n", mPbfPath.c_str());

    return PoiSet();
  }

  osm_parsing::SharedPOISet pois;
  uint32_t threadCount = 4;   // use 4 threads, usually 4 are more than enough
  uint32_t readBlobCount = 2; // parse 2 blocks at once
  bool threadPrivateProcessor = true; // set to true so that MyCounter is copied

  osmpbf::parseFileCPPThreads(
    osmFile,
    osm_parsing::BlockParser(&pois, aIncludeSettlements, aIncludeGeneral),
    threadCount, readBlobCount, threadPrivateProcessor);

  mPois.insert(mPois.end(), pois.pois->begin(), pois.pois->end());

  delete (pois.pois);

  return mPois;
}