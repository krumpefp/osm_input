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

mapping_helper::MappingHelper::Constraint::Constraint(Json::Value& aJson)
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
  const std::vector<Constraint>& aConstraints, Json::Value& aJson)
{
  mName = aJson["level"].asString();
  mConstraints = aConstraints;
  if (aJson.isMember("constraints")) {
    for (auto& c : aJson["constraints"]) {
      mConstraints.emplace_back(c);
    }
  }
}

std::string
mapping_helper::MappingHelper::Level::toString() const
{
  std::string constraints = "";
  for (auto& c : mConstraints) {
    if (constraints != "")
      constraints += ", ";
    constraints += c.toString();
  }

  return "(Level name " + mName + ", constraints: [" + constraints + "])";
}

namespace mappinghelper {
std::vector<Level>
importLevel(Json::Value aJson, const std::vector<Constraint>& aConstraints)
{
  std::vector<Level> result;
  if (!aJson.isMember("sublevels")) {
    result.emplace_back(aConstraints, aJson);
  } else {
    std::vector<Constraint> localConstraints = aConstraints;
    if (aJson.isMember("constraints")) {
      for (auto& c : aJson["constraints"]) {
        localConstraints.emplace_back(c);
      }
    }
    for (auto& lvl : aJson["sublevels"]) {
      auto sublevels = importLevel(lvl, localConstraints);
      result.insert(result.end(), sublevels.begin(), sublevels.end());
    }
  }
  return result;
}

std::vector<Level>
importLevels(std::string& aInputPath)
{
  std::vector<Level> result;

  std::ifstream inputFile(aInputPath);
  if (!inputFile.is_open()) {
    std::printf("[ERROR] Input file %s could not be opened!\n",
                aInputPath.c_str());
    return result;
  }

  Json::Value root;
  Json::Reader inputReader;
  inputReader.parse(inputFile, root);

  result = importLevel(root, std::vector<Constraint>());

  return result;
}
}

mapping_helper::MappingHelper::MappingHelper(std::string& aInputPath)
{
  mLevels = mappinghelper::importLevels(aInputPath);

  int counter = 1;
  for (Level& lvl : mLevels) {
    printf("Level %d: %s\n", counter, lvl.toString().c_str());
    ++counter;
  }
}
