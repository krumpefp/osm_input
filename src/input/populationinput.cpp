/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2016  Filip Krumpe <filip.krumpe@posteo.de>
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

#include "populationinput.h"

#include <fstream>
#include <sstream>
#include <vector>

namespace popinput {
std::vector<std::string>
split(const std::string& aInput, char aDelim)
{
  std::vector<std::string> result;

  std::stringstream stream(aInput);
  for (std::string item; std::getline(stream, item, aDelim);) {
    if (!item.empty())
      result.push_back(item);
  }

  return result;
}
}

pop_input::PopulationInput::PopulationInput(std::string aInputPath)
{
  std::ifstream file(aInputPath);

  if (!file.is_open()) {
    std::printf("File %s could not be opened!\n", aInputPath.c_str());
    return;
  }

  std::string line;
  for (std::string line; std::getline(file, line);) {
    if (line.substr(0, 1) == "#")
      continue;
    auto splitted = popinput::split(line, '\t');

    std::string name = splitted[0];
    int32_t population = std::atoi(splitted[1].c_str());

    mPopulations.insert(std::make_pair(name, population));
  }
}
