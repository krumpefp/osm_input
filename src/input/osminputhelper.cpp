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
#include <assert.h>

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
osm_input::OsmInputHelper::BoundingBox::adapt(double aLat, double aLon)
{
  mMinLat = std::min(mMinLat, aLat);
  mMaxLat = std::max(mMinLat, aLat);

  mMinLon = std::min(mMinLon, aLon);
  mMaxLon = std::max(mMinLon, aLon);
}

// ---- OsmInputHelper
typedef std::vector<osm_input::OsmPoi> PoiSet;

// osm.pbf parsing
namespace osm_parsing {
typedef int64_t SegmentId;
typedef int64_t NodeId;
typedef osm_input::OsmPoi::Position Position;

std::vector<NodeId>
concatenateSegments(NodeId aConcatNode,
                    std::vector<NodeId>& aSeg1,
                    std::vector<NodeId>& aSeg2)
{
  std::vector<NodeId> result;
  result.reserve(aSeg1.size() + aSeg2.size());

  if (aSeg1.back() == aConcatNode) {
    result.insert(result.end(), aSeg1.begin(), aSeg1.end());
  } else if (aSeg1.front() == aConcatNode) {
    result.insert(result.end(), aSeg1.rbegin(), aSeg1.rend());
  } else {
    assert(false);
  }

  // take care: don't insert the concat node twice
  if (aSeg2.front() == aConcatNode) {
    result.insert(result.end(), ++aSeg2.begin(), aSeg2.end());
  } else if (aSeg2.back() == aConcatNode) {
    result.insert(result.end(), ++aSeg2.rbegin(), aSeg2.rend());
  } else {
    assert(false);
  }

  return result;
}

std::list<std::vector<NodeId>>
assemblePolygon(
  const std::vector<SegmentId>& aSegIds,
  const std::unordered_map<SegmentId, std::vector<NodeId>>& aSegments)
{
  std::list<std::vector<NodeId>> result;

  std::unordered_map<NodeId, std::list<SegmentId>> adjacent;

  if (aSegIds.size() == 1) {
    std::vector<NodeId> tmp;
    tmp.insert(tmp.begin(),
               aSegments.at(aSegIds.front()).begin(),
               aSegments.at(aSegIds.front()).end());

    // hack to overcome data problems if polygons are not closed ...
    if (tmp.front() != tmp.back()) {
      tmp.push_back(tmp.front());
    }

    result.push_back(tmp);
    return result;
  }

  for (const SegmentId& segId : aSegIds) {
    assert(aSegments.count(segId) > 0);
    auto nodes = aSegments.at(segId);

    auto it = adjacent.find(nodes.front());
    if (it == adjacent.end()) {
      adjacent.emplace(nodes.front(), std::list<NodeId>());
      it = adjacent.find(nodes.front());
    }
    it->second.push_back(segId);

    it = adjacent.find(nodes.back());
    if (it == adjacent.end()) {
      adjacent.emplace(nodes.back(), std::list<NodeId>());
      it = adjacent.find(nodes.back());
    }
    it->second.push_back(segId);
  }

  while (adjacent.size() > 0) {
    // start at an arbitrary node
    NodeId startNode = adjacent.begin()->first;
    if (adjacent.at(startNode).size() == 0) {
      adjacent.erase(startNode);
      continue;
    }

    if (adjacent.at(startNode).size() == 1)
      assert(adjacent.at(startNode).size() > 1);
    SegmentId currentSegment = adjacent.at(startNode).front();
    adjacent.at(startNode).pop_front();

    std::vector<NodeId> segment;
    const auto& seg = aSegments.at(currentSegment);
    if (seg.front() == startNode)
      segment.insert(segment.end(), seg.begin(), seg.end());
    else if (seg.back() == startNode)
      segment.insert(segment.end(), seg.rbegin(), seg.rend());
    else
      assert(false);
    NodeId currentNode = segment.back();
    adjacent.at(currentNode).remove(currentSegment);

    while (currentNode != startNode) {
      assert(adjacent.at(currentNode).size() > 0);
      currentSegment = adjacent.at(currentNode).front();
      adjacent.at(currentNode).pop_front();
      assert(aSegments.count(currentSegment) > 0);
      auto& seg = aSegments.at(currentSegment);
      if (seg.front() == currentNode) {
        segment.insert(segment.end(), ++seg.begin(), seg.end());
      } else if (seg.back() == currentNode) {
        segment.insert(segment.end(), ++seg.rbegin(), seg.rend());
      } else {
        assert(false);
      }
      currentNode = segment.back();

      std::size_t s = adjacent.at(currentNode).size();
      adjacent.at(currentNode).remove(currentSegment);
      // assert that exactly one element was removed from the adjacent's list
      assert(adjacent.at(currentNode).size() == s - 1);
    }

    result.push_back(segment);
  }

  return result;
};

struct AreaPoi
{
  int64_t mOsmId;
  const mapping_helper::MappingHelper::Level* mPoiLevel;

