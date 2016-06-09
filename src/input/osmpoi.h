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

namespace osm_input {

const double COORDINATE_SCALING = 10000000;

class OsmPoi
{
public:
  enum Poi_Types
  {
    GENERAL_POI,
    SETTLEMENT,
    UNDEFINED
  };

  struct Position
  {
    double mLat;
    double mLon;

    Position()
      : mLat(0)
      , mLon(0){};

    Position(double aLat, double aLon)
      : mLat(aLat)
      , mLon(aLon){};

    double getLatDegree() { return mLat; };
    double getLonDegree() { return mLon; };
  };

  struct LabelBall
  {
    Position mPos;
    double mBallRadius;
    
    std::string mLabel;
    double mLabelFactor;

    LabelBall(const Position& aCenter, int32_t aRadius, std::string aLabel, double aFactor)
      : mPos(aCenter)
      , mBallRadius(aRadius)
      , mLabel(aLabel)
      , mLabelFactor(aFactor) {};
  };

public:
  OsmPoi(int64_t aOsmId, Position aPos);
  OsmPoi(int64_t aOsmId, Position aPos, Poi_Types aType);

  // comparison operators
  bool operator==(const OsmPoi& aOther) const;
  bool operator!=(const OsmPoi& aOther) const;
  bool operator<(const OsmPoi& aOther) const;
  bool operator>(const OsmPoi& aOther) const;
  bool operator<=(const OsmPoi& aOther) const;
  bool operator>=(const OsmPoi& aOther) const;

  void addTag(std::string aKey, std::string aValue);
  Poi_Types computeType(bool aUpdateType);

  int64_t getOsmId() const { return mOsmId; };
  Position getPosition() const { return mPos; };

  LabelBall getCorrespondingBall(std::size_t aSplitSize,
                                 const std::unordered_set<char>& aDelims) const;

  std::string getTagValue(std::string aTagName) const;
  
  void setLabelFactor(double aFactor);

private:
  int64_t mOsmId;
  Position mPos;
  Poi_Types mPoiType;
  int32_t mSubImportance = -1;
  double mLabelFactor = 1;

  std::vector<std::pair<std::string, std::string>> mTags;
};
}

#endif // OSMPOI_H
