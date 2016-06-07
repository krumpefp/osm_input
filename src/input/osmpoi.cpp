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

#include <assert.h>
#include <math.h>

namespace osmpoi {
int32_t
computeSettlementImportance(
  const std::vector<std::pair<std::string, std::string>>& aTags)
{
  std::string place = "";
  // find the place tag
  for (auto& tag : aTags) {
    if (tag.first == "place") {
      place = tag.second;
      break;
    }
  }

  int32_t subImportance = 0;

  if (place == "city")
    subImportance = 200;
  else if (place == "town")
    subImportance = 190;
  else if (place == "village")
    subImportance = 180;
  else if (place == "municipality")
    subImportance = 170;
  else if (place == "borough")
    subImportance = 160;
  else if (place == "suburb")
    subImportance = 150;
  else if (place == "quarter")
    subImportance = 140;
  else if (place == "neighbourhood")
    subImportance = 130;
  else if (place == "city_block")
    subImportance = 120;
  else if (place == "hamlet")
    subImportance = 110;
  else if (place == "isolated_dwelling")
    subImportance = 100;
  else if (place == "farm")
    subImportance = 90;
  else if (place == "allotments")
    subImportance = 80;
  else if (place == "plot")
    subImportance = 70;

  if (subImportance == 0) {
    std::printf("\tUnable to classify place tag: %s\n", place.c_str());
  }

  return subImportance;
}

int32_t
computeGeneralPoiImportance(
  const std::vector<std::pair<std::string, std::string>>& aTags)
{
  std::string amenity = "";
  // find the place tag
  for (auto& tag : aTags) {
    if (tag.first == "amenity") {
      amenity = tag.second;
      break;
    }
  }

  int32_t subImportance = 0;

  if (amenity == "kindergarden" || amenity == "kindergarten")
    subImportance = 200;
  else if (amenity == "school")
    subImportance = 190;
  else if (amenity == "bank")
    subImportance = 180;
  else if (amenity == "grave_yard")
    subImportance = 170;
  else if (amenity == "hospital")
    subImportance = 160;
  else if (amenity == "pharmacy")
    subImportance = 150;
  else if (amenity == "police")
    subImportance = 140;
  else if (amenity == "library")
    subImportance = 130;
  else if (amenity == "place_of_worship")
    subImportance = 120;
  else if (amenity == "public_building")
    subImportance = 110;
  else if (amenity == "recycling")
    subImportance = 100;
  else if (amenity == "cafe")
    subImportance = 90;
  else if (amenity == "fast_food")
    subImportance = 80;
  else if (amenity == "restaurant")
    subImportance = 70;
  else if (amenity == "parking")
    subImportance = 60;
  else if (amenity == "fuel")
    subImportance = 50;
  else if (amenity == "shelter")
    subImportance = 40;

  return subImportance;
}
}

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
    if (this->mSubImportance == aOther.mSubImportance) {
      // if both elements can not be distinguished fall back to id comparison
      less = mOsmId < aOther.mOsmId;
    } else {
      less = this->mSubImportance < aOther.mSubImportance;
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
  if (mPoiType != UNDEFINED) {
    return mPoiType;
  }

  Poi_Types type = UNDEFINED;

  for (auto& tag : mTags) {
    if (tag.first == "place" &&
        (tag.second == "city" || tag.second == "town" ||
         tag.second == "village" || tag.second == "municipality" ||
         tag.second == "borough" || tag.second == "suburb" ||
         tag.second == "quarter" || tag.second == "neighbourhood" ||
         tag.second == "city_block" || tag.second == "hamlet" ||
         tag.second == "isolated_dwelling" || tag.second == "farm" ||
         tag.second == "allotments" || tag.second == "plot")) {
      type = SETTLEMENT;
    } else if (type == UNDEFINED) {
      type = GENERAL_POI;
    }
  }

  if (aUpdateType) {
    mPoiType = type;

    if (type == GENERAL_POI) {
      mSubImportance = osmpoi::computeGeneralPoiImportance(mTags);
    } else {
      mSubImportance = osmpoi::computeSettlementImportance(mTags);
    }
  }

  return type;
}

namespace osmpoi {
std::string
computeSplit(std::string& aLabel, const std::unordered_set<char>& aDelims)
{
  if (aLabel.find("\n") != aLabel.npos) {
    return aLabel.replace(aLabel.find("\n"), aLabel.npos, "$");
  }

  std::string labelSplit = aLabel;

  std::size_t centerPos = aLabel.size() / 2;
  std::size_t pos = 0;
  while (pos < centerPos / 2) {
    char c = aLabel[centerPos + pos];
    if (aDelims.count(c) > 0) {
      labelSplit = aLabel.substr(0, centerPos + pos + 1) + "%" +
                   aLabel.substr(centerPos + pos + 1, aLabel.size());
      break;
    }
    c = aLabel[centerPos - pos];
    if (aDelims.count(c) > 0) {
      labelSplit = aLabel.substr(0, centerPos - pos + 1) + "%" +
                   aLabel.substr(centerPos - pos + 1, aLabel.size());
      break;
    }

    ++pos;
  }

  return labelSplit;
}

std::size_t
computeBallRadius(const std::string& aLabel)
{
  std::size_t delimPos = aLabel.find("%");
  if (delimPos == aLabel.npos)
    delimPos = aLabel.size();

  std::size_t labelSize =
    (delimPos > aLabel.size() / 2) ? delimPos : aLabel.size() - delimPos;
  return ((int32_t)labelSize + 1) / 2;
}
}

osm_input::OsmPoi::LabelBall
osm_input::OsmPoi::getCorrespondingBall(
  std::size_t aSplitSize, const std::unordered_set<char>& aDelims) const
{
  // corresponds to a little icon
  std::string label = "undef";
  int32_t ballRadius = ((int32_t)label.size() + 1) / 2;

  for (auto& tag : mTags) {
    if (tag.first == "name") {
      if (tag.second.size() > aSplitSize) {
        label = tag.second;
        label = osmpoi::computeSplit(label, aDelims);
        ballRadius = (int32_t)osmpoi::computeBallRadius(label);
      } else {
        label = tag.second;
        ballRadius = ((int32_t)label.size() + 1) / 2;
      }
    }
  }

  return LabelBall(mPos, ballRadius, label);
}

std::string
osm_input::OsmPoi::getTagValue(std::string aTagName) const
{
  for (auto& tag : mTags) {
    if (tag.first == aTagName) {
      return tag.second;
    }
  }

  return "<undefined>";
}