  std::vector<osm_input::Tag> mTags;
  std::vector<SegmentId> mOuter;
  std::vector<SegmentId> mInner;

  AreaPoi(int64_t aOsmId,
          const std::vector<osm_input::Tag>& aTags,
          const mapping_helper::MappingHelper& aMh,
          std::vector<SegmentId>& aOuterWays,
          std::vector<SegmentId>& aInnerWays)
    : mOsmId(aOsmId)
    , mPoiLevel(aMh.computeLevel(aTags))
    , mTags(aTags)
    , mOuter(aOuterWays)
    , mInner(aInnerWays){};

  bool getPoiInfo(std::unordered_map<SegmentId, std::vector<NodeId>>& aSegments,
                  std::unordered_map<NodeId, Position>& aNodes,
                  osm_input::OsmPoi*& aResult);
};

bool
AreaPoi::getPoiInfo(
  std::unordered_map<SegmentId, std::vector<NodeId>>& aSegments,
  std::unordered_map<NodeId, Position>& aNodes,
  osm_input::OsmPoi*& aResult)
{
  // TODO: Define - by bounding box? by area size? by maximum diameter?
  std::size_t count = 0;
  for (auto& seg : mOuter) {
    if (aSegments.count(seg) <= 0) {
      assert(false);
    }
    count += aSegments.at(seg).size();
  }
  if (count > 100) {
    return false;
  }

  // compute the centeroid of the polygone
  // compare https://en.wikipedia.org/wiki/Centroid#Centroid_of_polygon
  auto outerSegments = assemblePolygon(mOuter, aSegments);
  // TODO: Redefine. Using average point
  double sumLat = 0, sumLon = 0;
  count = 0;
  for (auto& outer : outerSegments) {
    for (NodeId node : outer) {
      assert(aNodes.count(node) > 0);
      Position p = aNodes.at(node);
      sumLat += p.getLatDegree();
      sumLon += p.getLonDegree();
      ++count;
    }
  }

  aResult = new osm_input::OsmPoi(
    mOsmId,
    osm_input::OsmPoi::Position(sumLat / (double)count, sumLon / (double)count),
    mTags,
    mPoiLevel);

  return true;
}

typedef std::vector<AreaPoi> AreaSet;

struct SharedAreaSet
{
  std::mutex lock;

  AreaSet* areas;

  SharedAreaSet()
    : areas(new AreaSet()){};
};

struct BlockParserAreaPoiInfo
{
  SharedAreaSet* globalAreas;
  AreaSet localAreas;
  const mapping_helper::MappingHelper& mMappingHelper;

  osmpbf::RCFilterPtr filter;

  BlockParserAreaPoiInfo(SharedAreaSet* aAreasGlobal,
                         const mapping_helper::MappingHelper& aMappingHelper)
    : globalAreas(aAreasGlobal)
    , mMappingHelper(aMappingHelper){};

