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
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

#include "argumentparser.h"

#include "mappinghelper.h"
#include "osminputhelper.h"
#include "osmpoi.h"
#include "poistatistics.h"
#include "populationinput.h"
#include "textoutputhelper.h"
#include "timer.h"

namespace {
const std::size_t SPLIT_SIZE = 15;
const std::unordered_set<char32_t> DELIMITERS({' ', '-', '/'});

const bool IMPORT_SETTLEMENTS = true;
const bool IMPORT_GENERAL_POIS = false;

}

int main(int argc, char **argv) {
  argumentparser::ArgumentParser args("Osm_Input",
                                      "Program to import poi data from osm.pbf "
                                      "source files. Poi candidates are named "
                                      "human settlements as well as amenities");

  bool optional = true;
  bool binary = true;
  args.addArgument("i", "input", "path to the input .pbf file", !optional,
                   !binary);
  args.addArgument("m", "mapping",
                   "path to the json file defining the tag to class mapping.",
                   !optional, !binary);
  args.addArgument("p", "population", "file containing extra population "
                                      "information for some human settlement "
                                      "pois",
                   optional, !binary);

  if (!args.parseArguments(argc, argv) || args.isSet("h")) {
    std::printf("%s", args.programHelp().c_str());
    return EXIT_FAILURE;
  }

  std::string pbfPath = args.getValue<std::string>("i");
  std::string jsonPath = args.getValue<std::string>("m");
  std::string popPath = args.getValue<std::string>("p");

  debug_timer::Timer t;

  t.start();
  osm_input::OsmInputHelper input(pbfPath, jsonPath, IMPORT_THREAD_COUNT,
                                  IMPORT_BLOB_COUNT);
  std::vector<osm_input::OsmPoi> pois;
  if (args.isSet("p")) {
    printf("Additionally importing population data from file %s\n",
           popPath.c_str());
    std::map<std::string, int32_t> populations;
    popPath = std::string(argv[2]);
    pop_input::PopulationInput popInput(popPath);
    populations = popInput.getPopulationsMap();
    pois = input.importPoiData(IMPORT_SETTLEMENTS, IMPORT_GENERAL_POIS, populations);
    pois = input.importPoiData(IMPORT_SETTLEMENTS, IMPORT_GENERAL_POIS);
    pois = input.importPoiData(true, true);
  }
  auto &mh = input.getMappingHelper();

  t.createTimepoint();

  std::sort(pois.begin(), pois.end());

  t.stop();

  std::printf("Dataset of size: %lu\t was imported within %4.2f "
              "seconds.\n\tSorting objects took %4.2f seconds.\n",
              pois.size(), t.getTimes()[0], t.getTimes()[1]);

  {
    std::vector<osm_input::OsmPoi> undefs;
    auto *defaultLvl = mh.getLevelDefault();
    for (const auto &p : pois) {
      if (*p.getLevel() == *defaultLvl) {
        undefs.push_back(p);
      }
    }
    std::printf("Computing statistics for %lu pois with undefined level only",
                undefs.size());
    statistics::PoiStatistics statsUndefs(undefs);
    printf("%s\n", statsUndefs.tagStatisticsDetailed(2.).c_str());
  }

  //   statistics::PoiStatistics stats(pois);
  //   printf("%s\n", stats.mappingStatistics(mh).c_str());
  //   printf("%s\n", stats.tagStatisticsSimple().c_str());

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

  return EXIT_SUCCESS;
}
