/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#ifndef _ColorRamps_h_
#define _ColorRamps_h_

#include <vector>
#include "Screen/PortableColor.hpp"
#include "LKLanguage.h"

#ifdef GREYSCALE
using TerrainColor = Luminosity8;
#else
using TerrainColor = RGB8Color;
#endif

struct height_color_t {
  int16_t height;
  TerrainColor color;
};

struct TerrainColorRamp {
  std::vector<height_color_t> colors;
  height_color_t shadow;
  height_color_t highlight;
  bool doShading;
  bool useMinAlt;
  MsgToken_t name;
};

/**
 * Helper class to access the global collection of terrain color ramps
 */
struct TerrainColorRamps {
  static const TerrainColorRamp* begin() noexcept;
  static const TerrainColorRamp* end() noexcept;
  static short size() noexcept;

  static const TerrainColorRamp& at(short index) noexcept;
};

TerrainColor ColorRampLookup(int16_t h, const TerrainColorRamp& ramp_colors);

TerrainColor TerrainShading(const height_color_t& tshadow,
                            const height_color_t& thighlight,
                            const int16_t illum,
                            const TerrainColor& color);

void rgb_lightness(uint8_t& r, uint8_t& g, uint8_t& b, float light);

Luminosity8 terrain_lightness(const Luminosity8& color, double lightness);
RGB8Color terrain_lightness(const RGB8Color& color, float lightness);

#endif  // _ColorRamps_h_