  BlockParserAreaPoiInfo(const BlockParserAreaPoiInfo& aOther)
    : globalAreas(aOther.globalAreas)
    , mMappingHelper(aOther.mMappingHelper){};

  void operator()(osmpbf::PrimitiveBlockInputAdaptor(&pbi))
  {
    // Filter to get all nodes that have an name tag
    //     osmpbf::KeyOnlyTagFilter* nameFilter = new
    //     osmpbf::KeyOnlyTagFilter("name");
    //     filter.reset(nameFilter);

    // Filter to get all nodes that have an name and (amenity or place) tag
    //     osmpbf::OrTagFilter* orFilter = new osmpbf::OrTagFilter();
    //     orFilter->addChild(new osmpbf::KeyOnlyTagFilter("place"));
    //     orFilter->addChild(new osmpbf::KeyOnlyTagFilter("amenity"));
    //
    //     osmpbf::AndTagFilter* andFilter = new osmpbf::AndTagFilter();
    //     andFilter->addChild(new osmpbf::KeyOnlyTagFilter("name"));
    //     andFilter->addChild(orFilter);
    //     filter.reset(andFilter);

    //     Filter to get all relations that have an amenity, name or place tag
    osmpbf::OrTagFilter* orFilter = new osmpbf::OrTagFilter();
    orFilter->addChild(new osmpbf::KeyOnlyTagFilter("place"));
    orFilter->addChild(new osmpbf::KeyOnlyTagFilter("amenity"));
    orFilter->addChild(new osmpbf::KeyOnlyTagFilter("name"));

    //     Filter to get all relations of type multipolygon having a amenity,
    //     name or place tag
    osmpbf::AndTagFilter* andFilter = new osmpbf::AndTagFilter();
    andFilter->addChild(new osmpbf::KeyValueTagFilter("type", "multipolygon"));
    andFilter->addChild(orFilter);

    filter.reset(andFilter);

    filter->assignInputAdaptor(&pbi);

    if (!(filter->rebuildCache())) {
      return;
    }

    localAreas.clear();

    if (pbi.relationsSize() > 0) {

      for (osmpbf::IRelationStream rel = pbi.getRelationStream(); !rel.isNull();
           rel.next()) {
        if (filter->matches(rel)) {
          int64_t id = rel.id();

          bool ignore = false;

          std::vector<SegmentId> outer, inner;
          for (osmpbf::IMemberStream memb = rel.getMemberStream();
               !memb.isNull();
               memb.next()) {
            int64_t ref = memb.id();
            osmpbf::PrimitiveType type = memb.type();
            std::string role = memb.role();

            if (type != osmpbf::WayPrimitive) {
              ignore = true;
              break;
            }
            if (role == "outer" || role == "") {
              outer.push_back(ref);
            } else if (role == "inner") {
              inner.push_back(ref);
            } else {
              printf("Found unknown way role %s\n", role.c_str());
              ignore = true;
              // assert(false);
            }
          }

          if (ignore) {
            continue;
          }

          std::vector<osm_input::Tag> tags;

          for (int32_t i = 0, s = rel.tagsSize(); i < s; ++i) {
            tags.emplace_back(rel.key(i), rel.value(i));
          }

          localAreas.push_back(AreaPoi(id, tags, mMappingHelper, outer, inner));
        }
      }
    }

    std::unique_lock<std::mutex> lck(globalAreas->lock);
    globalAreas->areas->insert(
      globalAreas->areas->end(), localAreas.begin(), localAreas.end());
  }
};

typedef std::unordered_map<SegmentId, std::vector<NodeId>> SegmentMap;

struct SharedSegmentMap
{
  std::mutex lock;

  SegmentMap* segments;

  SharedSegmentMap()
    : segments(new SegmentMap()){};
};

struct BlockParserSegment
{
  SharedSegmentMap* globalSegments;
  SegmentMap localSegments;

  std::unordered_set<SegmentId> requested;

