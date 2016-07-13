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

#include "osmpoi.h"

#include <assert.h>
#include <math.h>
#include <unordered_map>

namespace osmpoi {

std::unordered_map<std::string, int32_t> settlementSubImp = {
  { "city", 200 },
  { "town", 190 },
  { "village", 180 },
  { "municipality", 170 },
  { "borough", 160 },
  { "suburb", 150 },
  { "quarter", 140 },
  { "neighbourhood", 130 },
  { "city_block", 120 },
  { "hamlet", 110 },
  { "isolated_dwelling", 100 },
  { "farm", 90 },
  { "allotment", 80 },
  { "plot", 70 }
};

int32_t
computeSettlementImportance(const osm_input::OsmPoi* aPoi)
{
  // find the sub importance for the place tag value if no matching value was
  // found return 0
  auto subImp = settlementSubImp.find(aPoi->getTagValue("place"));

  return (subImp != settlementSubImp.end()) ? subImp->second : 0;
}

std::unordered_map<std::string, int32_t> amenitySubImp = {
  { "kindergarden", 200 },
  { "kindergarten", 200 },
  { "school", 190 },
  { "bank", 180 },
  { "hospital", 170 },
  { "pharmacy", 160 },
  { "police", 150 },
  { "library", 140 },
  { "cafe", 130 },
  { "fast_food", 120 },
  { "restaurant", 110 },
  { "place_of_worship", 100 },
  { "public_building", 90 },
  { "recycling", 80 },
  { "grave_yard", 70 },
  { "parking", 60 },
  { "fuel", 50 },
  { "shelter", 40 }
};

int32_t
computeGeneralPoiImportance(const osm_input::OsmPoi* aPoi)
{
  // find the sub importance for the place tag value if no matching value was
  // found return 0
  auto subImp = amenitySubImp.find(aPoi->getTagValue("amenity"));

  return (subImp != amenitySubImp.end()) ? subImp->second : 0;
}

osm_input::OsmPoi::Poi_Types
computeType(const osm_input::OsmPoi* aPoi)
{
  // try to find a matching item in the settlement sub importance map
  // if such an element was found return true, false otherwise
  auto place = settlementSubImp.find(aPoi->getTagValue("place"));

  return (place != settlementSubImp.end())
           ? osm_input::OsmPoi::Poi_Types::SETTLEMENT
           : osm_input::OsmPoi::Poi_Types::GENERAL_POI;
}

std::unordered_map<int32_t, double> settlementFactors;

double
computeFontFactor(const osm_input::OsmPoi* aPoi,
                  osm_input::OsmPoi::Poi_Types& aType, int32_t aSubtype)
{
  double factor = 1;

  switch (aType) {
    case osm_input::OsmPoi::GENERAL_POI:
      if (aSubtype >= 190) {
        factor = 9;
      } else if (aSubtype >= 140) {
        factor = 8;
      } else if (aSubtype >= 110) {
        factor = 7;
      } else if (aSubtype >= 90) {
        factor = 6;
      } else if (aSubtype >= 70) {
        factor = 5;
      } else if (aSubtype >= 40) {
        factor = 4;
      } else {
        factor = 3;
      }
      break;
    case osm_input::OsmPoi::SETTLEMENT: {
      std::string szPop = aPoi->getTagValue("population");
      int32_t population =
        (szPop != "<undefined>") ? std::atoi(szPop.c_str()) : 0;

      if (aSubtype >= 200) {
        if (population > 1000000)
          factor = 30;
        else if (population > 1000000)
          factor = 26;
        else
          factor = 22;
      } else if (aSubtype >= 190) {
        if (population > 100000)
          factor = 20;
        else
          factor = 18;
      } else if (aSubtype >= 180) {
        factor = 16;
      } else if (aSubtype >= 170) {
        factor = 14;
      } else if (aSubtype >= 160) {
        factor = 13;
      } else if (aSubtype >= 150) {
        factor = 12;
      } else if (aSubtype >= 110) {
        factor = 11;
      } else if (aSubtype >= 70) {
        factor = 10;
      } else {
        std::printf("[ERROR]\tDefault settlement label factor!\n");
      }
      break;
    }
    default:
      std::printf("[ERROR]\tDefault label factor for type %i subtype %i!\n",
                  aType, aSubtype);
      break;
  }

  return factor;
}
}

