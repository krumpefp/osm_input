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

statistics::PoiStatistics::PoiStatistics(
    std::vector<osm_input::OsmPoi *> &aPois)
    : mPois(aPois) {}

namespace {
struct ClassStatistics {
  const mapping_helper::MappingHelper::Level &mLevel;

  std::size_t mCount;

    ClassStatistics (const mapping_helper::MappingHelper::Level &aLevel);

  void addPoi(const osm_input::OsmPoi &aPoi);

  std::string toString() const;
};

ClassStatistics::ClassStatistics (
    const mapping_helper::MappingHelper::Level &aLevel)
    : mLevel(aLevel), mCount(0) {}

void ClassStatistics::addPoi(const osm_input::OsmPoi &aPoi) { ++mCount; }

std::string ClassStatistics::toString() const {
  std::string result = "";

  result += "Level " + mLevel.mName + " with id: " +
            std::to_string(mLevel.mLevelId);
  result += "\tcontains " + std::to_string(mCount) + " elements";

  return result;
}
}

std::string statistics::PoiStatistics::mappingStatistics(
    const mapping_helper::MappingHelper &aMapping) const {
  std::map<uint64_t, ClassStatistics> statsMap;
  for (const auto& lvl : aMapping.getLevelList()) {
      statsMap.emplace(lvl.mLevelId, ClassStatistics (lvl));
    }
  
  for (const auto& poi : mPois) {
      statsMap.at(poi->getLevel().mLevelId).addPoi(*poi);
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
    struct TagHash {
        std::size_t operator()(const osm_input::Tag& aTag) const {
            std::string concat = aTag.mKey + aTag.mValue;
            
            return std::hash<std::string>{}(concat);
        }
    };
    
    struct TagStatisticsDetails {
        osm_input::Tag mTag;
        std::size_t mCount;
        
        TagStatisticsDetails(const osm_input::Tag& aTag)
        : mTag(aTag), mCount(0) {}
        
        void addTag(const osm_input::Tag& aTag) {
            assert(aTag.mKey == mTag.mKey && aTag.mValue == mTag.mValue);
            ++mCount;
        }
    };
    
    struct TagStatisticsSimple {
        std::string mTagKey;
        std::size_t mCount;
        
        TagStatisticsSimple(const osm_input::Tag& aTag)
        : mTagKey(aTag.mKey), mCount(0) {}
        
        void addTag(const osm_input::Tag& aTag) {
            assert(aTag.mKey == mTagKey);
            ++mCount;
        }
    };
}

std::string statistics::PoiStatistics::tagStatisticsSimple() const
{
    std::unordered_map<std::string, TagStatisticsSimple> stats;
    for (const auto& poi : mPois)  {
        for (const auto& tag : poi->getTags()) {
            auto s = stats.find(tag.mKey);
            if (s == stats.end()) {
                stats.emplace(tag.mKey, tag);
                s = stats.find(tag.mKey);
            }
            assert(s != stats.end());
            
            s->second.addTag(tag);
        }
    }
    
    std::string result = "Tag Statistics:";
    for (const auto& s : stats) {
        result += "\n"+std::to_string(s.second.mCount)+"\t-\t"+s.second.mTagKey;
    }
    
    return result;
}


std::string statistics::PoiStatistics::tagStatisticsDetailed() const
{
    std::unordered_map<osm_input::Tag, TagStatisticsDetails, TagHash> stats;
    for (const auto& poi : mPois)  {
        for (const auto& tag : poi->getTags()) {
            auto s = stats.find(tag);
            if (s == stats.end()) {
                stats.emplace(tag, tag);
                s = stats.find(tag);
            }
            assert(s != stats.end());
            
            s->second.addTag(tag);
        }
    }
    
    std::string result = "Tag Statistics:";
    for (const auto& s : stats) {
        result += "\n"+std::to_string(s.second.mCount)+"\t-\t("+s.second.mTag.mKey+", "+s.second.mTag.mValue+")";
    }
    
    return result;
}
