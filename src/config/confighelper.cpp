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

#include "confighelper.h"

#include <assert.h>
#include <fstream>
#include <iostream>
#include <stdexcept>

// BEGIN ConfigHelper

namespace config_helper {
ConfigHelper::ConfigHelper(std::string input_path)
{
  std::ifstream inputFile(input_path);
  if (!inputFile) {
    throw std::runtime_error("Config file " + input_path +
                             " could not be opened!\n");
  }

  Json::Value root;
  Json::Reader inputReader;

  inputReader.parse(inputFile, root);

  m_labeling_name = root["labeling_name"].asString();
  m_description = root["description"].asString();

  assert(root["label_split"]["split_bound"].asInt() > 0);
  m_split_bound = (uint32_t)root["label_split"]["split_bound"].asInt();
  auto split_delims = root["label_split"]["split_chars"];
  assert(split_delims != Json::nullValue);
  for (auto split_char : split_delims) {
    auto split_str = split_char.asString();
    if (split_str.length() != 1) {
      std::cout << "Split delimiters are only allowed to be single characters!\
	      \nIgnoring split delimiter '" +
                     split_str + "'!"
                << std::endl;
      continue;
    }
    m_split_delimiters.insert(split_str.at(0));
  }

  m_font_name = root["font"]["name"].asString();
  m_font_ttf_path = root["font"]["ttf-path"].asString();

  m_mapping_helper = mapping_helper::MappingHelper(root["mapping"]);
}

std::string
ConfigHelper::get_labeling_name() const
{
  return m_labeling_name;
}
std::string
ConfigHelper::get_labeling_description() const
{
  return m_description;
}
uint32_t
ConfigHelper::get_split_bound() const
{
  return m_split_bound;
}
const std::unordered_set<char32_t>
ConfigHelper::get_split_delimiters() const
{
  return m_split_delimiters;
}
std::string
ConfigHelper::get_font_name() const
{
  return m_font_name;
}
std::string
ConfigHelper::get_ttf_path() const
{
  return m_font_ttf_path;
}
const mapping_helper::MappingHelper&
ConfigHelper::get_mapping_helper() const
{
  return m_mapping_helper;
}
}
// end ConfigHelper