osm_input::OsmPoi::OsmPoi(int64_t aOsmId, osm_input::OsmPoi::Position aPos,
                          const std::vector<Tag> aTags,
                          const mapping_helper::MappingHelper& aMh)
  : OsmPoi(aOsmId, aPos, UNDEFINED, aTags, aMh)
{
  mPoiType = osmpoi::computeType(this);
  // mPoiLevel = aMh.computeLevel(aTags);
  switch (mPoiType) {
    case osm_input::OsmPoi::GENERAL_POI:
      mSubImportance = osmpoi::computeGeneralPoiImportance(this);
      break;
    case osm_input::OsmPoi::SETTLEMENT:
      mSubImportance = osmpoi::computeSettlementImportance(this);
      break;
    default:
      break;
  }
  mLabelFactor = osmpoi::computeFontFactor(this, mPoiType, mSubImportance);
};

osm_input::OsmPoi::OsmPoi(int64_t aOsmId, osm_input::OsmPoi::Position aPos,
                          osm_input::OsmPoi::Poi_Types aType,
                          const std::vector<osm_input::Tag> aTags,
                          const mapping_helper::MappingHelper& aMh)
  : mOsmId(aOsmId)
  , mPos(aPos)
  , mPoiType(aType)
  , mPoiLevel(aMh.computeLevel(aTags))
  , mTags(aTags)
{
  // mPoiLevel = aMh.computeLevel(aTags);
  if (mPoiType == Poi_Types::UNDEFINED)
    mPoiType = osmpoi::computeType(this);
  switch (mPoiType) {
    case osm_input::OsmPoi::GENERAL_POI:
      mSubImportance = osmpoi::computeGeneralPoiImportance(this);
      break;
    case osm_input::OsmPoi::SETTLEMENT:
      mSubImportance = osmpoi::computeSettlementImportance(this);
      break;
    default:
      break;
  }
  mLabelFactor = osmpoi::computeFontFactor(this, mPoiType, mSubImportance);
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
  return (this->mPoiLevel.mLevelId < aOther.mPoiLevel.mLevelId);
  // if (*this == aOther) {
  //   return false;
  // }
  //
  // bool less = false;
  //
  // if (mPoiType == aOther.mPoiType) {
  //   if (this->mSubImportance == aOther.mSubImportance) {
  //     if (this->mPoiType == Poi_Types::SETTLEMENT) {
  //       // search for population tag and compare
  //       std::string szPop = this->getTagValue("population");
  //       std::string szOtherPop = aOther.getTagValue("population");
  //       int32_t pop = (szPop != "<undefined>") ? std::atoi(szPop.c_str()) :
  //       0;
  //       int32_t otherPop =
  //         (szOtherPop != "<undefined>") ? std::atoi(szOtherPop.c_str()) : 0;
  //       if (pop == otherPop)
  //         less = mOsmId < aOther.mOsmId;
  //       else
  //         less = pop < otherPop;
  //     } else {
  //       // if both elements can not be distinguished fall back to id
  //       comparison
  //       less = mOsmId < aOther.mOsmId;
  //     }
  //   } else {
  //     less = this->mSubImportance < aOther.mSubImportance;
  //   }
  // } else {
  //   less = mPoiType < aOther.mPoiType;
  // }
  //
  // return less;
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

  double labelSize =
    (delimPos > aLabel.size() / 2) ? delimPos : aLabel.size() - delimPos;
  return labelSize / 2;
}

