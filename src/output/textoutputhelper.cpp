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

#include "textoutputhelper.h"

#include "osmpoi.h"

bool
text_output::TextOutputHelper::writeBallsFile(
  std::vector<osm_input::OsmPoi::LabelBall>& aBalls)
{
  std::ofstream file(mOutputPath.c_str());

  if (!file.is_open()) {
    return false;
  }
  file.clear();

  std::size_t importance = 0;
  file << "Longitude [-180, 180] Latitude [-90, 90] Importance Radius";

  for (auto& ball : aBalls) {
    file << "\n"
         << ball.mPos.getLonDegree() << "\t" << ball.mPos.getLatDegree() << "\t"
         << importance++ << "\t" << ball.mBallRadius;
  }

  file.close();
  
  return true;
}

bool
text_output::TextOutputHelper::writeCompleteFile(
  std::vector< const osm_input::OsmPoi* >& aPois, std::size_t aSplitSize, const std::unordered_set< char >& aDelimiters)
{
  std::ofstream file(mOutputPath.c_str());

  if (!file.is_open()) {
    return false;
  }
  file.clear();

  std::size_t importance = 0;
  file
    << "Longitude [-180, 180] Latitude [-90, 90] Importance Radius OsmID name";

  for (auto& poi : aPois) {
    osm_input::OsmPoi::LabelBall ball =
      poi->getCorrespondingBall(aSplitSize, aDelimiters);

      file << "\n"
           << ball.mPos.getLonDegree() << "\t" << ball.mPos.getLatDegree() << "\t"
           << importance++ << "\t" << ball.mBallRadius << "\t" << poi->getOsmId()
           << "\t" << poi->getTagValue("name");
  }

  file.close();
  
  return true;
}
