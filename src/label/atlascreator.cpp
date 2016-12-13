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

#include <cairo/cairo.h>

#include <assert.h>
#include <cstring>
#include <math.h>
#include <vector>


#include "atlascreator.h"

namespace atlas_creator {
  // Font size in pixels for the font atlas
  int32_t FONT_SIZE = 100;
  
  const double pow6 = 64;       // 2^6
  const double pow16 = 65536;   // 2^16
  
  double fromFP26_6(int32_t aFixPoint) {
    return (double) aFixPoint / pow6;
  }
  
  double fromFP16_16(int32_t aFixPoint) {
    return (double) aFixPoint / pow16;
  }
  
  void initFontFace(FT_Library& aLib, FT_Face& aFace, std::string aFontName) {
    auto error = FT_Init_FreeType (&aLib);
    if ( error ) {
      throw std::runtime_error("Unable to load the freetype library!");
    }
    
    error = FT_New_Face( aLib, aFontName.c_str(), 0, &aFace );
    if ( error == FT_Err_Unknown_File_Format )
    {
      throw std::runtime_error("Font file format is not supported!");
    }
    else if ( error )
    {
      throw std::runtime_error("Unable to open or read the font file: " + aFontName + "!");
    }
    
    error = FT_Set_Pixel_Sizes(aFace, FONT_SIZE, FONT_SIZE);
    if ( error ) {
      throw std::runtime_error("Font does not support assignment of pixel size!");
    }
  }
  
  void loadGlyph(FT_Face& aFace, char32_t aChar) {
    int32_t idx = FT_Get_Char_Index(aFace, aChar);
    
    auto error = FT_Load_Glyph(aFace, idx, FT_LOAD_RENDER);
//    auto error = FT_Load_Glyph(aFace, idx, FT_LOAD_NO_SCALE);  
    
    if ( error ) {
      throw std::runtime_error("Unable to load glyph with index " + std::to_string(idx) + "!");
    }
  }
  
  struct AlignedBMP_A8{
    std::vector<unsigned char> mData;
    int32_t mWidth;
    int32_t mHeight;
    
    AlignedBMP_A8(const unsigned char* aData, int32_t aWidth, int32_t aHeight) {
      mWidth = cairo_format_stride_for_width(CAIRO_FORMAT_A8, aWidth);
      mHeight = aHeight;
      mData.reserve(mWidth * mHeight);
      mData.insert(mData.end(), mWidth * mHeight, 0);
      
      for (size_t i = 0; i < aHeight; ++i) {
        std::memcpy(mData.data() + i * mWidth, aData + i * aWidth, aWidth);
      }
    }
    
    unsigned char* getBytes() {
      return mData.data();
    }
  };
}

atlas_creator::AtlasCreator::AtlasCreator ( const std::string& aFont )
{
  initFontFace(mFTLib, mFont, aFont );
}

bool atlas_creator::AtlasCreator::addLetter(char32_t aLetter)
{
  auto res = mAlphabet.insert(aLetter);
  
  // res.second indicates whether the insertion actually took place or not
  return res.second;
}

void atlas_creator::AtlasCreator::createFontAtlas(const std::string& aName) const
{
  // get overall glyph information
  int32_t maxAdv = 0;
  for (auto& c : mAlphabet) {
    loadGlyph(mFont, c);
    maxAdv = std::max(maxAdv, (int32_t)std::ceil(fromFP26_6(mFont->glyph->metrics.horiAdvance)));
  }
  
  int32_t width = (2 + 14) * maxAdv;
  int32_t height = (int32_t) std::ceil((double)mAlphabet.size() / 14.) * FONT_SIZE;
  
  cairo_surface_t* surface;
  cairo_t* ctx;
  
  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  ctx = cairo_create (surface);
  cairo_set_source_rgb (ctx, 1, 1, 1);
  cairo_rectangle (ctx, 0, 0, width, height);
  cairo_fill (ctx);
  
  int32_t posX = maxAdv;
  int32_t posY = 0;
  FT_Glyph glyph;
  for (auto& c : mAlphabet) {
    loadGlyph(mFont, c);
      auto error = FT_Get_Glyph(mFont->glyph, &glyph);
      if (error) {
        throw std::runtime_error("Unable to get glyph " + std::to_string(c) + "!");
      }
      
      error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, 1);
      if (error) {
        throw std::runtime_error("Unable to render glyph " + std::to_string(c) + "!");
      }
      
      FT_BitmapGlyph g = (FT_BitmapGlyph)glyph;
      AlignedBMP_A8 glyphBmp(g->bitmap.buffer, g->bitmap.width, g->bitmap.rows);
      cairo_surface_t* bmp = cairo_image_surface_create_for_data(glyphBmp.getBytes(),CAIRO_FORMAT_A8, glyphBmp.mWidth, glyphBmp.mHeight, glyphBmp.mWidth);
      cairo_set_source_surface(ctx, bmp, posX, posY);
      cairo_paint(ctx);
      
      cairo_surface_destroy(bmp);
      FT_Done_Glyph(glyph);
      
      posX += maxAdv;
      if (posX > 14*maxAdv) {
        posX = maxAdv;
        posY += FONT_SIZE;
      }
  }
  
  std::string path = aName+".png";
  
  cairo_surface_flush(surface);
  cairo_surface_write_to_png(surface, path.c_str());
  cairo_surface_destroy(surface);
  cairo_destroy(ctx);
}
