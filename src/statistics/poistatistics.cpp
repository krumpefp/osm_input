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

#include "poistatistics.h"

statistics::PoiStatistics::StatisticElement::StatisticElement(
  const mapping_helper::MappingHelper::Level& aLevel)
  : mLevel(aLevel)
  , mCount(0)
{
}

void
statistics::PoiStatistics::StatisticElement::addPoi(
  const osm_input::OsmPoi& aPoi)
{
  ++mCount;
}

std::string
statistics::PoiStatistics::StatisticElement::toString() const
{
  std::string result = "";

  result += "Level " + mLevel.toString() + "\n";
  result += "\tcontains " + std::to_string(mCount) + " elements";

  return result;
}

statistics::PoiStatistics::PoiStatistics(
  const mapping_helper::MappingHelper& aMapping,
  std::vector<osm_input::OsmPoi*>& aPois)
{
  for (auto& lvl : aMapping.getLevelList()) {
    mStatsMap.insert(std::make_pair(lvl.mLevelId, StatisticElement(lvl)));
  }

  for (auto& poi : aPois) {
    if (poi->getLevel().mName == "MEGA_CITY") {
      printf("Found mega city: %s with id: %ld\n", poi->getName().c_str(),
             poi->getOsmId());
    }

    mStatsMap.at(poi->getLevel().mLevelId).addPoi(*poi);
  }
}

std::string
statistics::PoiStatistics::toString() const
{
  std::string result = "Poi statistics contains:";

  for (auto& lvl : mStatsMap) {
    result += "\n" + lvl.second.toString();
  }

  return result;
}
