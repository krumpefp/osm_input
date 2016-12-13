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

#ifndef ATLASCREATOR_H
#define ATLASCREATOR_H

#include <unordered_set>
#include <string.h>

// for the use of the freetype library compare https://www.freetype.org/freetype2/docs/tutorial/index.html
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "utf8helper.h"

namespace atlas_creator {

class AtlasCreator
{
public:
  AtlasCreator(const std::string& aFont);
  
  bool addLetter(char32_t aLetter);
  
  void createFontAtlas(const std::string& aName) const;
private:
  std::unordered_set<char32_t> mAlphabet;
  
  FT_Library  mFTLib;
  mutable FT_Face mFont;
};

}

#endif // ATLASCREATOR_H
