/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2016  Filip Krumpe <filip.krumpe@posteo.de>
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

#include "osmpoi.h"

osm_input::OsmPoi::OsmPoi(int64_t aOsmId, osm_input::OsmPoi::Position aPos)
  : OsmPoi(aOsmId, aPos, UNDEFINED){};

osm_input::OsmPoi::OsmPoi(int64_t aOsmId, osm_input::OsmPoi::Position aPos,
                          osm_input::OsmPoi::Poi_Types aType)
  : mOsmId(aOsmId)
  , mPos(aPos)
  , mPoiType(aType){};

bool
osm_input::OsmPoi::operator==(const osm_input::OsmPoi& aOther) const
{
  return aOther.mOsmId == mOsmId;
}

bool
osm_input::OsmPoi::operator!=(const osm_input::OsmPoi& aOther) const
{
  return !(*this == aOther);
}

bool
osm_input::OsmPoi::operator<(const osm_input::OsmPoi& aOther) const
{
  if (*this == aOther) {
    return false;
  }

  bool less = false;

  if (mPoiType == aOther.mPoiType) {
    if (false) {
      // compare tags like population size
      less = true;
    } else {
      // if both elements can not be distinguished fall back to id comparison
      less = mOsmId < aOther.mOsmId;
    }
  } else {
    less = mPoiType < aOther.mPoiType;
  }

  return less;
}

bool
osm_input::OsmPoi::operator>(const osm_input::OsmPoi& aOther) const
{
  return *this != aOther && !(*this < aOther);
}

bool
osm_input::OsmPoi::operator<=(const osm_input::OsmPoi& aOther) const
{
  return !(*this > aOther);
}

bool
osm_input::OsmPoi::operator>=(const osm_input::OsmPoi& aOther) const
{
  return !(*this < aOther);
}

void
osm_input::OsmPoi::addTag(std::string aKey, std::string aValue)
{
  mTags.push_back(std::pair<std::string, std::string>(aKey, aValue));
}

osm_input::OsmPoi::Poi_Types
osm_input::OsmPoi::computeType(bool aUpdateType)
{
  if (mPoiType == UNDEFINED) {
    return mPoiType;
  }

  Poi_Types type = GENERAL_POI;

  for (auto& tag : mTags) {
    if (tag.first == "place" && tag.second != "locality") {
      type = SETTLEMENT;
    }
  }

  if (aUpdateType) {
    mPoiType = type;
  }

  return type;
}