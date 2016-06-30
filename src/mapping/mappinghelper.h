/*
 * Map osm pois to a class depending on their specific tag set
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

#ifndef MAPPINGHELPER_H
#define MAPPINGHELPER_H

#include <stdint.h>
#include <string>
#include <vector>

#include "json.h"

namespace mapping_helper {
class MappingHelper
{
public:
  struct Constraint
  {
    enum ConstraintType
    {
      EQUALS,
      GREATER,
      LESS,
      TAG
    };

  private:
    ConstraintType mType;
    std::string mTag;
    int32_t mNumericComp = 0;
    std::string mStringComp = "";

  public:
    Constraint(Json::Value& aJson);

    std::string toString() const;
  };

  struct Level
  {
  private:
    std::string mName;
    std::vector<Constraint> mConstraints;

  public:
    Level(const std::vector<Constraint>& aConstraints, Json::Value& aJson);

    std::string toString() const;
  };

  MappingHelper(std::string& aInputPath);

private:
  std::vector<Level> mLevels;
};
}

#endif
