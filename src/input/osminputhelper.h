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

#ifndef OSMINPUTHELPER_H
#define OSMINPUTHELPER_H

#include <map>
#include <stdint.h>

#include "confighelper.h"
#include "filterhelper.h"
#include "mappinghelper.h"
#include "osmpoi.h"

namespace osm_input {

class OsmInputHelper
{
public:
  struct BoundingBox
  {
    double mMinLat;
    double mMaxLat;
    double mMinLon;
    double mMaxLon;

    BoundingBox()
      : mMinLat(90)
      , mMaxLat(-90)
      , mMinLon(180)
      , mMaxLon(-180){};

    BoundingBox(double aMinLat, double aMaxLat, double aMinLon, double aMaxLon)
      : mMinLat(aMinLat)
      , mMaxLat(aMaxLat)
      , mMinLon(aMinLon)
      , mMaxLon(aMaxLon){};

    void adapt(const osm_input::OsmPoi::Position& aPos);

    void adapt(double aLat, double aLon);
  };

public:
  OsmInputHelper(std::string aPbfPath,
                 const config_helper::ConfigHelper& aConfig,
                 int32_t aThreadCount,
                 int32_t aBlobCount);
  OsmInputHelper(const OsmInputHelper& other) = delete;
  OsmInputHelper& operator=(const OsmInputHelper& other) = delete;
  bool operator==(const OsmInputHelper& other) const = delete;

  std::vector<osm_input::OsmPoi> importPoiData();

private:
  std::string mPbfPath;
  //  std::string mClassDescriptionPath;
  int32_t mThreadCount;
  int32_t mBlobCount;

  BoundingBox mDataBox;

  const mapping_helper::MappingHelper& mMappingHelper;
  const filter_helper::FilterHelper& mFilterHelper;
};
}

#endif // OSMINPUTHELPER_H
