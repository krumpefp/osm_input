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

#include <cairo/cairo.h>
#include <json/json.h>

#include <algorithm>
#include <assert.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <math.h>
#include <stdexcept>

#include "font.h"
#include "utf8helper.h"

namespace fonts {
// Font size in pixels for the font atlas
int32_t FONT_SIZE = 100;
int32_t MEAN_LETTER_WITH = FONT_SIZE * 2 / 3;

const double pow6 = 64;     // 2^6
const double pow16 = 65536; // 2^16

double fromFP26_6(int32_t aFixPoint) { return (double)aFixPoint / pow6; }

double fromFP16_16(int32_t aFixPoint) { return (double)aFixPoint / pow16; }

void initFontFace(FT_Library &aLib, FT_Face &aFace, std::string aFontName) {
  auto error = FT_Init_FreeType(&aLib);
  if (error) {
    throw std::runtime_error("Unable to load the freetype library!");
  }

  error = FT_New_Face(aLib, aFontName.c_str(), 0, &aFace);
  if (error == FT_Err_Unknown_File_Format) {
    throw std::runtime_error("Font file format is not supported!");
  } else if (error) {
    throw std::runtime_error("Unable to open or read the font file: " +
                             aFontName + "!");
  }

  error = FT_Set_Pixel_Sizes(aFace, FONT_SIZE, FONT_SIZE);
  if (error) {
    throw std::runtime_error("Font does not support assignment of pixel size!");
  }
}

struct AlignedBMP_A8 {
  std::vector<unsigned char> mData;
  int32_t mWidth;
  int32_t mHeight;

  AlignedBMP_A8(const unsigned char *aData, int32_t aWidth, int32_t aHeight) {
    mWidth = cairo_format_stride_for_width(CAIRO_FORMAT_A8, aWidth);
    mHeight = aHeight;
    mData.reserve(mWidth * mHeight);
    mData.insert(mData.end(), mWidth * mHeight, 0);

    for (size_t i = 0; i < aHeight; ++i) {
      std::memcpy(mData.data() + i * mWidth, aData + i * aWidth, aWidth);
    }
  }

  unsigned char *getBytes() { return mData.data(); }
};

// glyph struct
void Font::Glyph::updateKerning(std::u32string aAlphabet, fonts::Font *aFont) {
  FT_Face face = *aFont->getFontFace();
  int32_t idx = FT_Get_Char_Index(face, mLetter);

  for (char32_t c : aAlphabet) {
    if (mKerning.find(c) != mKerning.end()) {
      // kerning was already inserted ...
      continue;
    }

    int32_t c_idx = FT_Get_Char_Index(face, c);
    FT_Vector kerning;
    auto error = FT_Get_Kerning(face, c_idx, idx, FT_KERNING_DEFAULT, &kerning);
    if (error) {
      throw std::runtime_error(
          "Unable to get glyph kerning: " + std::to_string(c) + " - " +
          std::to_string(mLetter) + "! Error was: " + std::to_string(error));
    }
    mKerning.emplace(
        c, Kerning(c, mLetter, (int32_t)std::ceil(fromFP26_6(kerning.x))));
  }
}

int32_t Font::Glyph::getKerning(char32_t c) {
  auto it = mKerning.find(c);
  assert(it != mKerning.end());

  return it->second.mKerning;
}
}

// public class functions
fonts::Font::Font(const std::string &fontPath) {
  initFontFace(mFTLib, mFace, fontPath);
  mName = mFace->family_name;
  mStyle = mFace->style_name;

  std::cout << "Started font config import: " << mName << " - " << mName
            << std::endl;
}

int32_t fonts::Font::computeTextLength(const std::u32string &aStr) {
  if (aStr.size() == 0) {
    return 0;
  }

  bool updateRequired = false;
  for (char32_t c : aStr) {
    if (mAlphabet.find(c) == mAlphabet.end()) {
      updateRequired = true;
      break;
    }
  }

  if (updateRequired) {
    for (char32_t c : aStr) {
      if (mAlphabet.find(c) != mAlphabet.end()) {
        continue;
      }

      mCurrentAlphabet += c;

      auto error = FT_Load_Char(mFace, c, FT_LOAD_RENDER);
      if (error) {
        throw std::runtime_error("Unable to load glyph: " + std::to_string(c) +
                                 "! Error was: " + std::to_string(error));
      }
      int32_t adv =
          (int32_t)std::ceil(fromFP26_6(mFace->glyph->metrics.horiAdvance));

      mAlphabet.emplace(c, Glyph(c, adv));
    }

    for (auto &c : mAlphabet) {
      c.second.updateKerning(mCurrentAlphabet, this);
    }
  }

  auto glyph = mAlphabet.at(aStr[0]);
  int32_t length = glyph.mAdvance;

  for (int32_t idx = 1, size = (int32_t)aStr.size(); idx < size; ++idx) {
    glyph = mAlphabet.at(aStr[idx]);
    length += glyph.mAdvance + glyph.getKerning(aStr[idx - 1]);
  }

  return length;
}

