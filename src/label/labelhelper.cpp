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

#include "labelhelper.h"

#include "utf8helper.h"

#include <limits>
#include <math.h>

namespace label_helper {
const char32_t NEWLINE = U'\u000A';
const char32_t SPACE = U'\u0020';
const std::u32string SZ_NEWLINE = std::u32string() + label_helper::NEWLINE;
const std::u32string SZ_SPACE = std::u32string() + label_helper::SPACE;

void
replaceAllOf(const std::unordered_set<std::u32string>& aReplace,
             const std::u32string& aReplaceBy,
             std::u32string& aLabel)
{
  for (auto& r : aReplace) {
    if (aLabel.find(r) != aLabel.npos) {
      aLabel.replace(aLabel.find(r), r.size(), aReplaceBy);
    }
  }
}

void
trim(const char32_t& aChar, std::u32string& aLabel)
{
  // trim front
  std::size_t idx = 0;
  while (aLabel[idx] == aChar) {
    ++idx;
  }
  aLabel = aLabel.substr(idx);

  idx = aLabel.size() - 1;
  while (aLabel[idx] == aChar) {
    --idx;
  }
  aLabel = aLabel.substr(0, idx + 1);
}

std::pair<std::u32string, std::u32string>
toLabelSplit(const std::u32string& aLabel,
             std::size_t aSplitPos,
             const std::unordered_set<std::u32string>& aSpaces,
             const std::unordered_set<std::u32string>& aNewLines)
{
  std::u32string label1 = aLabel.substr(0, aSplitPos + 1);
  std::u32string label2 = aLabel.substr(aSplitPos + 1);

  // replace all newline and blank characters by spaces
  replaceAllOf(aNewLines, label_helper::SZ_SPACE, label1);
  replaceAllOf(aNewLines, label_helper::SZ_SPACE, label2);
  replaceAllOf(aSpaces, label_helper::SZ_SPACE, label1);
  replaceAllOf(aSpaces, label_helper::SZ_SPACE, label2);

  // trim whitespaces at the end of the labels
  trim(label_helper::SPACE, label1);
  trim(label_helper::SPACE, label2);

  return std::make_pair(label1, label2);
}
}

label_helper::LabelHelper::LabelHelper(
  const std::string& aFontTTFPath,
  int32_t aSplitSize,
  const std::unordered_set<char32_t>& aSplitPoints)
  : mFont(aFontTTFPath)
  , mSplitSizePx(aSplitSize * mFont.getMeanLetterWidth())
  , mSplitPoints(aSplitPoints)
{
  for (std::size_t i = 0; i < utf8_helper::UTF8Helper::BLANK_COUNT; ++i) {
    mSpaces.insert(utf8_helper::UTF8Helper::BLANK[i]);
  }

  for (std::size_t i = 0; i < utf8_helper::UTF8Helper::NEWLINE_COUNT; ++i) {
    mNewLines.insert(utf8_helper::UTF8Helper::NEWLINE[i]);
  }
}

label_helper::LabelHelper::LabelBall
label_helper::LabelHelper::computeLabelBall(
  const osm_input::OsmPoi& aOsmPoi) const
{
  std::string label;
  double ballRadius = 1;

  if (aOsmPoi.getLevel()->mIconName != "") {
    label = "icon:" + aOsmPoi.getLevel()->mIconName;
    ballRadius = mFont.getMeanLetterWidth();
  } else {
    int32_t l = computeLabelSize(aOsmPoi.getName());
    if (l > mSplitSizePx) {
      label = computeLabelSplit(aOsmPoi.getName());
    } else {
      label = aOsmPoi.getName();
    }

    std::pair<int32_t, int32_t> size = computeLabelSplitSize(label);
    ballRadius = std::max(size.first, size.second) / 2;
  }

  ballRadius *= aOsmPoi.getLevel()->mLevelFactor;

  return LabelBall(aOsmPoi.getPosition(),
                   aOsmPoi.getOsmId(),
                   ballRadius,
                   label,
                   aOsmPoi.getLevel()->mLevelFactor);
}

int32_t
label_helper::LabelHelper::computeLabelSize(const std::string& aLabel) const
{
  std::u32string label_u32 = utf8_helper::UTF8Helper::toUTF8String(aLabel);

  return mFont.computeTextLength(label_u32);
}

std::pair<int32_t, int32_t>
label_helper::LabelHelper::computeLabelSplitSize(
  const std::string& aLabel) const
{
  std::pair<int32_t, int32_t> result;
  std::size_t splitPos = aLabel.find("\n");
  if (splitPos == aLabel.npos) {
    result = std::make_pair(computeLabelSize(aLabel), -1);
  } else {
    result = std::make_pair(computeLabelSize(aLabel.substr(0, splitPos)),
                            computeLabelSize(aLabel.substr(splitPos + 1)));
  }

  return result;
}

