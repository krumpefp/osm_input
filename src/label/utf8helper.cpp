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

#include "utf8helper.h"


const std::u32string utf8_helper::UTF8Helper::NEWLINE[] = {
  U"\u000D\u000A", // Carriage Return & Line Feed
  U"\u000A",       // Line Feed
  U"\u000B",       // Vertical Tab
  U"\u000C",       // Form Feed
  U"\u000D",       // Carriage Return
  U"\u0085",       // Next Line
  U"\u2028",       // Line Separator
  U"\u2029",       // Paragraph Separator
  U"^M" // sometimes indicating that mixed newline symbols were used
};


std::size_t utf8_helper::UTF8Helper::computeLengthUTF8(const std::string& aStr)
{
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
  
  std::u32string ws = convert.from_bytes(aStr);
    
  return ws.size();
}

std::string utf8_helper::UTF8Helper::computeSplit(const std::string& aLabel, const std::unordered_set<char32_t>& aDelims)
{
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
  
  std::u32string tmpLabel = convert.from_bytes(aLabel);
  std::u32string result = tmpLabel;
  
  bool newlineInfoPresent = false;
  for (std::u32string newline : NEWLINE) {
    while (tmpLabel.find(newline) != tmpLabel.npos) {
      newlineInfoPresent = true;
      tmpLabel = tmpLabel.replace(tmpLabel.find(newline), newline.size(), U"%");
    }
  }
  
  if (newlineInfoPresent) {
    std::unordered_set<char32_t> delim;
    delim.insert(U'%');
    return computeSplit(convert.to_bytes(tmpLabel), delim);
  } else {
    std::size_t centerPos = (tmpLabel.size() + 1) / 2; // ceil division value
    std::size_t pos = 0;
    
    while (pos < centerPos) {
      char32_t c = tmpLabel[centerPos + pos];
      if (aDelims.count(c) > 0) {
        std::size_t occ = tmpLabel.find(U"%");
        while (occ != tmpLabel.npos) {
          tmpLabel = tmpLabel[occ] = U' ';
          occ = tmpLabel.find(U"%");
        }
        result = tmpLabel.substr(0, centerPos + pos + 1) + U"%" +
        tmpLabel.substr(centerPos + pos + 1, tmpLabel.size());
        break;
      }
      c = tmpLabel[centerPos - pos];
      if (aDelims.count(c) > 0) {
        std::size_t occ = tmpLabel.find(U"%");
        while (occ != tmpLabel.npos) {
          tmpLabel = tmpLabel[occ] = U' ';
          occ = tmpLabel.find(U"%");
        }
        result = tmpLabel.substr(0, centerPos - pos + 1) + U"%" +
        tmpLabel.substr(centerPos - pos + 1, tmpLabel.size());
        break;
      }
      
      ++pos;
    }
  }
  if (result.find(U" %") != result.npos)
    result = result.replace(result.find(U" %"), 2, U"%");
  if (result.find(U"% ") != result.npos)
    result = result.replace(result.find(U"% "), 2, U"%");
  
  return convert.to_bytes(result);
}

std::u32string utf8_helper::UTF8Helper::toUTF8String(const std::string& aStr)
{
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
  
  return convert.from_bytes(aStr);
}
