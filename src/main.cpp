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

#include <algorithm>
#include <boost/iterator/iterator_concepts.hpp>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

#include "mappinghelper.h"
#include "osminputhelper.h"
#include "osmpoi.h"
#include "poistatistics.h"
#include "populationinput.h"
#include "textoutputhelper.h"
#include "timer.h"

namespace {
const std::size_t SPLIT_SIZE = 15;
const std::unordered_set<char> DELIMITERS({' ', '-', '/'});

bool osmPoiComparatorASC(const osm_input::OsmPoi &aLhs,
                         const osm_input::OsmPoi &aRhs) {
  return aLhs > aRhs;
}
}

int main(int argc, char **argv) {
  if (argc < 3) {
    std::printf("To few arguments given.\nPlease use: osm_input <osm.pbf> "
                "<mapping> <population (optional)>\n");
    return 0;
  }

  std::string pbfPath;
  std::string jsonPath;
  std::string popPath;

  debug_timer::Timer t;

  pbfPath = std::string(argv[1]);
  jsonPath = std::string(argv[2]);

  t.start();
  osm_input::OsmInputHelper input(pbfPath, jsonPath);
  std::vector<osm_input::OsmPoi> pois;
  if (argc > 3) {
    std::map<std::string, int32_t> populations;
    popPath = std::string(argv[2]);
    pop_input::PopulationInput popInput(popPath);
    populations = popInput.getPopulationsMap();
    pois = input.importPoiData(true, true, populations);
  } else {
    pois = input.importPoiData(true, true);
  }

  t.createTimepoint();

  std::sort(pois.begin(), pois.end(), osmPoiComparatorASC);

  t.stop();

  std::printf("Dataset of size: %lu\t was imported within %4.2f "
              "seconds.\n\tSorting objects took %4.2f seconds.\n",
              pois.size(), t.getTimes()[0], t.getTimes()[1]);

  statistics::PoiStatistics stats(pois);
  printf("%s\n", stats.mappingStatistics(input.getMappingHelper()).c_str());
  printf("%s\n", stats.tagStatisticsSimple().c_str());

  std::vector<osm_input::OsmPoi::LabelBall> balls;
  balls.reserve(pois.size());
  for (auto it = pois.begin(), end = pois.end(); it != end;) {
    balls.push_back(it->getCorrespondingBall(SPLIT_SIZE, DELIMITERS));
    ++it;
  }

  std::string outputname =
      (pbfPath.find("/") == pbfPath.npos)
          ? pbfPath.substr(0, pbfPath.size())
          : pbfPath.substr(pbfPath.rfind('/') + 1, pbfPath.size());

  std::string outputpath = outputname + ".balls.txt";
  std::printf("Outputting data to %s\n", outputpath.c_str());
  text_output::TextOutputHelper out(outputpath);
  out.writeBallsFile(balls, ' ');

  outputpath = outputname + ".complete.txt";
  std::printf("Outputting data to %s\n", outputpath.c_str());
  text_output::TextOutputHelper outComplete(outputpath);
  outComplete.writeCompleteFile(pois, SPLIT_SIZE, DELIMITERS, ' ');
}
