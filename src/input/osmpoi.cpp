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
  const std::vector<osm_input::OsmPoi::Tag>& aTags)
{
  std::string place = "";
  // find the place tag
  for (auto& tag : aTags) {
    if (tag.mKey == "place") {
      place = tag.mValue;
      break;
    }
  }

  int32_t subImportance = 0;

  if (place == "city")
    subImportance = 200;                // factor <500k: 22; < 1m: 26; >1m: 30
  else if (place == "town")
    subImportance = 190;                // factor <100k: 18; >100k: 20
  else if (place == "village")
    subImportance = 180;                // factor 16
  else if (place == "municipality")
    subImportance = 170;                // factor 14
  else if (place == "borough")
    subImportance = 160;                // factor 13
  else if (place == "suburb")
    subImportance = 150;                // factor 12
  else if (place == "quarter")
    subImportance = 140;                // factor 11
  else if (place == "neighbourhood")
    subImportance = 130;                // factor 11
  else if (place == "city_block")
    subImportance = 120;                // factor 11
  else if (place == "hamlet")
    subImportance = 110;                // factor 11
  else if (place == "isolated_dwelling")
    subImportance = 100;                // factor 10
  else if (place == "farm")
    subImportance = 90;                 // factor 10
  else if (place == "allotments")
    subImportance = 80;                 // factor 10
  else if (place == "plot")
    subImportance = 70;                 // factor 10

  if (subImportance == 0) {
    std::printf("\tUnable to classify place tag: %s\n", place.c_str());
  }

  return subImportance;
}

int32_t
computeGeneralPoiImportance(
  const std::vector<osm_input::OsmPoi::Tag> aTags)
{
  std::string amenity = "";
  // find the place tag
  for (auto& tag : aTags) {
    if (tag.mKey == "amenity") {
      amenity = tag.mValue;
      break;
    }
  }

  int32_t subImportance = 0;

  if (amenity == "kindergarden" || amenity == "kindergarten")
    subImportance = 200;                 // factor 9
  else if (amenity == "school")
    subImportance = 190;                 // factor 9
  else if (amenity == "bank")
    subImportance = 180;                 // factor 8
  else if (amenity == "hospital")
    subImportance = 170;                 // factor 8
  else if (amenity == "pharmacy")
    subImportance = 160;                 // factor 8
  else if (amenity == "police")
    subImportance = 150;                 // factor 8
  else if (amenity == "library")
    subImportance = 140;                 // factor 8
  else if (amenity == "cafe")
    subImportance = 130;                 // factor 7
  else if (amenity == "fast_food")
    subImportance = 120;                 // factor 7
  else if (amenity == "restaurant")
    subImportance = 110;                 // factor 7
  else if (amenity == "place_of_worship")
    subImportance = 100;                 // factor 6
  else if (amenity == "public_building")
    subImportance = 90;                  // factor 6
  else if (amenity == "recycling")
    subImportance = 80;                  // factor 5
  else if (amenity == "grave_yard")
    subImportance = 70;                  // factor 5
  else if (amenity == "parking")
    subImportance = 60;                  // factor 4
  else if (amenity == "fuel")
    subImportance = 50;                  // factor 4
  else if (amenity == "shelter")
    subImportance = 40;                  // factor 2

  return subImportance;
}



osm_input::OsmPoi::Poi_Types computeType(std::vector<osm_input::OsmPoi::Tag>& aTags)
{
  
  osm_input::OsmPoi::Poi_Types type = osm_input::OsmPoi::UNDEFINED;
  
  for (auto& tag : aTags) {
    if (tag.mKey == "place" &&
      (tag.mValue == "city" || tag.mValue == "town" ||
      tag.mValue == "village" || tag.mValue == "municipality" ||
      tag.mValue == "borough" || tag.mValue == "suburb" ||
      tag.mValue == "quarter" || tag.mValue == "neighbourhood" ||
      tag.mValue == "city_block" || tag.mValue == "hamlet" ||
      tag.mValue == "isolated_dwelling" || tag.mValue == "farm" ||
      tag.mValue == "allotments" || tag.mValue == "plot")) {
      type = osm_input::OsmPoi::SETTLEMENT;
      } else if (type == osm_input::OsmPoi::UNDEFINED) {
        type = osm_input::OsmPoi::GENERAL_POI;
      }
  }
  
  return type;
}

double computeFontFactor (std::vector<osm_input::OsmPoi::Tag>& aTags, osm_input::OsmPoi::Poi_Types& aType, int32_t aSubtype) {
  double factor = 1;  
  
  switch(aType) {
    case osm_input::OsmPoi::GENERAL_POI:
      switch (aSubtype) {
        case 200:
        case 190:
          factor = 9;
          break;
        case 180:
        case 170:
        case 160:
        case 150:
        case 140:
          factor = 8;
          break;
        case 130:
        case 120:
        case 110:
          factor = 7;
          break;
        case 100:
        case 90:
          factor = 6;
          break;
        case 80:
        case 70:
          factor = 5;
          break;
        case 60:
        case 50:
          factor = 4;
          break;
        case 40:
          factor = 2;
          break;
        default:
          factor = 1;
          break;
      }
      break;
    case osm_input::OsmPoi::SETTLEMENT: {
      int32_t population = 0;
      for (auto& tag : aTags) {
        if (tag.mKey == "population") {
          population = std::atoi(tag.mValue.c_str());
        }
      }
      
      switch(aSubtype) {
        case 200:               // city
          if (population > 1000000)
            factor = 30;
          else if (population > 1000000)
            factor = 26;
          else
            factor = 22;
          break;
        case 190:
          if (population > 100000)
            factor = 20;
          else
            factor = 18;
          break;
        case 180:
          factor = 16;
          break;
        case 170:
          factor = 14;
          break;
        case 160:
          factor = 13;
          break;
        case 150:
          factor = 12;
          break;
        case 140:
        case 130:
        case 120:
        case 110:
          factor = 11;
          break;
        case 100:
        case 90:
        case 80:
        case 70:
          factor = 10;
          break;
        default:
          std::printf("[ERROR]\tDefault settlement label factor!\n");
          break;
      }
      break;
    }
    default:
      std::printf("[ERROR]\tDefault label factor for type %i subtype %i!\n", aType, aSubtype);;
      break;
  }
  
  return factor;
}
}