std::string
label_helper::LabelHelper::computeLabelSplit(const std::string& aLabel) const
{
  return computeLabelSplit(aLabel, mSplitPoints);
}

std::string
label_helper::LabelHelper::computeLabelSplit(
  const std::string& aLabel,
  const std::unordered_set<char32_t>& aDelims) const
{
  std::u32string label_u32 = utf8_helper::UTF8Helper::toUTF8String(aLabel);

  // remove trailing newline information
  std::u32string s = label_u32.substr(label_u32.size() - 2);
  if (mNewLines.count(label_u32.substr(label_u32.size() - 2)) > 0) {
    label_u32 = label_u32.substr(0, label_u32.size() - 1);
  }
  s = label_u32.substr(label_u32.size() - 1);
  if (mNewLines.count(label_u32.substr(label_u32.size() - 1)) > 0) {
    label_u32 = label_u32.substr(0, label_u32.size() - 1);
  }

  if (label_u32.size() <= 1) {
    return utf8_helper::UTF8Helper::toByteString(label_u32);
  }

  std::u32string result = std::u32string();

  bool newlineInfoPresent = false;
  for (std::size_t idx = 0; idx < utf8_helper::UTF8Helper::NEWLINE_COUNT;
       ++idx) {
    std::u32string newline = utf8_helper::UTF8Helper::NEWLINE[idx];

    if (aDelims.count(newline[0]) > 0) {
      // don't replace newline if it is the SZ_NEWLINE
      continue;
    }

    if (label_u32.find(newline) != label_u32.npos &&
        newline == label_helper::SZ_NEWLINE) {
      newlineInfoPresent = true;
      continue;
    }

    while (label_u32.find(newline) != label_u32.npos) {
      newlineInfoPresent = true;
      label_u32 = label_u32.replace(
        label_u32.find(newline), newline.size(), label_helper::SZ_NEWLINE);
    }
  }

  if (newlineInfoPresent) {
    std::unordered_set<char32_t> delim;
    delim.insert(label_helper::NEWLINE);
    return computeLabelSplit(utf8_helper::UTF8Helper::toByteString(label_u32),
                             delim);
  }

  // we realy need to do work here ...

  // compute the median character
  int32_t length = mFont.computeTextLength(label_u32);
  std::size_t index = 0;
  while (mFont.computeTextLength(label_u32.substr(0, index)) < length / 2) {
    ++index;
  }
  // now index points to the median element
  if (aDelims.count(label_u32[index]) > 0) {
    auto split = toLabelSplit(label_u32, index, mSpaces, mNewLines);
    result = split.first + label_helper::NEWLINE + split.second;
  } else {
    // compute the best split if split point is in the first half
    std::pair<std::u32string, std::u32string> splitFirst;
    int32_t sizeSplitFirst = std::numeric_limits<int32_t>::max();
    for (size_t i = index - 1; i > 0; --i) {
      if (aDelims.count(label_u32[i]) > 0) {
        splitFirst = toLabelSplit(label_u32, i, mSpaces, mNewLines);
        sizeSplitFirst = std::max(mFont.computeTextLength(splitFirst.first),
                                  mFont.computeTextLength(splitFirst.second));
        break;
      }
    }

    std::pair<std::u32string, std::u32string> splitSecond;
    int32_t sizeSplitSecond = std::numeric_limits<int32_t>::max();
    for (size_t i = index + 1; i < label_u32.size(); ++i) {
      if (aDelims.count(label_u32[i]) > 0) {
        splitSecond = toLabelSplit(label_u32, i, mSpaces, mNewLines);
        sizeSplitSecond = std::max(mFont.computeTextLength(splitSecond.first),
                                   mFont.computeTextLength(splitSecond.second));
        break;
      }
    }

    if (sizeSplitFirst != std::numeric_limits<int32_t>::max() ||
        sizeSplitSecond != std::numeric_limits<int32_t>::max()) {
      if (sizeSplitFirst < sizeSplitSecond) {
        result = splitFirst.first + label_helper::NEWLINE + splitFirst.second;
      } else {
        result = splitSecond.first + label_helper::NEWLINE + splitSecond.second;
      }
    } else {
      // can't find any viable label split
      result = label_u32;
    }
  }

  return utf8_helper::UTF8Helper::toByteString(result);
}

void
label_helper::LabelHelper::outputFontAtlas(std::string aAtlasName)
{
  mFont.createFontAtlas(aAtlasName);
}