void fonts::Font::createFontAtlas(const std::string &aName) const {
  std::u32string alphabet = mCurrentAlphabet;
  std::sort(alphabet.begin(), alphabet.end());
  // get overall glyph information
  int32_t maxAdv = 0;
  int32_t maxHeight;
  double meanAdv = 0;
  int32_t maxBearingY = 0;
  int32_t maxNegBearingY = 0;
  for (auto &c : alphabet) {
    auto error = FT_Load_Char(mFace, c, FT_LOAD_RENDER);
    if (error) {
      throw std::runtime_error("Unable to load glyph: " + std::to_string(c) +
                               "! Error was: " + std::to_string(error));
    }
    maxAdv = std::max(maxAdv, (int32_t)std::ceil(fromFP26_6(
                                  mFace->glyph->metrics.horiAdvance)));
    meanAdv += (double)fromFP26_6(mFace->glyph->metrics.horiAdvance);
    maxBearingY = std::max(maxBearingY, (int32_t)std::ceil(fromFP26_6(
      mFace->glyph->metrics.horiBearingY)));
    maxNegBearingY = std::max(maxNegBearingY, (int32_t)std::ceil(fromFP26_6(
      mFace->glyph->metrics.height - mFace->glyph->metrics.horiBearingY)));
  }
  meanAdv /= alphabet.size();
  maxHeight = maxBearingY + maxNegBearingY;

  int32_t columns = 14;
  int32_t rows = (int32_t)std::ceil((double)alphabet.size() / (double)columns);
  int32_t width = columns * maxAdv;
  int32_t height = rows * maxHeight;

  // create the atlas
  {
    cairo_surface_t *surface;
    cairo_t *ctx;

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    ctx = cairo_create(surface);
    cairo_set_source_rgb(ctx, 1, 1, 1);
    cairo_rectangle(ctx, 0, 0, width, height);
    cairo_fill(ctx);

    int32_t posX = 0;
    int32_t posY = 0;
    FT_Glyph glyph;
    for (auto &c : alphabet) {
      auto error = FT_Load_Char(mFace, c, FT_LOAD_RENDER);
      if (error) {
        throw std::runtime_error("Unable to load glyph: " + std::to_string(c) +
                                 "! Error was: " + std::to_string(error));
      }
      error = FT_Get_Glyph(mFace->glyph, &glyph);
      if (error) {
        throw std::runtime_error("Unable to get glyph " + std::to_string(c) +
                                 "!");
      }

      error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, 1);
      if (error) {
        throw std::runtime_error("Unable to render glyph " + std::to_string(c) +
                                 "!");
      }

      FT_BitmapGlyph g = (FT_BitmapGlyph)glyph;
      AlignedBMP_A8 glyphBmp(g->bitmap.buffer, g->bitmap.width, g->bitmap.rows);
      cairo_surface_t *bmp = cairo_image_surface_create_for_data(
          glyphBmp.getBytes(), CAIRO_FORMAT_A8, glyphBmp.mWidth,
          glyphBmp.mHeight, glyphBmp.mWidth);
      int32_t basePointY = posY + maxBearingY - (int32_t)std::ceil(fromFP26_6(mFace->glyph->metrics.horiBearingY));
      cairo_set_source_surface(ctx, bmp, posX, basePointY);
      cairo_paint(ctx);

      cairo_surface_destroy(bmp);
      FT_Done_Glyph(glyph);

      posX += maxAdv;
      if (posX > columns * maxAdv) {
        posX = 0;
        posY += maxHeight;
      }
    }

    std::string path = aName + ".png";

    cairo_surface_flush(surface);
    cairo_surface_write_to_png(surface, path.c_str());
    cairo_surface_destroy(surface);
    cairo_destroy(ctx);
  }

  // create the info file
  std::string alphabetUTF8 = utf8_helper::UTF8Helper::toByteString(alphabet);
  //   std::cout << "Alphabet: " << alphabetUTF8.c_str() << std::endl;

  // font specific information
  Json::Value atlas;
  atlas["size"]["width"] = width;
  atlas["size"]["height"] = height;
  atlas["dimension"]["columns"] = columns;
  atlas["dimension"]["rows"] = rows;
  atlas["name"] = aName;

  Json::Value font;
  font["style"] = mFace->style_name;
  font["name"] = mFace->family_name;

  Json::Value glyph;
  glyph["ascender"] = (int32_t)std::ceil(fromFP26_6(mFace->ascender));
  glyph["descender"] = (int32_t)std::ceil(fromFP26_6(mFace->descender));
  glyph["top_height"] = maxBearingY;
  glyph["bottom_height"] = maxNegBearingY;
  glyph["height"] = (int32_t)std::ceil(maxHeight);
  glyph["width"] = maxAdv;
  glyph["mean_width"] = (int32_t)std::ceil(meanAdv);

  // glyph specific info
  Json::Value advances = Json::arrayValue;
  Json::Value kerning = Json::arrayValue;
  for (char32_t c : alphabet) {
    auto error = FT_Load_Char(mFace, c, FT_LOAD_RENDER);
    if (error) {
      throw std::runtime_error("Unable to load glyph: " + std::to_string(c) +
                               "! Error was: " + std::to_string(error));
    }

    int32_t adv =
        (int32_t)std::ceil(fromFP26_6(mFace->glyph->metrics.horiAdvance));
    advances.append(adv);

    Json::Value kerning_c = Json::arrayValue;
    int32_t c_idx = FT_Get_Char_Index(mFace, c);
    FT_Vector kerning_c2c;
    for (char32_t c2 : alphabet) {
      int32_t c2_idx = FT_Get_Char_Index(mFace, c2);
      error = FT_Get_Kerning(mFace, c2_idx, c_idx, FT_KERNING_DEFAULT,
                             &kerning_c2c);
      int32_t kerning = (int32_t)std::ceil(fromFP26_6(kerning_c2c.x));
      kerning_c.append(kerning);
    }
    kerning.append(kerning_c);
  }

  // general information
  Json::Value root;
  root["advance"] = advances;
  root["alphabet"] = alphabetUTF8;
  root["atlas"] = atlas;
  root["font"] = font;
  root["glyph"] = glyph;
  root["kerning"] = kerning;

  Json::StyledWriter jsonWriter;
  std::ofstream output(aName + ".info");
  output << jsonWriter.write(root);
  output.close();
}

int32_t fonts::Font::getMeanLetterWidth() const { return MEAN_LETTER_WITH; }
