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

#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// for the use of the freetype library compare
// https://www.freetype.org/freetype2/docs/tutorial/index.html
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace fonts {

class Font {
private:
  struct Kerning {
    char32_t mPredecessor;
    char32_t mSuccessor;

    int32_t mKerning;

    Kerning(char32_t aPred, char32_t aSucc, int32_t aKern)
        : mPredecessor(aPred), mSuccessor(aSucc), mKerning(aKern){};
  };

  struct Glyph {
    char32_t mLetter;
    int32_t mAdvance;
    std::unordered_map<char32_t, Kerning> mKerning;

    Glyph(char32_t aLetter, int32_t aAdv) : mLetter(aLetter), mAdvance(aAdv){};

    void updateKerning(std::u32string aAlphabet, Font *aFontFace);

    int32_t getKerning(char32_t c);
  };

  // general font information
  std::string mName;
  std::string mStyle;

  FT_Library mFTLib;
  mutable FT_Face mFace;

  std::u32string mCurrentAlphabet;

  // Glyph specific information
  std::map<char32_t, Glyph> mAlphabet;
  std::vector<std::vector<Kerning>> mKerning;

public:
  Font(const std::string &configPath);

  int32_t computeTextLength(const std::u32string &aStr);

  void createFontAtlas(const std::string &aName) const;

  FT_Face *getFontFace() { return &mFace; };
  int32_t getMeanLetterWidth() const;
};
}

#endif // FONT_H
