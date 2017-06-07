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
#include <codecvt>
#include <locale>
#include <math.h>
#include <unordered_map>

osm_input::OsmPoi::OsmPoi(int64_t aOsmId,
                          osm_input::OsmPoi::Position aPos,
                          const std::vector<osm_input::Tag>& aTags,
                          const mapping_helper::MappingHelper::Level* aLevel)
  : mOsmId(aOsmId)
  , mPos(aPos)
  , mPoiLevel(aLevel)
  , mTags(aTags){};

/*
osm_input::OsmPoi::OsmPoi(int64_t aOsmId,
                          osm_input::OsmPoi::Position aPos,
                          const std::vector<osm_input::Tag>& aTags,
                          const mapping_helper::MappingHelper::Level* aLvl)
  : mOsmId(aOsmId)
  , mPos(aPos)
  , mPoiLevel(aLvl)
  , mTags(aTags){};
*/

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
  if (this->mPoiLevel != aOther.mPoiLevel) {
    return (*this->mPoiLevel < *aOther.mPoiLevel);
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

bool
osm_input::OsmPoi::operator>(const osm_input::OsmPoi& aOther) const
{
  return aOther < *this;
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
std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> CONVERT;

std::size_t
computeLengthUTF8(const std::string& aStr)
{
  std::u32string ws = CONVERT.from_bytes(aStr);

  return ws.size();
};

// compare https://en.wikipedia.org/wiki/Newline#Unicode
std::u32string NEWLINE[] = {
  U"\u000D\u000A", // Carriage Return & Line Feed
  U"\u000A",       // Line Feed
  U"\u000B",       // Vertical Tab
  U"\u000C",       // Form Feed
  U"\u000D",       // Carriage Return
  U"\u0085",       // Next Line
  U"\u2028",       // Line Separator
  U"\u2029",       // Paragraph Separator
  U"^M"            // sometimes indicating that mixed newline symbols were used
};

std::string
computeSplit(const std::string& aLabel,
             const std::unordered_set<char32_t>& aDelims)
{

  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
  std::u32string tmpLabel = convert.from_bytes(aLabel);
  std::u32string result = tmpLabel;

  bool newlineInfoPresent = false;
  for (std::u32string newline : NEWLINE) {
    while (tmpLabel.find(newline) != tmpLabel.npos) {
      newlineInfoPresent = true;
      tmpLabel = tmpLabel.replace(tmpLabel.find(newline), newline.size(), U"%");
    }
  }

  if (newlineInfoPresent) {
    std::unordered_set<char32_t> delim;
    delim.insert(U'%');
    return computeSplit(convert.to_bytes(tmpLabel), delim);
  } else {
    std::size_t centerPos = (tmpLabel.size() + 1) / 2; // ceil division value
    std::size_t pos = 0;

    while (pos < centerPos) {
      char32_t c = tmpLabel[centerPos + pos];
      if (aDelims.count(c) > 0) {
        std::size_t occ = tmpLabel.find(U"%");
        while (occ != tmpLabel.npos) {
          tmpLabel = tmpLabel[occ] = U' ';
          occ = tmpLabel.find(U"%");
        }
        result = tmpLabel.substr(0, centerPos + pos + 1) + U"%" +
                 tmpLabel.substr(centerPos + pos + 1, tmpLabel.size());
        break;
      }
      c = tmpLabel[centerPos - pos];
      if (aDelims.count(c) > 0) {
        std::size_t occ = tmpLabel.find(U"%");
        while (occ != tmpLabel.npos) {
          tmpLabel = tmpLabel[occ] = U' ';
          occ = tmpLabel.find(U"%");
        }
        result = tmpLabel.substr(0, centerPos - pos + 1) + U"%" +
                 tmpLabel.substr(centerPos - pos + 1, tmpLabel.size());
        break;
      }

      ++pos;
    }
  }
  if (result.find(U" %") != result.npos)
    result = result.replace(result.find(U" %"), 2, U"%");
  if (result.find(U"% ") != result.npos)
    result = result.replace(result.find(U"% "), 2, U"%");

  return convert.to_bytes(result);
};

double
computeBallRadius(const std::string& aLabel)
{
  std::size_t delimPos = aLabel.find("%");
  if (delimPos == aLabel.npos)
    delimPos = aLabel.size();

  std::size_t labelSize =
    (delimPos > aLabel.size() / 2) ? delimPos : aLabel.size() - delimPos;
  return (double)labelSize / 2;
}
}

bool
osm_input::OsmPoi::hasIcon() const
{
  return mPoiLevel->mIconName != "";
}

// osm_input::OsmPoi::LabelBall osm_input::OsmPoi::getCorrespondingBall(
//     std::size_t aSplitSize, const std::unordered_set<char32_t> &aDelims)
//     const {
//   std::string label;
//   double ballRadius = 4;
//   if (mPoiLevel->mIconName != "") {
//     label = "icon:" + mPoiLevel->mIconName;
//   } else {
//     if (osmpoi::computeLengthUTF8(getName()) > aSplitSize) {
//       label = osmpoi::computeSplit(getName(), aDelims);
//     } else {
//       label = getName();
//     }
//
//     ballRadius = osmpoi::computeBallRadius(label);
//   }
//
//   ballRadius *= mPoiLevel->mLevelFactor;
//
//   return LabelBall(mPos, mOsmId, ballRadius, label, mPoiLevel->mLevelFactor);
// }

const mapping_helper::MappingHelper::Level*
osm_input::OsmPoi::getLevel() const
{
  return mPoiLevel;
}

const std::vector<osm_input::Tag>&
osm_input::OsmPoi::getTags() const
{
  return mTags;
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
