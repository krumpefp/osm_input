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

#ifndef POPULATIONINPUT_H
#define POPULATIONINPUT_H

#include <map>
#include <stdint.h>
#include <string>

namespace pop_input {

class PopulationInput
{
public:
  PopulationInput(std::string aInputPath);
  PopulationInput(const PopulationInput& other) = delete;
  PopulationInput& operator=(const PopulationInput& other) = delete;
  bool operator==(const PopulationInput& other) const = delete;

  std::map<std::string, int32_t> getPopulationsMap() const
  {
    return mPopulations;
  };

private:
  std::map<std::string, int32_t> mPopulations;
};
}

#endif // POPULATIONINPUT_H