std::string
computeIcon(const std::vector<osm_input::Tag>& aTagSet)
{
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
  if (amenity == "financial_institution" || amenity == "bank" ||
      amenity == "financial_service")
    result = "icon:bank";
  if (amenity == "hospital" ||
      amenity == "healthcare:speciality=occupational" || amenity == "red_cross")
    result = "icon:hospital";
  if (amenity == "canteen")
    result = "icon:cafeteria";
  if (amenity == "food_court" || amenity == "biergarten" ||
      amenity == "biergarten")
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
  if (amenity == "vehicle_inspection" || amenity == "car_repair" ||
      amenity == "car_rental" || amenity == "car_sharing")
    result = "icon:car";
  if (amenity == "driving_school")
    result = "icon:car_share";
  if (amenity == "grave_yard" || amenity == "deaddrop")
    result = "icon:catholicgrave";
  if (amenity == "place_of_worship")
    result = "icon:church";
  if (amenity == "cinema")
    result = "icon:cinema";
  if (amenity == "clock")
    result = "icon:clock";
  if (amenity == "cafe-bar" || amenity == "cafe" ||
      amenity == "cafe,_konditorei")
    result = "icon:coffee";
  if (amenity == "community_center" || amenity == "social_centre")
    result = "icon:communitycentre";
  if (amenity == "conference_centre")
    result = "icon:conference";
  if (amenity == "public_building" || amenity == "public_facility" ||
      amenity == "community_hall" || amenity == "townhall")
    result = "icon:congress";
  if (amenity == "courthouse")
    result = "icon:court";
  if (amenity == "danzing_school")
    result = "icon:dance_class";
  if (amenity == "dancing_club" || amenity == "nightclub")
    result = "icon:dancinghall";
  if (amenity == "kindergarten" || amenity == "childcare" ||
      amenity == "childcare;kindergarten")
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
  if (amenity == "red_cross" || amenity == "emergency_service" ||
      amenity == "healthcare")
    result = "icon:firstaid";
  if (amenity == "fountain")
    result = "icon:fountain";
  if (amenity == "Beratungsstellen")
    result = "icon:group-2";
  if (amenity == "college")
    result = "icon:highschool";
  if (amenity == "hospital" ||
      amenity == "healthcare:speciality=occupational" ||
      amenity == "red_cross" || amenity == "clinic")
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
  if (amenity == "bank; post_office" || amenity == "post_office" ||
      amenity == "post_box")
    result = "icon:postal";
  if (amenity == "restaurant")
    result = "icon:restaurant";
  if (amenity == "sauna")
    result = "icon:sauna";
  if (amenity == "education" || amenity == "language_school" ||
      amenity == "school")
    result = "icon:school";
  if (amenity == "shoe")
    result = "icon:shoes";
  if (amenity == "spa" || amenity == "solarium")
    result = "icon:spa";
  if (amenity == "love_hotel" || amenity == "swingerclub" ||
      amenity == "stripclub")
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

bool
osm_input::OsmPoi::hasIcon() const
{
  bool icon = false;

  if (mPoiType != osm_input::OsmPoi::SETTLEMENT)
    icon = osmpoi::computeIcon(mTags) != "";

  return icon;
}

osm_input::OsmPoi::LabelBall
osm_input::OsmPoi::getCorrespondingBall(
  std::size_t aSplitSize, const std::unordered_set<char>& aDelims) const
{
  std::string label = (mPoiType == osm_input::OsmPoi::SETTLEMENT)
                        ? ""
                        : osmpoi::computeIcon(mTags);
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

const mapping_helper::MappingHelper::Level&
osm_input::OsmPoi::getLevel() const
{
  return mPoiLevel;
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

std::string
osm_input::OsmPoi::getName() const
{
  std::string name = "<undefined>";

  enum Dom
  {
    UNDEF,
    NAME,
    NAME_DE,
    NAME_EN
  };
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