osm_input::OsmPoi::OsmPoi(int64_t aOsmId, osm_input::OsmPoi::Position aPos, const std::vector<Tag> aTags)
: OsmPoi(aOsmId, aPos, UNDEFINED, aTags) {
    mPoiType = osmpoi::computeType(mTags);
    switch (mPoiType) {
      case osm_input::OsmPoi::GENERAL_POI:
        mSubImportance = osmpoi::computeGeneralPoiImportance(mTags);
        break;
      case osm_input::OsmPoi::SETTLEMENT:
        mSubImportance = osmpoi::computeSettlementImportance(mTags);
        break;
      default:
        break;
    }
    mLabelFactor = osmpoi::computeFontFactor(mTags, mPoiType, mSubImportance);
  };

  osm_input::OsmPoi::OsmPoi( int64_t aOsmId, osm_input::OsmPoi::Position aPos, osm_input::OsmPoi::Poi_Types aType, const std::vector< osm_input::OsmPoi::Tag > aTags )
  : mOsmId(aOsmId)
  , mPos(aPos)
  , mPoiType(aType)
  , mTags(aTags) {
    if (mPoiType == Poi_Types::UNDEFINED)
      mPoiType = osmpoi::computeType(mTags);
    switch (mPoiType) {
      case osm_input::OsmPoi::GENERAL_POI:
        mSubImportance = osmpoi::computeGeneralPoiImportance(mTags);
        break;
      case osm_input::OsmPoi::SETTLEMENT:
        mSubImportance = osmpoi::computeSettlementImportance(mTags);
        break;
        default:
        break;
    }
    mLabelFactor = osmpoi::computeFontFactor(mTags, mPoiType, mSubImportance);
  };

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
      if (this->mPoiType == Poi_Types::SETTLEMENT) {
        // search for population tag and compare
        std::string szPop = this->getTagValue("population");
        std::string szOtherPop = aOther.getTagValue("population");
        int32_t pop = (szPop != "<undefined>") ? std::atoi(szPop.c_str()) : 0;
        int32_t otherPop =
          (szOtherPop != "<undefined>") ? std::atoi(szOtherPop.c_str()) : 0;
        if (pop == otherPop)
          less = mOsmId < aOther.mOsmId;
        else
          less = pop < otherPop;
      } else {
        // if both elements can not be distinguished fall back to id comparison
        less = mOsmId < aOther.mOsmId;
      }
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

namespace osmpoi {
std::string
computeSplit(std::string& aLabel, const std::unordered_set<char>& aDelims)
{
  std::string labelSplit = aLabel;
  // if the label already contains newline information use this
  if (aLabel.find("\r\n") != aLabel.npos) {
    labelSplit = aLabel.replace(aLabel.find("\r\n"), 2, "%");
  } else if (aLabel.find("\n") != aLabel.npos) {
    labelSplit = aLabel.replace(aLabel.find("\n"), 2, "%");
  } else if (aLabel.find("\r") != aLabel.npos) {
    labelSplit = aLabel.replace(aLabel.find("\r"), 2, "%");
  } else if (aLabel.find("^M") != aLabel.npos) {
    labelSplit = aLabel.replace(aLabel.find("^M"), 2, "%");
  } else {
    // otherwise split at one of the delimiters

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
  }
  if (labelSplit.find(" %") != labelSplit.npos)
    labelSplit.replace(labelSplit.find(" %"), 2, "%");

  return labelSplit;
}

double
computeBallRadius(const std::string& aLabel)
{
  std::size_t delimPos = aLabel.find("%");
  if (delimPos == aLabel.npos)
    delimPos = aLabel.size();

  double labelSize = (double)(delimPos > aLabel.size() / 2)
                       ? delimPos
                       : aLabel.size() - delimPos;
  return labelSize / 2;
}
}

osm_input::OsmPoi::LabelBall
osm_input::OsmPoi::getCorrespondingBall(
  std::size_t aSplitSize, const std::unordered_set<char>& aDelims) const
{
  // corresponds to a little icon
  std::string label = "undef";
  double ballRadius = ((double)label.size() + 1) / 2;

  for (auto& tag : mTags) {
    if (tag.mKey == "name") {
      if (tag.mValue.size() > aSplitSize) {
        label = tag.mValue;
        label = osmpoi::computeSplit(label, aDelims);
        ballRadius = osmpoi::computeBallRadius(label);
      } else {
        label = tag.mValue;
        ballRadius = label.size() / 2;
      }
    }
  }

  ballRadius *= mLabelFactor;

  return LabelBall(mPos, ballRadius, label, mLabelFactor);
}

std::string
osm_input::OsmPoi::getTagValue(std::string aTagName) const
{
  for (auto& tag : mTags) {
    if (tag.mKey == aTagName) {
      return tag.mValue;
    }
  }

  return "<undefined>";
}
