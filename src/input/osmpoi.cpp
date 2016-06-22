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
 * MERCHANTABILITY or FITNESS FOR A PARTbICULAR PURPOSE.  See the
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
  else if (amenity == "school" || amenity == "preschool")
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
    subImportance = 40;                  // factor 4

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
          factor = 9;
          break;
        case 130:
        case 120:
        case 110:
          factor = 8;
          break;
        case 100:
        case 90:
          factor = 8;
          break;
        case 80:
        case 70:
          factor = 7;
          break;
        case 60:
        case 50:
          factor = 7;
          break;
        case 40:
          factor = 7;
          break;
        default:
          factor = 6;
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

std::string computeIcon(const std::vector<osm_input::OsmPoi::Tag>& aTagSet) {
  std::string result = "";
  std::string amenity = "";
  
  for (auto& tag : aTagSet) {
    if (tag.mKey == "amenity")
      amenity = tag.mValue;
  }
  
  if (amenity == "administration")
	  result = "icon:administration";
  if (amenity == "airport")
	  result = "icon:airport";
  if (amenity == "artwork" || amenity == "arts_centre")
	  result = "icon:artgallery";
  if (amenity == "waste_transfer_station")
	  result = "icon:assortment";
  if (amenity == "atm" || amenity == "atm;parking")
	  result = "icon:atm";
  if (amenity == "financial_institution" || amenity == "bank" || amenity == "financial_service")
	  result = "icon:bank";
  if (amenity == "pub" || amenity == "brewery")
	  result = "icon:bar";
  if (amenity == "food_court" || amenity == "biergarten" || amenity == "biergarten")
	  result = "icon:beergarden";
  if (amenity == "bicycle_repair_station" || amenity == "bicycle_rental")
	  result = "icon:bicycle_shop";
  if (amenity == "boat_rental")
	  result = "icon:boat";
  if (amenity == "gym" || amenity == "fitness_studio" || amenity == "training")
	  result = "icon:breastfeeding";
  if (amenity == "bus_station")
	  result = "icon:bus";
  if (amenity == "canteen")
	  result = "icon:cafeteria";
  if (amenity == "vehicle_inspection" || amenity == "car_repair" || amenity == "car_rental" || amenity == "car_sharing")
	  result = "icon:car";
  if (amenity == "driving_school")
	  result = "icon:car_share";
  if (amenity == "casino")
	  result = "icon:casino-2";
  if (amenity == "grave_yard" || amenity == "deaddrop")
	  result = "icon:catholicgrave";
  if (amenity == "place_of_worship")
	  result = "icon:church";
  if (amenity == "cinema")
	  result = "icon:cinema";
  if (amenity == "clock")
	  result = "icon:clock";
  if (amenity == "cafe-bar" || amenity == "cafe" || amenity == "cafe,_konditorei")
	  result = "icon:coffee";
  if (amenity == "community_center" || amenity == "social_centre")
	  result = "icon:communitycentre";
  if (amenity == "conference_centre")
	  result = "icon:conference";
  if (amenity == "public_building" || amenity == "public_facility" || amenity == "community_hall" || amenity == "townhall")
	  result = "icon:congress";
  if (amenity == "courthouse")
	  result = "icon:court";
  if (amenity == "danzing_school")
	  result = "icon:dance_class";
  if (amenity == "dancing_club" || amenity == "nightclub")
	  result = "icon:dancinghall";
  if (amenity == "kindergarten" || amenity == "childcare" || amenity == "childcare;kindergarten")
	  result = "icon:daycare";
  if (amenity == "dentist")
	  result = "icon:dentist";
  if (amenity == "drinking_water")
	  result = "icon:drinkingfountain";
  if (amenity == "perfume")
	  result = "icon:drugstore";
  if (amenity == "charging_station")
	  result = "icon:e-bike-charging";
  if (amenity == "fast_food")
	  result = "icon:fast_food";
  if (amenity == "ferry_terminal")
	  result = "icon:ferry";
  if (amenity == "fuel")
	  result = "icon:fillingstation";
  if (amenity == "fire_station")
	  result = "icon:firemen";
  if (amenity == "red_cross"|| amenity == "emergency_service" || amenity == "healthcare")
	  result = "icon:firstaid";
  if (amenity == "fountain")
	  result = "icon:fountain";
  if (amenity == "Beratungsstellen")
	  result = "icon:group-2";
  if (amenity == "college")
	  result = "icon:highschool";
  if (amenity == "hospital" || amenity == "healthcare:speciality=occupational" || amenity == "red_cross" || amenity == "clinic")
	  result = "icon:hospital";
  if (amenity == "ice_cream")
	  result = "icon:ice_cream";
  if (amenity == "club_house" || amenity == "club" || amenity == "bar")
	  result = "icon:jazzclub";
  if (amenity == "library;archive" || amenity == "library")
	  result = "icon:library";
  if (amenity == "marketplace")
	  result = "icon:market";
  if (amenity == "pharmacy")
	  result = "icon:medicalstore";
  if (amenity == "doctors")
	  result = "icon:medicine";
  if (amenity == "music_school")
	  result = "icon:music_classical";
  if (amenity == "nursery")
	  result = "icon:nursery";
  if (amenity == "nursing_home")
	  result = "icon:nursing_home_icon";
  if (amenity == "register_office")
	  result = "icon:office-building";
  if (amenity == "parking" || amenity == "parking_space")
	  result = "icon:parking";
  if (amenity == "bicycle_parking")
	  result = "icon:parking_bicycle-2";
  if (amenity == "police" || amenity == "ranger_station")
	  result = "icon:police";
  if (amenity == "bank; post_office" || amenity == "post_office" || amenity == "post_box")
	  result = "icon:postal";
  if (amenity == "restaurant")
	  result = "icon:restaurant";
  if (amenity == "sauna")
	  result = "icon:sauna";
  if (amenity == "education" || amenity == "language_school" || amenity == "school")
	  result = "icon:school";
  if (amenity == "shoe")
	  result = "icon:shoes";
  if (amenity == "spa" || amenity == "solarium")
	  result = "icon:spa";
  if (amenity == "love_hotel" || amenity == "swingerclub" || amenity == "stripclub")
	  result = "icon:stripclub";
  if (amenity == "shop" || amenity == "fair_trade")
	  result = "icon:supermarket";
  if (amenity == "taxi")
	  result = "icon:taxi";
  if (amenity == "telephone")
    result = "icon:telephone";
  if (amenity == "theatre")
    result = "icon:theater";
  if (amenity == "toilets")
	  result = "icon:toilets";
  if (amenity == "Handwerksbetrieb")
	  result = "icon:tools";
  if (amenity == "waste_disposal" || amenity == "waste_basket")
	  result = "icon:trash";
  if (amenity == "travel agency")
	  result = "icon:travel_agency";
  if (amenity == "university")
	  result = "icon:university";
  if (amenity == "veterinary")
	  result = "icon:veterinary";
  if (amenity == "shelter")
	  result = "icon:waiting";
  if (amenity == "internet_cafe")
	  result = "icon:wifi";
  if (amenity == "coworking_space")
	  result = "icon:workoffice";
  
  
  return result;
}
}

