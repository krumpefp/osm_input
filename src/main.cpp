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
#include "populationinput.h"
#include "textoutputhelper.h"
#include "timer.h"

namespace {
const std::size_t SPLIT_SIZE = 15;
const std::unordered_set<char> DELIMITERS({ ' ', '-', '/' });

bool
osmPoiComp(osm_input::OsmPoi* aLhs, osm_input::OsmPoi* aRhs)
{
  return *aLhs < *aRhs;
}
}

int
main(int argc, char** argv)
{
  std::printf("Hallo!\n");

  std::string jsonPath = "../mapping/tagToClass.json";
  mapping_helper::MappingHelper mh(jsonPath);
  mh.test();

  return 1;

  std::string path;

  debug_timer::Timer t;

  std::map<std::string, int32_t> populations;
  if (argc > 2) {
    path = std::string(argv[2]);
    pop_input::PopulationInput popInput(path);
    populations = popInput.getPopulationsMap();
  }

  path = std::string(argv[1]);

  t.start();
  osm_input::OsmInputHelper input(path);

  std::vector<osm_input::OsmPoi*> pois =
    input.importPoiData(true, true, populations);

  t.createTimepoint();

  std::sort(pois.begin(), pois.end(), osmPoiComp);

  t.stop();

  std::printf("Dataset of size: %lu\t was imported within %4.2f seconds.\n \
              \tSorting objects took %4.2f seconds.\n",
              pois.size(), t.getTimes()[0], t.getTimes()[1]);

  //   std::unordered_set<std::string> amenities;
  //   for (auto& poi : pois) {
  //     std::string amenity = poi->getTagValue("amenity");
  //     amenities.insert(amenity);
  //   }

  //   for (auto& am : amenities) {
  //     std::printf("%s\n", am.c_str());
  //   }

  std::vector<osm_input::OsmPoi::LabelBall> balls;
  balls.reserve(pois.size());
  for (auto it = pois.begin(), end = pois.end(); it != end;) {
    if ((*it)->getType() == osm_input::OsmPoi::Poi_Types::SETTLEMENT ||
        (*it)->hasIcon()) {
      balls.push_back((*it)->getCorrespondingBall(SPLIT_SIZE, DELIMITERS));
      ++it;
    } else {
      it = pois.erase(it);
      end = pois.end();
    }
  }

  std::string outputname = (path.find("/") == path.npos)
                             ? path.substr(0, path.size())
                             : path.substr(path.rfind('/') + 1, path.size());

  std::string outputpath = outputname + ".balls.txt";
  std::printf("Outputting data to %s\n", outputpath.c_str());
  text_output::TextOutputHelper out(outputpath);
  out.writeBallsFile(balls, ' ');

  outputpath = outputname + ".complete.txt";
  std::printf("Outputting data to %s\n", outputpath.c_str());
  text_output::TextOutputHelper outComplete(outputpath);
  outComplete.writeCompleteFile(pois, SPLIT_SIZE, DELIMITERS, ' ');

  return 1;
}