  BlockParserSegment(SharedSegmentMap* aSegmentsGlobal,
                     std::unordered_set<SegmentId>& aRequestedSegments)
    : globalSegments(aSegmentsGlobal)
    , requested(aRequestedSegments){};

  BlockParserSegment(const BlockParserSegment& aOther)
    : globalSegments(aOther.globalSegments)
    , requested(aOther.requested){};

  void operator()(osmpbf::PrimitiveBlockInputAdaptor(&pbi))
  {
    localSegments.clear();

    if (pbi.waysSize() > 0) {

      for (osmpbf::IWayStream way = pbi.getWayStream(); !way.isNull();
           way.next()) {
        if (requested.count(way.id()) == 0) {
          continue;
        }

        std::vector<NodeId> nodes;
        for (auto it = way.refBegin(), end = way.refEnd(); it != end; ++it) {
          nodes.push_back(*it);
        }

        localSegments.emplace(way.id(), nodes);
      }
    }

    std::unique_lock<std::mutex> lck(globalSegments->lock);
    globalSegments->segments->insert(localSegments.begin(),
                                     localSegments.end());
  }
};

typedef std::unordered_map<NodeId, osm_input::OsmPoi::Position> NodeMap;

struct SharedNodeMap
{
  std::mutex lock;

  NodeMap* nodes;

  SharedNodeMap()
    : nodes(new NodeMap()){};
};

struct BlockParserNode
{
  SharedNodeMap* globalNodes;
  NodeMap localNodes;

  std::unordered_set<NodeId> requested;

  osmpbf::RCFilterPtr filter;

  BlockParserNode(SharedNodeMap* aNodesGlobal,
                  std::unordered_set<NodeId>& aRequestedNodes)
    : globalNodes(aNodesGlobal)
    , requested(aRequestedNodes){};

  BlockParserNode(const BlockParserNode& aOther)
    : globalNodes(aOther.globalNodes)
    , requested(aOther.requested){};

  void operator()(osmpbf::PrimitiveBlockInputAdaptor(&pbi))
  {
    localNodes.clear();

    if (pbi.nodesSize() > 0) {

      for (osmpbf::INodeStream node = pbi.getNodeStream(); !node.isNull();
           node.next()) {
        if (requested.count(node.id()) == 0) {
          continue;
        }

        localNodes.emplace(
          node.id(), osm_input::OsmPoi::Position(node.latd(), node.lond()));
      }
    }

    std::unique_lock<std::mutex> lck(globalNodes->lock);
    globalNodes->nodes->insert(localNodes.begin(), localNodes.end());
  }
};

struct SharedPOISet
{
  std::mutex lock;

  PoiSet* pois;

  SharedPOISet()
    : pois(new PoiSet()){};
};

namespace {
enum NameLvl
{
  undefined = 0,
  name_en = 50,
  int_name = 70,
  official_name = 80,
  name = 100,
};

std::string
get_name(std::vector<osm_input::Tag>& tags)
{
  std::string res = "";
  NameLvl max_name_lvl = NameLvl::undefined;

  for (auto& t : tags) {
    if (t.mKey == "name:en" && max_name_lvl < NameLvl::name_en) {
      res = t.mValue;
      max_name_lvl = NameLvl::name_en;
    } else if (t.mKey == "int_name" && max_name_lvl < NameLvl::int_name) {
      res = t.mValue;
      max_name_lvl = NameLvl::int_name;
    } else if (t.mKey == "official_name" &&
               max_name_lvl < NameLvl::official_name) {
      res = t.mValue;
      max_name_lvl = NameLvl::name;
    } else if (t.mKey == "name" && max_name_lvl < NameLvl::name) {
      res = t.mValue;
      // highest valued name - skip when found this!
      break;
    }
  }

  return res;
}
}

struct BlockParserPoi
{

  SharedPOISet* globalPois;
  PoiSet localPois;
  const mapping_helper::MappingHelper& mMappingHelper;

  osmpbf::RCFilterPtr filter;

