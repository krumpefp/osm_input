/*
 * Create some statistics about a poi set
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

#include <assert.h>
#include <unordered_map>

#include "poistatistics.h"

statistics::PoiStatistics::PoiStatistics(std::vector<osm_input::OsmPoi> &aPois)
    : mPois(aPois) {}

namespace {
struct ClassStatistics {
  const mapping_helper::MappingHelper::Level *mLevel;

  std::size_t mCount;

  ClassStatistics(const mapping_helper::MappingHelper::Level *aLevel)
      : mLevel(aLevel), mCount(0){};

  void addPoi(const osm_input::OsmPoi &aPoi);

  std::string toString() const;
};

void ClassStatistics::addPoi(const osm_input::OsmPoi &aPoi) { ++mCount; }

std::string ClassStatistics::toString() const {
  std::string result = "";

  result += "Level " + mLevel->mName + " with id: " +
            std::to_string(mLevel->mLevelId);
  result += "\tcontains " + std::to_string(mCount) + " elements";

  return result;
}
}

std::string statistics::PoiStatistics::mappingStatistics(
    const mapping_helper::MappingHelper &aMapping) const {
  std::map<uint64_t, ClassStatistics> statsMap;
  for (const auto &lvl : aMapping.getLevels()) {
    statsMap.emplace(lvl->mLevelId, ClassStatistics(lvl));
  }

  for (const auto &poi : mPois) {
    statsMap.at(poi.getLevel()->mLevelId).addPoi(poi);
  }

  std::string result = "Poi statistics contains:";
  std::size_t total = 0;

  for (auto &lvl : statsMap) {
    if (lvl.second.mCount != 0) {
      result += "\n" + lvl.second.toString();
      total += lvl.second.mCount;
    }
  }
  result += "\n\t\tTotal:\t" + std::to_string(total);

  return result;
}

namespace {
struct TagStatistics {
  std::string mTagKey;
  std::size_t mCount;
  std::unordered_map<std::string, std::size_t> mDetails;

  TagStatistics(const osm_input::Tag &aTag) : mTagKey(aTag.mKey), mCount(1) {
    mDetails.emplace(aTag.mValue, 1);
  }

  void addTag(const osm_input::Tag &aTag) {
    assert(aTag.mKey == mTagKey);
    ++mCount;

    auto it = mDetails.find(aTag.mValue);
    if (it == mDetails.end()) {
      mDetails.emplace(aTag.mValue, 1);
    } else {
      ++it->second;
    }
  }

  std::string toShortString() const {
    return "Tag: '" + mTagKey + "': #" + std::to_string(mCount);
  }

  std::string toLongString() const {
    std::string result = toShortString();

    for (auto &s : mDetails) {
      result += "\n\tValue: '" + s.first + "': #" + std::to_string(s.second);
    }

    return result;
  }
};

std::unordered_map<std::string, TagStatistics>
computeStatistics(std::vector<osm_input::OsmPoi> &aPois) {
  std::unordered_map<std::string, TagStatistics> result;

  for (const auto &poi : aPois) {
    for (const auto &tag : poi.getTags()) {
      auto s = result.find(tag.mKey);
      if (s == result.end()) {
        result.emplace(tag.mKey, tag);
      } else {
        s->second.addTag(tag);
      }
    }
  }

  return result;
}
}

std::string statistics::PoiStatistics::tagStatisticsSimple() const {
  std::unordered_map<std::string, TagStatistics> stats =
      computeStatistics(mPois);

  std::string result = "Simple tag statistics:";
  for (const auto &s : stats) {
    result += "\n" + s.second.toShortString();
  }

  return result;
}

std::string statistics::PoiStatistics::tagStatisticsDetailed(
    std::size_t aMaxSubSize) const {
  std::unordered_map<std::string, TagStatistics> stats =
      computeStatistics(mPois);

  std::string result = "Detailed tag statistics:";
  for (const auto &s : stats) {
    if (s.second.mDetails.size() > aMaxSubSize) {
      result += "\n" + s.second.toShortString();
      result +=
          "\n\tskipped as >" + std::to_string(aMaxSubSize) + " sub elements";
    } else {
      result += "\n" + s.second.toLongString();
    }
  }

  return result;
}

std::string
statistics::PoiStatistics::tagStatisticsDetailed(double aMinAvgSubSize) const {
  std::unordered_map<std::string, TagStatistics> stats =
      computeStatistics(mPois);

  std::string result = "Detailed tag statistics:";
  for (const auto &s : stats) {
    double avgSubSize =
        (double)s.second.mCount / (double)s.second.mDetails.size();
    if (avgSubSize < aMinAvgSubSize) {
      result += "\n" + s.second.toShortString();
      result += "\n\tskipped as average sub element count <" +
                std::to_string(aMinAvgSubSize);
    } else {
      result += "\n" + s.second.toLongString();
    }
  }

  return result;
}
