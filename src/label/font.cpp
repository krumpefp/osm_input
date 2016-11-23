/*
 * Class to store font information and provide methods to compute label size and
 * stuff like that
 *
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

#include "font.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "json.h"
#include "utf8helper.h"

namespace font {
Json::Value importFont(std::string aConfigPath) {
  Json::Value config;

  std::ifstream configFile(aConfigPath, std::ifstream::binary);

  configFile >> config;

  return config;
}
}

// public class functions

fonts::Font::Font(const std::string &configPath) {
  Json::Value fontConfig = font::importFont(configPath);

  Json::Value info = fontConfig["font"];
  Json::Value alphabet = fontConfig["alphabet"];

  Json::Value atlas = fontConfig["atlas"];
  Json::Value advances = fontConfig["advance"];
  Json::Value glyph = fontConfig["glyph"];
  Json::Value kerning = fontConfig["kerning"];

  std::cout << "Started font config import: " << info["name"].asString()
            << " - " << info["style"].asString() << std::endl;

  // get some general font informations
  mName = info["name"].asString();
  mStyle = info["style"].asString();
  mDefault =
      utf8_helper::UTF8Helper::toUTF8String(info["default"].asString())[0];

  // get the glyph information
  mGlyph_Asc = glyph["ascender"].asInt();
  mGlyph_Desc = glyph["descender"].asInt();
  mGlyph_Height = glyph["height"].asInt();
  mGlyph_Width = glyph["width"].asInt();

  // define the alphabet glyphs
  std::cout << "Alphabet: '" << alphabet.asString() << "'." << std::endl;
  std::size_t index = 0;
  std::u32string alphabetString =
      utf8_helper::UTF8Helper::toUTF8String(alphabet.asString());
  for (char32_t c : alphabetString) {
    mAlphabet.emplace(
        c, Glyph(c, index, advances[(int)index].asInt(), mGlyph_Width));

    std::vector<Kerning> k;
    std::size_t index_pred = 0;
    for (char32_t c_pred : alphabetString) {
      k.emplace_back(c_pred, c, kerning[(int)index][(int)index_pred].asInt(),
                     mGlyph_Width);
      ++index_pred;
    }
    mKerning.push_back(k);
    ++index;
  }

  std::cout << "Finished the font import with the import of "
            << alphabetString.size() << " glyphs" << std::endl;
}

int32_t fonts::Font::computeTextLength(const std::u32string &aStr) const {
  if (aStr.size() == 0) {
    return 0;
  }

  auto glyph = getLetter(aStr[0]);
  int32_t length = glyph->second.mAdvance;

  for (int32_t idx = 1, size = (int32_t)aStr.size(); idx < size; ++idx) {
    std::size_t idx_pred = glyph->second.mIndex;
    glyph = getLetter(aStr[idx]);

    length += glyph->second.mAdvance +
              mKerning[glyph->second.mIndex][idx_pred].mKerning;
  }

  return length;
}

const std::unordered_set<char32_t> &
fonts::Font::getUnsupportedCharacters() const {
  return mUnsupported;
}

std::u32string
fonts::Font::createFontString(const std::u32string &aString) const {
  return createFontString(aString, std::unordered_set<char32_t>());
}

std::u32string fonts::Font::createFontString(const std::u32string &aString,
                                             const char32_t aSkip) const {
  std::unordered_set<char32_t> skip;
  skip.insert(aSkip);
  return createFontString(aString, skip);
}

std::u32string
fonts::Font::createFontString(const std::u32string &aString,
                              const std::unordered_set<char32_t> &aSkip) const {
  std::u32string result;

  for (auto c : aString) {
    if (aSkip.count(c) > 0) {
      result += c;
    } else {
      result += getLetter(c)->second.mLetter;
    }
  }

  return result;
}

// private functions
std::map<char32_t, fonts::Font::Glyph>::const_iterator
fonts::Font::getLetter(const char32_t aChar) const {
  auto it = mAlphabet.find(aChar);
  if (it == mAlphabet.end()) {
    mUnsupported.insert(aChar);
    return mAlphabet.find(mDefault);
  }

  return it;
}