bool osm_input::OsmPoi::hasIcon() const {
	bool icon = false;
	
	if (mPoiType != osm_input::OsmPoi::SETTLEMENT)
		icon = osmpoi::computeIcon(mTags) != "";
	
	return icon;
}


osm_input::OsmPoi::LabelBall
osm_input::OsmPoi::getCorrespondingBall(
  std::size_t aSplitSize, const std::unordered_set<char>& aDelims) const
{
  std::string label = (mPoiType == osm_input::OsmPoi::SETTLEMENT) ? "SETTLEMENT" : osmpoi::computeIcon(mTags);
  // initialize for an icon
  double ballRadius = 4;
  if (label == "SETTLEMENT") {
    label = getName();
    ballRadius = (double)label.size() / 2;
    if (label.size() > aSplitSize) {
      label = osmpoi::computeSplit(label, aDelims);
      ballRadius = osmpoi::computeBallRadius(label);
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

std::string osm_input::OsmPoi::getName() const {
  std::string name = "<undefined>";
  
  enum Dom {UNDEF, NAME, NAME_DE, NAME_EN};
  Dom d = Dom::UNDEF;
  
  for (auto& tag : mTags) {
    if (tag.mKey == "name" && d < Dom::NAME) {
      name = tag.mValue;
      d = Dom::NAME;
    } else if (tag.mKey == "name:de" && d < Dom::NAME_DE) {
      name = tag.mValue;
      d = Dom::NAME_DE;
    } else if (tag.mKey == "name:en" && d < Dom::NAME_EN) {
      name = tag.mValue;
      d = Dom::NAME_EN;
    }
  }
  
  return name;
}
