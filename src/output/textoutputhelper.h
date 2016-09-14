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

#ifndef TEXTOUTPUTHELPER_H
#define TEXTOUTPUTHELPER_H

#include <fstream>
#include <string>
#include <vector>

#include "osmpoi.h"

namespace text_output {

class TextOutputHelper {
public:
  TextOutputHelper(std::string aOutputPath) : mOutputPath(aOutputPath){};
  TextOutputHelper(const TextOutputHelper &other) = delete;
  TextOutputHelper &operator=(const TextOutputHelper &other) = delete;
  bool operator==(const TextOutputHelper &other) const = delete;

  bool writeBallsFile(std::vector<osm_input::OsmPoi::LabelBall> &aBalls,
                      char aSep);

  bool writeCompleteFile(const std::vector<osm_input::OsmPoi> &aPois,
                         std::size_t aSplitSize,
                         const std::unordered_set<char32_t> &aDelimiters,
                         char aSep);

private:
  std::string mOutputPath;
};
}

#endif // TEXTOUTPUTHELPER_H
