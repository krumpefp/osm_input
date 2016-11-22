/*
 * Class to store font information and provide methods to compute label size and
 * stuff like that
 *
 * Copyright (C) 2016  Filip Krumpe <filip.krumpe@fmi.uni-stuttgart.de>
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

#include "font.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "json.h"

namespace font {
Json::Value importFont(std::string aConfigPath) {
  Json::Value config;

  std::ifstream configFile(aConfigPath, std::ifstream::binary);

  configFile >> config;

  return config;
}
}

label::Font::Font(std::string configPath) {
  Json::Value fontConfig = font::importFont(configPath);

  Json::Value info = fontConfig.get("font", "undefined");

  std::cout << "Started font config import: "
            << info.get("name", "undefined").asString() << " - "
            << info.get("style", "undefined").asString() << std::endl;

  // insert the alphabet
  std::string alphabet = fontConfig.get("alphabet", "").asString();
  if (alphabet == "") {
    throw std::invalid_argument(
        "Given font alphabet does not contain any letters!");
  }

  std::cout << "Alphabet: '" << alphabet << "'." << std::endl;
}