  BlockParserPoi(SharedPOISet* aPoiGlobal,
                 const mapping_helper::MappingHelper& aMappingHelper)
    : globalPois(aPoiGlobal)
    , mMappingHelper(aMappingHelper){};

  BlockParserPoi(const BlockParserPoi& aOther)
    : globalPois(aOther.globalPois)
    , mMappingHelper(aOther.mMappingHelper){};

  void operator()(osmpbf::PrimitiveBlockInputAdaptor(&pbi))
  {
    // Filter to get all nodes that have an name tag
    //     osmpbf::KeyOnlyTagFilter* nameFilter = new
    //     osmpbf::KeyOnlyTagFilter("name");
    //     filter.reset(nameFilter);

    // Filter to get all nodes that have an name and (amenity or place) tag
    //     osmpbf::OrTagFilter* orFilter = new osmpbf::OrTagFilter();
    //     orFilter->addChild(new osmpbf::KeyOnlyTagFilter("place"));
    //     orFilter->addChild(new osmpbf::KeyOnlyTagFilter("amenity"));
    //
    //     osmpbf::AndTagFilter* andFilter = new osmpbf::AndTagFilter();
    //     andFilter->addChild(new osmpbf::KeyOnlyTagFilter("name"));
    //     andFilter->addChild(orFilter);
    //     filter.reset(andFilter);

    //     Filter to get all nodes that have an name or amenity or place tag
    osmpbf::OrTagFilter* orFilter = new osmpbf::OrTagFilter();
    orFilter->addChild(new osmpbf::KeyOnlyTagFilter("place"));
    orFilter->addChild(new osmpbf::KeyOnlyTagFilter("amenity"));
    orFilter->addChild(new osmpbf::KeyOnlyTagFilter("name"));
    filter.reset(orFilter);

    filter->assignInputAdaptor(&pbi);

    if (!(filter->rebuildCache())) {
      return;
    }

    localPois.clear();

    if (pbi.nodesSize() > 0) {
      const auto& tag_keys = mMappingHelper.get_tag_key_set();

      for (osmpbf::INodeStream node = pbi.getNodeStream(); !node.isNull();
           node.next()) {
        if (filter->matches(node)) {
          osm_input::OsmPoi::Position pos(node.latd(), node.lond());
          int64_t id = node.id();
          std::vector<osm_input::Tag> tags;

          for (int32_t i = 0, s = node.tagsSize(); i < s; ++i) {
            std::string key = node.key(i);
            std::string value = node.value(i);
            tags.emplace_back(key, value);
          }

          std::string name = get_name(tags);
          auto level = mMappingHelper.computeLevel(tags);
          if (level->isUndefinedLvl()) {
            // skip if no level could be assigned to the poi!
            continue;
          }
          if (name == "" && !level->hasIcon()) {
            // skip the poi
            continue;
          }

          localPois.emplace_back(id, pos, tags, level);
        }
      }
    }

    std::unique_lock<std::mutex> lck(globalPois->lock);
    globalPois->pois->insert(
      globalPois->pois->end(), localPois.begin(), localPois.end());
  }
};

PoiSet
importAreaPois(osmpbf::OSMFileIn& aOsmFile,
               const mapping_helper::MappingHelper& aMappingHelper,
               int32_t aThreadCount,
               int32_t aBlobCount)
{
  bool threadPrivateProcessor = true; // set to true so that MyCounter is copied

  aOsmFile.reset();
  osm_parsing::SharedAreaSet areas;
  osmpbf::parseFileCPPThreads(
    aOsmFile,
    osm_parsing::BlockParserAreaPoiInfo(&areas, aMappingHelper),
    aThreadCount,
    aBlobCount,
    threadPrivateProcessor);

  std::unordered_set<SegmentId> requestedSegments;
  for (auto& area : *(areas.areas)) {
    requestedSegments.insert(area.mOuter.begin(), area.mOuter.end());
    requestedSegments.insert(area.mInner.begin(), area.mInner.end());
  }

  aOsmFile.reset();
  osm_parsing::SharedSegmentMap segments;
  osmpbf::parseFileCPPThreads(
    aOsmFile,
    osm_parsing::BlockParserSegment(&segments, requestedSegments),
    aThreadCount,
    aBlobCount,
    threadPrivateProcessor);

  std::unordered_set<NodeId> requestedNodes;
  for (auto segment : *(segments.segments)) {
    requestedNodes.insert(segment.second.begin(), segment.second.end());
  }

  aOsmFile.reset();
  osm_parsing::SharedNodeMap nodes;
  osmpbf::parseFileCPPThreads(
    aOsmFile,
    osm_parsing::BlockParserNode(&nodes, requestedNodes),
    aThreadCount,
    aBlobCount,
    threadPrivateProcessor);

  PoiSet result;
  result.reserve(areas.areas->size());
  for (auto it = areas.areas->begin(), end = areas.areas->end(); it != end;
       ++it) {
    // skip and remove / ignore the area if it was not fully contained in the
    // data set
    bool ignore = false;
    for (auto& seg : it->mOuter) {
      if (segments.segments->count(seg) == 0) {
        ignore = true;
        break;
      }
    }
    for (auto& seg : it->mInner) {
      if (segments.segments->count(seg) == 0) {
        ignore = true;
        break;
      }
    }

    if (ignore) {
      continue;
    }

    osm_input::OsmPoi* tmpPoi;
    // #pragma clang diagnostics ignore maybe-uninitialized
    if (it->getPoiInfo(*(segments.segments), *(nodes.nodes), tmpPoi)) {
      if (!tmpPoi->getLevel()->isUndefinedLvl()) {
        result.push_back(*tmpPoi);
      }
    }
  }

  return result;
};

PoiSet
importNodePois(osmpbf::OSMFileIn& aOsmFile,
               const mapping_helper::MappingHelper& aMappingHelper,
               int32_t aThreadCount,
               int32_t aBlobCount)
{
  osm_parsing::SharedPOISet pois;
  bool threadPrivateProcessor = true; // set to true so that MyCounter is copied

  osmpbf::parseFileCPPThreads(
    aOsmFile,
    osm_parsing::BlockParserPoi(&pois, aMappingHelper),
    aThreadCount,
    aBlobCount,
    threadPrivateProcessor);

  PoiSet result;
  result.reserve(pois.pois->size());
  result.insert(result.begin(), pois.pois->begin(), pois.pois->end());

  return result;
};
}

