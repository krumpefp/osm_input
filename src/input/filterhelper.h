/*
 * Create a filter to restrict elements visited during the pbf parsing
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

#ifndef FILTERHELPER_H
#define FILTERHELPER_H

#include <json/json.h>
#include "osmpbf/filter.h"

namespace filter_helper {

class FilterHelper
{
public:
  FilterHelper();

  FilterHelper(const Json::Value& j_filter);

  FilterHelper(const FilterHelper& aOther);

  FilterHelper& operator=(const FilterHelper& aOther);

  osmpbf::RCFilterPtr get_filter() const;

private:
  osmpbf::RCFilterPtr m_filter;
};
}

#endif // FILTERHELPER_H
