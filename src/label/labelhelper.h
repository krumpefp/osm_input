/*
 * <one line to give the program's name and a brief idea of what it does.>
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

#ifndef LABELHELPER_H
#define LABELHELPER_H

#include <string>
#include <unordered_set>

#include "font.h"
#include "osmpoi.h"

namespace label_helper {

class LabelHelper {
public:
  struct LabelBall {
    osm_input::OsmPoi::Position mPos;
    int64_t mOsmId;

    double mBallRadius;

    std::string mLabel;
    double mLabelFactor;

    LabelBall(const osm_input::OsmPoi::Position &aCenter, int64_t aOsmId,
              double aRadius, std::string aLabel, double aFactor)
        : mPos(aCenter), mOsmId(aOsmId), mBallRadius(aRadius), mLabel(aLabel),
          mLabelFactor(aFactor){};
  };

private:
  fonts::Font mFont;
  double mSplitSize;
  int32_t mSplitSizePx;

  const std::unordered_set<char32_t> mSplitPoints;

  std::unordered_set<std::u32string> mSpaces;
  std::unordered_set<std::u32string> mNewLines;

public:
  LabelHelper(const std::string &aFontConfigPath, double aSplitSize,
              const std::unordered_set<char32_t> &aSplitPoints);

  LabelBall computeLabelBall(const osm_input::OsmPoi &aOsmPoi) const;
  int32_t computeLabelSize(const std::string &aLabel) const;
  std::pair<int32_t, int32_t>
  computeLabelSplitSize(const std::string &aLabel) const;

  std::string computeLabelSplit(const std::string &aLabel) const;

  std::string
  computeLabelSplit(const std::string &aLabel,
                    const std::unordered_set<char32_t> &aDelims) const;

  const std::unordered_set<char32_t> &getUnsupportedCharacters() const;

  std::string labelify(const std::string &aLabel) const;

  std::string labelify(const std::u32string &aLabel) const;
};
}

#endif // LABELHELPER_H
