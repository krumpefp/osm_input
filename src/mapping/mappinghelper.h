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

#include <list>
#include <stdint.h>
#include <string>
#include <vector>

#include "json.h"
#include "tag.h"

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

    ConstraintType mType;
    std::string mTag;
    int32_t mNumericComp = 0;
    std::string mStringComp = "";

    Constraint(const Json::Value& aJson);

    std::string toString() const;
  };

  struct Level
  {

    std::string mName;
    uint64_t mLevelId;

	int32_t mLevelFactor;
	std::string mIconName;

    std::vector<Constraint> mConstraints;
    Level(const std::vector<Constraint>& aConstraints, const Json::Value& aJson,
          uint64_t aId);

	std::string toString() const;

	// comparison operators
	bool operator==(const Level& aOther) const;
	bool operator!=(const Level& aOther) const;
	bool operator<(const Level& aOther) const;
	bool operator>(const Level& aOther) const;
	bool operator<=(const Level& aOther) const;
	bool operator>=(const Level& aOther) const;
  };

public:
  MappingHelper(std::string& aInputPath);
  MappingHelper( const MappingHelper& ) = delete;
  MappingHelper( MappingHelper&& ) = delete;


  const Level& computeLevel(const std::vector<osm_input::Tag>& aTags) const;

  const std::list<Level>& getLevelList() const;

  void test();

private:
  struct LevelTree
  {
  public:
    LevelTree(const LevelTree* aParent, const Json::Value& aData,
              const std::vector<Constraint>& aParentConstraints,
              std::list<Level>& aLevelList, uint32_t& aNodeId);

    const Level& computeLevel(const std::vector<osm_input::Tag>& aTags,
                              const Level& aDefault) const;

    std::string toString(std::size_t aDepth) const;

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
  std::list<Level> mLevelList;
};
}

#endif
