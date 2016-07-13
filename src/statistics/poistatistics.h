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

#ifndef POISTATISTICS_H
#define POISTATISTICS_H

#include <map>
#include <stdint.h>
#include <string>

#include "mappinghelper.h"
#include "osmpoi.h"

namespace statistics {

class PoiStatistics
{
private:
  struct StatisticElement
  {
    const mapping_helper::MappingHelper::Level& mLevel;

    std::size_t mCount;

    StatisticElement(const mapping_helper::MappingHelper::Level& aLevel);

    void addPoi(const osm_input::OsmPoi& aPoi);

    std::string toString() const;
  };

public:
  PoiStatistics(const mapping_helper::MappingHelper& aMapping,
                std::vector<osm_input::OsmPoi*>& aPois);

  std::string toString() const;

private:
  std::map<uint64_t, StatisticElement> mStatsMap;
};
}

#endif
