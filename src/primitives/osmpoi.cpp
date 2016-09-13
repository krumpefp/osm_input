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

osm_input::OsmPoi::OsmPoi(int64_t aOsmId, osm_input::OsmPoi::Position aPos,
                          const std::vector<osm_input::Tag> aTags,
                          const mapping_helper::MappingHelper &aMh)
    : mOsmId(aOsmId), mPos(aPos), mPoiLevel(aMh.computeLevel(aTags)),
      mTags(aTags){};

bool osm_input::OsmPoi::operator==(const osm_input::OsmPoi &aOther) const {
  return aOther.mOsmId == mOsmId;
}

bool osm_input::OsmPoi::operator!=(const osm_input::OsmPoi &aOther) const {
  return !(*this == aOther);
}

bool osm_input::OsmPoi::operator<(const osm_input::OsmPoi &aOther) const {
  if (this->mPoiLevel != aOther.mPoiLevel) {
    return (this->mPoiLevel < aOther.mPoiLevel);
  } else {
    std::string szPop = this->getTagValue("population");
    std::string szOtherPop = aOther.getTagValue("population");
    int32_t pop = (szPop != "<undefined>") ? std::atoi(szPop.c_str()) : 0;
    int32_t otherPop =
        (szOtherPop != "<undefined>") ? std::atoi(szOtherPop.c_str()) : 0;
    if (pop == otherPop)
      return mOsmId < aOther.mOsmId;
    else
      return pop < otherPop;
  }
}

bool osm_input::OsmPoi::operator>(const osm_input::OsmPoi &aOther) const {
  return *this != aOther && !(*this < aOther);
}

bool osm_input::OsmPoi::operator<=(const osm_input::OsmPoi &aOther) const {
  return !(*this > aOther);
}

bool osm_input::OsmPoi::operator>=(const osm_input::OsmPoi &aOther) const {
  return !(*this < aOther);
}

namespace osmpoi {
std::string computeSplit(const std::string &aLabel,
                         const std::unordered_set<char> &aDelims) {
  std::string tmpLabel = aLabel;
  std::string labelSplit = aLabel;
  // if the label already contains newline information use this
  if (tmpLabel.find("\r\n") != tmpLabel.npos) {
    labelSplit = tmpLabel.replace(tmpLabel.find("\r\n"), 2, "%");
  } else if (tmpLabel.find("\n") != tmpLabel.npos) {
    labelSplit = tmpLabel.replace(tmpLabel.find("\n"), 2, "%");
  } else if (tmpLabel.find("\r") != tmpLabel.npos) {
    labelSplit = tmpLabel.replace(tmpLabel.find("\r"), 2, "%");
  } else if (tmpLabel.find("^M") != tmpLabel.npos) {
    labelSplit = tmpLabel.replace(tmpLabel.find("^M"), 2, "%");
  } else {
    // otherwise split at one of the delimiters

    std::size_t centerPos = tmpLabel.size() / 2;
    std::size_t pos = 0;
    while (pos < centerPos / 2) {
      char c = tmpLabel[centerPos + pos];
      if (aDelims.count(c) > 0) {
        labelSplit = tmpLabel.substr(0, centerPos + pos + 1) + "%" +
                     tmpLabel.substr(centerPos + pos + 1, tmpLabel.size());
        break;
      }
      c = tmpLabel[centerPos - pos];
      if (aDelims.count(c) > 0) {
        labelSplit = tmpLabel.substr(0, centerPos - pos + 1) + "%" +
                     tmpLabel.substr(centerPos - pos + 1, tmpLabel.size());
        break;
      }

      ++pos;
    }
  }
  if (labelSplit.find(" %") != labelSplit.npos)
    labelSplit.replace(labelSplit.find(" %"), 2, "%");

  return labelSplit;
}

double computeBallRadius(const std::string &aLabel) {
  std::size_t delimPos = aLabel.find("%");
  if (delimPos == aLabel.npos)
    delimPos = aLabel.size();

  std::size_t labelSize =
      (delimPos > aLabel.size() / 2) ? delimPos : aLabel.size() - delimPos;
  return (double)labelSize / 2;
}
}

bool osm_input::OsmPoi::hasIcon() const { return mPoiLevel->mIconName != ""; }

osm_input::OsmPoi::LabelBall osm_input::OsmPoi::getCorrespondingBall(
    std::size_t aSplitSize, const std::unordered_set<char> &aDelims) const {
  std::string label;
  double ballRadius = 4;
  if (mPoiLevel->mIconName != "") {
    label = "icon:" + mPoiLevel->mIconName;
  } else {
    if (getName().size() > aSplitSize) {
      label = osmpoi::computeSplit(getName(), aDelims);
    } else {
      label = getName();
    }

    ballRadius = osmpoi::computeBallRadius(label);
  }

  ballRadius *= mPoiLevel->mLevelFactor;

  return LabelBall(mPos, ballRadius, label, mPoiLevel->mLevelFactor);
}

const mapping_helper::MappingHelper::Level *
osm_input::OsmPoi::getLevel() const {
  return mPoiLevel;
}

const std::vector<osm_input::Tag> &osm_input::OsmPoi::getTags() const {
  return mTags;
}

std::string osm_input::OsmPoi::getTagValue(std::string aTagName) const {
  for (auto &tag : mTags) {
    if (tag.mKey == aTagName) {
      return tag.mValue;
    }
  }

  return "<undefined>";
}

std::string osm_input::OsmPoi::getName() const {
  std::string name = "<undefined>";

  enum Dom { UNDEF, NAME, NAME_DE, NAME_EN };
  Dom d = Dom::UNDEF;

  for (auto &tag : mTags) {
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
