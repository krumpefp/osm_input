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

#ifndef FONT_H
#define FONT_H

#include <codecvt>
#include <locale>

#include <map>
#include <string>
#include <vector>

namespace label {

class Font {
private:
  struct Glyph {
    char32_t mLetter;
    std::size_t mIndex;

    int32_t mAdvance;
    double mAdvanceRatio;

    Glyph(char32_t aLetter, std::size_t aIdx, int32_t aAdv, int32_t aGlyphWidth)
        : mLetter(aLetter), mIndex(aIdx), mAdvance(aAdv),
          mAdvanceRatio((double)aAdv / (double)aGlyphWidth){};
  };

  struct Kerning {
    char32_t mPredecessor;
    char32_t mSuccessor;

    int32_t mKerning;
    double mKerningRatio;

    Kerning(char32_t aPred, char32_t aSucc, int32_t aKern, int32_t aGlyphWidth)
        : mPredecessor(aPred), mSuccessor(aSucc), mKerning(aKern),
          mKerningRatio((double)aKern / (double)aGlyphWidth){};
  };

  // general font information
  std::string mName;
  std::string mStyle;

  int32_t mGlyph_Asc;
  int32_t mGlyph_Desc;
  int32_t mGlyph_Width;
  int32_t mGlyph_Height;

  // Glyph specific information
  std::map<char32_t, Glyph> mAlphabet;

  std::vector<std::vector<Kerning>> mKerning;

public:
  Font(std::string configPath);
};
}

#endif // FONT_H
