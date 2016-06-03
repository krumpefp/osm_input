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

#include <stdint.h>

#include "osmpoi.h"

namespace osm_input {

class OsmInputHelper
{
public:
  struct BoundingBox
  {
    int32_t mMinLat;
    int32_t mMaxLat;
    int32_t mMinLon;
    int32_t mMaxLon;

    BoundingBox()
      : mMinLat(90)
      , mMaxLat(-90)
      , mMinLon(180)
      , mMaxLon(-180){};

    BoundingBox(int32_t aMinLat, int32_t aMaxLat, int32_t aMinLon,
                int32_t aMaxLon)
      : mMinLat(aMinLat)
      , mMaxLat(aMaxLat)
      , mMinLon(aMinLon)
      , mMaxLon(aMaxLon){};

    void adapt(const osm_input::OsmPoi::Position& aPos);

    void adapt(int32_t aLat, int32_t aLon);
  };

public:
  OsmInputHelper(std::string aPbfPath);
  OsmInputHelper(const OsmInputHelper& other) = delete;
  ~OsmInputHelper();
  OsmInputHelper& operator=(const OsmInputHelper& other) = delete;
  bool operator==(const OsmInputHelper& other) const = delete;

  std::vector<const osm_input::OsmPoi*> importPoiData(bool aIncludeSettlements,
                                                      bool aIncludeGeneral);

private:
  std::string mPbfPath;

  BoundingBox mDataBox;

  std::vector<const osm_input::OsmPoi*> mPois;
};
}

#endif // OSMINPUTHELPER_H
