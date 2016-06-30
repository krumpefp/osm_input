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

#include "mappinghelper.h"

#include <fstream>

#include "json.h"

typedef mapping_helper::MappingHelper::Constraint Constraint;
typedef mapping_helper::MappingHelper::Level Level;

mapping_helper::MappingHelper::Constraint::Constraint(const Json::Value& aJson)
{
  if (aJson.isMember("equals")) {
    mType = Constraint::ConstraintType::EQUALS;
    mStringComp = aJson["equals"].asString();
  } else if (aJson.isMember("greater")) {
    mType = Constraint::ConstraintType::GREATER;
    mNumericComp = aJson["greater"].asInt();
  } else if (aJson.isMember("less")) {
    mType = Constraint::ConstraintType::LESS;
    mNumericComp = aJson["less"].asInt();
  } else if (aJson.isMember("tag")) {
    mType = Constraint::ConstraintType::TAG;
  }

  mTag = aJson["tag"].asString();
}

std::string
mapping_helper::MappingHelper::Constraint::toString() const
{
  std::string result = "";
  switch (mType) {
    case Constraint::ConstraintType::EQUALS: {
      result = "(tag " + mTag + " == " + mStringComp + ")";
      break;
    }
    case Constraint::ConstraintType::GREATER: {
      result = "(tag " + mTag + " >= " + std::to_string(mNumericComp) + ")";
      break;
    }
    case Constraint::ConstraintType::LESS: {
      result = "(tag " + mTag + " < " + std::to_string(mNumericComp) + ")";
      break;
    }
    case Constraint::ConstraintType::TAG: {
      result = "(tag " + mTag + " exists)";
      break;
    }
  }

  return result;
}

mapping_helper::MappingHelper::Level::Level(
  const std::vector<Constraint>& aConstraints, const Json::Value& aJson,
  uint64_t aId)
{
  mName = aJson["level"].asString();
  mLevelId = aId;
  mConstraints = aConstraints;
  if (aJson.isMember("constraints")) {
    for (const auto& c : aJson["constraints"]) {
      mConstraints.emplace_back(c);
    }
  }
}

std::string
mapping_helper::MappingHelper::Level::toString() const
{
  std::string constraints = "";
  for (const auto& c : mConstraints) {
    if (constraints != "")
      constraints += ", ";
    constraints += c.toString();
  }
  return "(Level name " + mName + ", constraints: [" + constraints + "])";
}

mapping_helper::MappingHelper::LevelTree::LevelTree(
  const LevelTree* aParent, const Json::Value& aData,
  const std::vector<Constraint>& aParentConstraints,
  std::vector<Level>& aLevels, uint64_t aNodeId)
  : mParent(aParent)
  , mLevel(nullptr)
{
  mName = aData["level"].asString();
  mNodeId = aNodeId;

  // handle the level constraints
  if (aData.isMember("constraints")) {
    for (const auto& c : aData["constraints"]) {
      mConstraints.emplace_back(c);
    }
  }

  // handle the sublevels and construct corresponding subtrees
  if (aData.isMember("sublevels")) {
    mIsLeaf = false;

    std::vector<Constraint> localConstraints = aParentConstraints;
    localConstraints.insert(localConstraints.begin(), mConstraints.begin(),
                            mConstraints.end());
    uint32_t aChildId = 1;
    for (const auto& lvl : aData["sublevels"]) {
      mChildren.emplace_back(this, lvl, localConstraints, aLevels,
                             aNodeId * 100 + aChildId);
      ++aChildId;
    }
  } else {
    // ... if no sublevels are available construct a leaf node
    mIsLeaf = true;
    aLevels.emplace_back(aParentConstraints, aData, aNodeId);
    mLevel = &aLevels.at(aLevels.size() - 1);
  }
}

std::string
mapping_helper::MappingHelper::LevelTree::toString(int32_t aDepth) const
{
  std::string constraints = "";
  for (const auto& c : mConstraints) {
    if (constraints != "")
      constraints += ", ";
    constraints += c.toString();
  }
  std::string prefix = std::string(aDepth, '\t');

  std::string result = "";
  if (mIsLeaf) {
    result = "\n" + prefix + "LEAF " + std::to_string(mNodeId) + " '" + mName +
             "', constraints: [" + constraints + "]";
  } else {
    std::string subtree = "";
    for (const auto& child : mChildren) {
      subtree += child.toString(aDepth + 1);
    }
    result = "\n" + prefix + "SUBTREE " + std::to_string(mNodeId) + " '" +
             mName + "', constraints: [" + constraints + "]: " + subtree;
  }

  return result;
}

mapping_helper::MappingHelper::MappingHelper(std::string& aInputPath)
{
  std::ifstream inputFile(aInputPath);
  if (!inputFile.is_open()) {
    std::printf("[ERROR] Input file %s could not be opened!\n",
                aInputPath.c_str());
    return;
  }

  Json::Value root;
  Json::Reader inputReader;
  inputReader.parse(inputFile, root);
  mLevelTree =
    new LevelTree(nullptr, root, std::vector<Constraint>(), mLevelList, 0);

  printf("%s\n", mLevelTree->toString(0).c_str());
}
