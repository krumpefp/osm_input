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

// TODO: Update NEWLINE_COUNT in the header file when changing the arrays
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

// TODO: Update BLANK_COUNT in the header file when changing the arrays
const std::u32string utf8_helper::UTF8Helper::BLANK[] = {
    U"\u0009", // Character Tabulation
    U"\u0020", // Space
    U"\u00A0", // No-Break Space
    U"\u1680", // Ogham Space Mark
    U"\u2000", // EN Quad
    U"\u2001", // EM Quad
    U"\u2002", // EN Space
    U"\u2003", // EM Space
    U"\u2004", // three-per-em space
    U"\u2005", // four-per-em space
    U"\u2006", // six-per-em space
    U"\u2007", // figure space
    U"\u2008", // punctuation space
    U"\u2009", // thin space
    U"\u200A", // hair space
    U"\u202F", // narrow no-break space
    U"\u205F", // medium mathematical space
    U"\u3000", // ideographic space
};

std::size_t
utf8_helper::UTF8Helper::computeLengthUTF8(const std::string &aStr) {
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;

  std::u32string ws = convert.from_bytes(aStr);

  return ws.size();
}

bool utf8_helper::UTF8Helper::isBlank(char32_t c) {
  std::u32string sz = std::u32string() + c;

  return isBlank(sz);
}

bool utf8_helper::UTF8Helper::isBlank(const std::u32string &aStr) {
  for (size_t i = 0; i < BLANK_COUNT; ++i) {
    if (BLANK[i] == aStr) {
      return true;
    }
  }

  return false;
}

bool utf8_helper::UTF8Helper::isNewLine(char32_t c) {
  std::u32string sz = std::u32string() + c;

  return isNewLine(sz);
}

bool utf8_helper::UTF8Helper::isNewLine(const std::u32string &aStr) {
  for (size_t i = 0; i < NEWLINE_COUNT; ++i) {
    if (NEWLINE[i] == aStr) {
      return true;
    }
  }

  return false;
}

std::u32string utf8_helper::UTF8Helper::toUTF8String(const std::string &aStr) {
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;

  return convert.from_bytes(aStr);
}

std::string utf8_helper::UTF8Helper::toByteString(const std::u32string &aStr) {
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;

  return convert.to_bytes(aStr);
}
