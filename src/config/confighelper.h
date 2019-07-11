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

#ifndef CONFIGHELPER_H
#define CONFIGHELPER_H

#include <list>
#include <stdint.h>
#include <string>
#include <unordered_set>

#include <json/json.h>

#include "filterhelper.h"
#include "mappinghelper.h"

namespace config_helper {
class ConfigHelper
{
public:
  ConfigHelper(std::string aInputPath);

  // getters
  std::string get_labeling_name() const;
  std::string get_labeling_description() const;
  uint32_t get_split_bound() const;
  const std::unordered_set<char32_t> get_split_delimiters() const;
  std::string get_font_name() const;
  std::string get_ttf_path() const;

  const mapping_helper::MappingHelper& get_mapping_helper() const;
  const filter_helper::FilterHelper& get_filter_helper() const;

private:
  std::string m_labeling_name;
  std::string m_description;

  uint32_t m_split_bound;
  std::unordered_set<char32_t> m_split_delimiters;

  std::string m_font_name;
  std::string m_font_ttf_path;

  mapping_helper::MappingHelper m_mapping_helper;
  filter_helper::FilterHelper m_filter_helper;
};
}

#endif // CONFIGHELPER_H
