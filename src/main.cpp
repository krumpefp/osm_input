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

#include <iostream>
#include <string>

#include "osminputhelper.h"
#include "osmpoi.h"

int
main(int argc, char** argv)
{
  std::printf("Hallo!\n");
  
  std::string path = std::string(argv[1]);
  
  osm_input::OsmInputHelper input (path);
  
  std::vector<const osm_input::OsmPoi*> pois = input.importPoiData(true, true);
  
  std::printf("Dataset size: %lu\n", pois.size());
  
  return 1;
}