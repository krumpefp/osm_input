/*
 * Import poi data from osm data sets
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

#include "filterhelper.h"

#include <assert.h>
#include <iostream>

namespace filter_helper {
namespace {

osmpbf::AbstractTagFilter*
create_filter(Json::Value j_filter)
{
  auto type = j_filter["type"].asString();
  if (type == "value") {
    return new osmpbf::KeyOnlyTagFilter(j_filter["value"].asString());
  }

  // if j_filter is not a string, it is an object and has the type and operands
  // field
  osmpbf::AbstractMultiTagFilter* filter = nullptr;
  if (type == "and") {
    filter = new osmpbf::AndTagFilter();
  } else if (type == "or") {
    filter = new osmpbf::OrTagFilter();
  } else {
    std::cout << "Found unknown filter type " << type << std::endl;
    assert(false);
  }
  for (auto& v : j_filter["operands"]) {
    filter->addChild(create_filter(v));
  }

  return filter;
}
}

FilterHelper::FilterHelper()
  : m_filter(){};

FilterHelper::FilterHelper(const Json::Value& j_filter)
  : FilterHelper()
{
  if (j_filter.isNull()) {
    return;
  }

  m_filter.reset(create_filter(j_filter));
}

FilterHelper::FilterHelper(const FilterHelper& aOther)
  : m_filter(aOther.m_filter->copy()){};

FilterHelper&
FilterHelper::operator=(const FilterHelper& aOther)
{
  m_filter.reset(aOther.m_filter->copy());
  return *this;
}

osmpbf::RCFilterPtr
FilterHelper::get_filter() const
{
  return osmpbf::RCFilterPtr(m_filter->copy());
}
}
