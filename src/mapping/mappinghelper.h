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
      DEFAULT,
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
    Constraint(const Json::Value& aJson);

    std::string toString() const;
  };

  struct Level
  {
  private:
    std::string mName;
    uint64_t mLevelId;
    std::vector<Constraint> mConstraints;

  public:
    Level(const std::vector<Constraint>& aConstraints, const Json::Value& aJson,
          uint64_t aId);

    std::string toString() const;
  };

public:
  MappingHelper(std::string& aInputPath);

private:
  struct LevelTree
  {
  public:
    LevelTree(const LevelTree* aParent, const Json::Value& aData,
              const std::vector<Constraint>& aParentConstraints,
              std::vector<Level>& aLevelList, uint64_t aNodeId);

    std::string toString(int32_t aDepth) const;

  private:
    const LevelTree* mParent;
    std::vector<LevelTree> mChildren;
    Level* mLevel;

    bool mIsLeaf;
    std::string mName;
    uint64_t mNodeId;
    std::vector<Constraint> mConstraints;
  };

  LevelTree* mLevelTree;
  std::vector<Level> mLevelList;
};
}

#endif
