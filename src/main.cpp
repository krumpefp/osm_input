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

#include "labelhelper.h"
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

typedef argumentparser::ArgumentParser::ARGUMENT_TYPES ARG_TYPES;
}

int main(int argc, char **argv) {
  
  label_helper::LabelHelper labelHelper("font.info");
  std::string label = argv[1];
  int32_t size = labelHelper.computeLabelSize(label);
  
  std::cout << "Label " << label << " has size " << size << std::endl
            << "Label will be converted to: '" << labelHelper.labelify(label)
            << "'" << std::endl;
  
  return 0;
  
  argumentparser::ArgumentParser args("Osm_Input",
                                      "Program to import poi data from osm.pbf "
                                      "source files. Poi candidates are named "
                                      "human settlements as well as amenities");

  args.addArgumentRequired("-i", "--input", "path to the input .pbf file",
                           ARG_TYPES::STRING);
  args.addArgumentRequired(
      "-m", "--mapping",
      "path to the json file defining the tag to class mapping.",
      ARG_TYPES::STRING);
  args.addArgument("-p", "--population", "file containing extra population "
                                         "information for some human "
                                         "settlement pois",
                   ARG_TYPES::STRING);

  args.addArgument("-c", "--cities",
                   "if set, city (human settlement) labels are imported",
                   ARG_TYPES::BINARY);
  args.addArgument("-g", "--general", "if set, general poi labels are imported",
                   ARG_TYPES::BINARY);

  args.addArgument(
      "-tc", "--threadcount",
      "define the number of threads used during the pbf import. Default 4",
      ARG_TYPES::INT);
  args.addArgument("-bc", "--blobcount", "define the number of blobs used per "
                                         "thread during the pbf import. "
                                         "Default 2",
                   ARG_TYPES::INT);

  try {
    if (!args.parseArguments(std::size_t(argc), argv) && !args.isSet("-h")) {
      std::cerr << "Some required arguments were not given. Terminating!"
                << std::endl;
      return 1;
    }
  } catch (const std::exception &e) {
    std::cerr << "Parsing command line parameter failed with explanation:\n"
              << e.what() << std::endl;
    std::cerr << "Terminating ..." << std::endl;

    return 1;
  }

  if (args.getValue<bool>("-h")) {
    std::cout << args.programHelp() << std::endl;
    return 0;
  }

  std::string pbfPath = args.getValue<std::string>("-i");
  std::string jsonPath = args.getValue<std::string>("-m");
  std::string popPath = args.getValue<std::string>("-p");

  debug_timer::Timer t;

  int threadCount = (args.isSet("-tc")) ? args.getValue<int>("-tc") : 4;
  int blobCount = (args.isSet("-bc")) ? args.getValue<int>("-bc") : 2;

  t.start();
  osm_input::OsmInputHelper input(pbfPath, jsonPath, threadCount, blobCount);
  std::vector<osm_input::OsmPoi> pois;
  if (args.isSet("p")) {
    printf("Additionally importing population data from file %s\n",
           popPath.c_str());
    std::map<std::string, int32_t> populations;
    popPath = std::string(argv[2]);
    pop_input::PopulationInput popInput(popPath);
    populations = popInput.getPopulationsMap();
    pois = input.importPoiData(args.getValue<bool>("-c"),
                               args.getValue<bool>("-g"), populations);
  } else {
    pois = input.importPoiData(args.getValue<bool>("-c"),
                               args.getValue<bool>("-g"));
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
