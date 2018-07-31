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

#include "argumentparser/argumentparser.h"

#include "confighelper.h"
#include "labelhelper.h"
#include "mappinghelper.h"
#include "osminputhelper.h"
#include "osmpoi.h"
#include "poistatistics.h"
#include "populationinput.h"
#include "textoutputhelper.h"
#include "timer.h"
#include "utf8helper.h"

namespace {
using ARG_TYPES = argumentparser::ArgumentParser::ARGUMENT_TYPES;
}

int
main(int argc, char** argv)
{
  argumentparser::ArgumentParser args("Osm_Input",
                                      "Program to import poi data from osm.pbf "
                                      "source files. Poi candidates are named "
                                      "human settlements as well as amenities");

  // required arguments
  args.addArgumentRequired("-C",
                           "--config",
                           "Defines the config file which guides the import.",
                           ARG_TYPES::STRING);
  args.addArgumentRequired(
    "-i", "--input", "path to the input .pbf file", ARG_TYPES::STRING);
  args.addArgumentRequired("-C",
                           "--config",
                           "Defines the config file which guides the import.",
                           ARG_TYPES::STRING);
  // optional arguments
  args.addArgument("-bc",
                   "--blobcount",
                   "define the number of blobs used per "
                   "thread during the pbf import. "
                   "Default 2",
                   ARG_TYPES::INT);
  args.addArgument("-eh",
                   "--exporthierarchy",
                   "if set the hierarchy levels will be exported instead of a "
                   "total oder.",
                   ARG_TYPES::BINARY);
  args.addArgument("-fa",
                   "--fontatlas",
                   "if set, the font information will be "
                   "outputted to a font file",
                   ARG_TYPES::BINARY);
  args.addArgument(
    "-tc",
    "--threadcount",
    "define the number of threads used during the pbf import. Default 4",
    ARG_TYPES::INT);

  try {
    if (!args.parseArguments(std::size_t(argc), argv) && !args.isSet("-h")) {
      std::cerr << "Some required arguments were not given." << std::endl
                << args.programHelp() << std::endl;
      return 1;
    }
  } catch (const std::exception& e) {
    std::cerr << "Parsing command line parameter failed with explanation:\n"
              << e.what() << std::endl;
    std::cerr << "Terminating ..." << std::endl;

    return 1;
  }

  if (args.getValue<bool>("-h")) {
    std::cout << args.programHelp() << std::endl;
    return 0;
  }

  // required arguments
  std::string pbfPath = args.getValue<std::string>("-i");
  config_helper::ConfigHelper config(args.getValue<std::string>("-C"));

  // optional arguments
  int threadCount = (args.isSet("-tc")) ? args.getValue<int>("-tc") : 4;
  int blobCount = (args.isSet("-bc")) ? args.getValue<int>("-bc") : 2;

  label_helper::LabelHelper labelHelper(config.get_ttf_path(),
                                        config.get_split_bound(),
                                        config.get_split_delimiters());

  const mapping_helper::MappingHelper& mappingHelper =
    config.get_mapping_helper();

  debug_timer::Timer t;
  t.start();
  osm_input::OsmInputHelper input(pbfPath, config, threadCount, blobCount);
  std::vector<osm_input::OsmPoi> pois;
  pois = input.importPoiData();

  t.createTimepoint();

  std::sort(pois.begin(), pois.end());

  t.stop();

  std::printf("Dataset of size: %lu\t was imported within %4.2f "
              "seconds.\n\tSorting objects took %4.2f seconds.\n",
              pois.size(),
              t.getTimes()[0],
              t.getTimes()[1]);

  if (args.isSet("-fa")) {
    std::cout << "Writing font atlas to files ... " << std::endl;

    labelHelper.outputFontAtlas(config.get_font_name());

    std::cout << "... successfull!" << std::endl;
  }

  std::cout << "Creating label discs ..." << std::endl;

  std::vector<label_helper::LabelHelper::LabelBall> balls;
  balls.reserve(pois.size());
  std::size_t count = 0;
  for (const auto& poi : pois) {
    balls.push_back(labelHelper.computeLabelBall(poi));
    ++count;
  }

  std::cout << "... successfull!" << std::endl;

  std::cout << "Exporting data ..." << std::endl;

  std::string outputpath = config.get_labeling_name() + ".complete.txt";
  std::replace(outputpath.begin(), outputpath.end(), ' ', '_');
  std::printf("Outputting data to %s\n", outputpath.c_str());
  text_output::TextOutputHelper outComplete(outputpath);
  outComplete.writeCompleteFile(balls, ' ', args.isSet("-eh"));

  std::cout << "... successfull!" << std::endl;

  return EXIT_SUCCESS;
}
