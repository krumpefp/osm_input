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

#ifndef UTF8HELPER_H
#define UTF8HELPER_H

#include <codecvt>
#include <iostream>
#include <locale>
#include <string>
#include <unordered_set>

namespace utf8_helper {

class UTF8Helper
{
public:
  // compare https://en.wikipedia.org/wiki/Newline#Unicode
  static const std::size_t NEWLINE_COUNT = 9;
  static const std::u32string NEWLINE[];
  // compare https://en.wikipedia.org/wiki/Whitespace_character#Unicode
  static const std::size_t BLANK_COUNT = 18;
  static const std::u32string BLANK[];

public:
  static std::size_t computeLengthUTF8(const std::string& aStr);

  static bool isBlank(char32_t c);
  static bool isBlank(const std::u32string& aStr);

  static bool isNewLine(char32_t c);
  static bool isNewLine(const std::u32string& aStr);

  static std::u32string toUTF8String(const std::string& aStr);

  static std::string toByteString(const std::u32string& aStr);
};
}

#endif // UTF8HELPER_H