osm_input::OsmInputHelper::OsmInputHelper(
  std::string aPbfPath,
  const mapping_helper::MappingHelper& aMappingHelper,
  int32_t aThreadCount,
  int32_t aBlobCount)
  : mPbfPath(aPbfPath)
  //  , mClassDescriptionPath(aClassDescriptionPath)
  , mThreadCount(aThreadCount)
  , mBlobCount(aBlobCount)
  , mMappingHelper(aMappingHelper)
{
}

PoiSet
osm_input::OsmInputHelper::importPoiData()
{
  osmpbf::OSMFileIn osmFile(mPbfPath.c_str(), false);

  if (!osmFile.open()) {
    printf("Failed to open input osm file %s\n", mPbfPath.c_str());

    return PoiSet();
  }

  PoiSet result;
  PoiSet nodeResult = osm_parsing::importNodePois(
    osmFile, mMappingHelper, mThreadCount, mBlobCount);
  result.reserve(nodeResult.size());
  result.insert(result.end(), nodeResult.begin(), nodeResult.end());

  std::printf("Imported %lu pois from the data set.\n", nodeResult.size());

  PoiSet areaResult = osm_parsing::importAreaPois(
    osmFile, mMappingHelper, mThreadCount, mBlobCount);

  result.reserve(areaResult.size() + nodeResult.size());
  result.insert(result.end(), areaResult.begin(), areaResult.end());

  std::printf("Imported %lu area pois from the data set.\n", areaResult.size());

  return result;
}
