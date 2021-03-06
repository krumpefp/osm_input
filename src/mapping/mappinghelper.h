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

#include <limits>
#include <list>
#include <stdint.h>
#include <string>
#include <unordered_set>
#include <vector>

#include <json/json.h>
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
    static const uint64_t UNDEFINED_ID = 0;

    std::string mName;
    uint64_t mLevelId;

    int32_t mLevelFactor;
    std::string mIconName;

    std::vector<Constraint> mConstraints;

    Level();

    Level(const std::vector<Constraint>& aConstraints,
          const Json::Value& aJson,
          uint64_t aId);
    Level(Level&& aOther);

    bool hasIcon() const;
    bool isUndefinedLvl() const;

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
  MappingHelper();
  MappingHelper(std::string& aInputPath);
  MappingHelper(const Json::Value& aMapping);
  MappingHelper(const MappingHelper&) = delete;
  MappingHelper(MappingHelper&& aOther);

  MappingHelper& operator=(MappingHelper&& aOther);

  const Level* computeLevel(const std::vector<osm_input::Tag>& aTags) const;

  std::vector<const Level*> getLevels() const;
  const Level* getLevelDefault() const;

  const std::unordered_set<std::string>& get_tag_key_set() const;

  void test();

private:
  struct LevelTree
  {
  public:
    LevelTree(LevelTree* aParent,
              const Json::Value& aData,
              const std::vector<Constraint>& aParentConstraints,
              uint32_t& aNodeId);

    const Level* computeLevel(const std::vector<osm_input::Tag>& aTags,
                              const Level* aDefault) const;

    void computeLevelList(std::vector<const Level*>& aLevels) const;
    std::size_t computeTreeSize() const;

    std::string toString(std::size_t aDepth) const;

  private:
    LevelTree* mParent;
    std::vector<LevelTree> mChildren;
    const Level* mLevel;

    bool mIsLeaf;
    std::string mName;
    uint64_t mNodeId;
    std::vector<Constraint> mConstraints;
  };

  std::size_t mCountLevels;
  LevelTree* mLevelTree;
  const Level* mDefaultLevel;
  std::unordered_set<std::string> m_required_tag_keys;
};
} // namespace mapping_helper

#endif
