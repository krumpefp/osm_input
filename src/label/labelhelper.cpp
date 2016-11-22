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

#include "label/labelhelper.h"

#include "utf8helper.h"

label_helper::LabelHelper::LabelHelper(const std::string& aFontConfigPath)
: mFont(aFontConfigPath) {}



int32_t label_helper::LabelHelper::computeLabelSize(const std::string& aLabel) const
{
  std::u32string label_u32 = utf8_helper::UTF8Helper::toUTF8String(aLabel);
  
  return mFont.computeTextLength(label_u32);
}


std::string label_helper::LabelHelper::labelify(const std::string& aLabel) const
{
  std::u32string label = mFont.toFont(utf8_helper::UTF8Helper::toUTF8String(aLabel));
  
  return utf8_helper::UTF8Helper::toByteString(label);
}
