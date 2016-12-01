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

#ifndef OSMPOI_H
#define OSMPOI_H

#include <stdint.h>
#include <string>
#include <unordered_set>
#include <vector>

#include "mappinghelper.h"
#include "tag.h"

namespace osm_input {

class OsmPoi {
public:
  struct Position {
    double mLat;
    double mLon;

    Position() : mLat(0), mLon(0){};

    Position(double aLat, double aLon) : mLat(aLat), mLon(aLon){};

    double getLatDegree() const { return mLat; };
    double getLonDegree() const { return mLon; };
  };

public:
  OsmPoi(int64_t aOsmId, osm_input::OsmPoi::Position aPos,
         const std::vector<osm_input::Tag> &aTags,
         const mapping_helper::MappingHelper &aMh);

  OsmPoi(int64_t aOsmId, osm_input::OsmPoi::Position aPos,
         const std::vector<osm_input::Tag> &aTags,
         const mapping_helper::MappingHelper::Level *aLvl);

  // comparison operators
  bool operator==(const OsmPoi &aOther) const;
  bool operator!=(const OsmPoi &aOther) const;
  bool operator<(const OsmPoi &aOther) const;
  bool operator>(const OsmPoi &aOther) const;
  bool operator<=(const OsmPoi &aOther) const;
  bool operator>=(const OsmPoi &aOther) const;

  int64_t getOsmId() const { return mOsmId; };
  Position getPosition() const { return mPos; };

  const mapping_helper::MappingHelper::Level *getLevel() const;

  const std::vector<osm_input::Tag> &getTags() const;
  std::string getTagValue(std::string aTagName) const;

  std::string getName() const;

  bool hasIcon() const;

private:
  int64_t mOsmId;
  Position mPos;
  const mapping_helper::MappingHelper::Level *mPoiLevel;

  std::vector<Tag> mTags;
};
}

#endif // OSMPOI_H
