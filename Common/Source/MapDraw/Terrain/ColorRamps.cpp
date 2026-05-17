/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   ColorRamps.cpp
 * Author: Bruno de Lacheisserie
 */

#include "options.h"
#include "ColorRamps.h"
#include <cstdint>
#include <algorithm>

namespace {

template<typename T>
constexpr bool is_power_of_two(T v) {
  static_assert(std::is_integral<T>::value, "is_power_of_two : T must be integral");
  return v && ((v & (v - 1)) == 0);
}

template<uint32_t level>
uint8_t linear_interpolation(uint8_t a, uint8_t b, uint32_t f) {
  static_assert(is_power_of_two(level), "linear_interpolation : level must be power of 2");
  const uint32_t of = level - f;

  return (f * b + of * a) / level;
}

template <uint32_t level>
RGB8Color linear_interpolation(const RGB8Color& a,
                                      const RGB8Color& b,
                                      uint32_t f) {
  return {
    linear_interpolation<level>(a.Red(), b.Red(), f),
    linear_interpolation<level>(a.Green(), b.Green(), f),
    linear_interpolation<level>(a.Blue(), b.Blue(), f)
  };
}

template <uint32_t level>
Luminosity8 linear_interpolation(const Luminosity8& a,
                                        const Luminosity8& b,
                                        uint32_t f) {
  return linear_interpolation<level>(a.GetLuminosity(), b.GetLuminosity(), f);
}

const auto terrain_color_ramps = std::to_array<TerrainColorRamp>({
     {// Low lands
      .colors = {{
#ifdef DITHER
          {0, {0xff, 0xff, 0xff}},
          {250, {0xff, 0xff, 0xff}},
#else
          {0, {0x70, 0xc0, 0xa7}},
          {250, {0xca, 0xe7, 0xb9}},
#endif
          {500, {0xf4, 0xea, 0xaf}},
          {750, {0xdc, 0xb2, 0x82}},
          {1000, {0xca, 0x8e, 0x72}},
          {1250, {0xde, 0xc8, 0xbd}},
          {1500, {0xe3, 0xe4, 0xe9}},
          {1750, {0xdb, 0xd9, 0xef}},
          {2000, {0xce, 0xcd, 0xf5}},
          {2250, {0xc2, 0xc1, 0xfa}},
          {2500, {0xb7, 0xb9, 0xff}},
      }},
      .shadow = {63, {60, 10, 10}},
      .highlight = {255, {0, 0, 0}},
      .doShading = true,
      .useMinAlt = true,
      .name = MsgToken<413>},
     {// LK Mountain 101016
      .colors = {{
#ifdef DITHER
          {0, {0xff, 0xff, 0xff}},
          {350, {0xff, 0xff, 0xff}},
#else
          {0, {0x70, 0xc0, 0xa7}},
          {350, {0xca, 0xe7, 0xb9}},
#endif
          {700, {0xf4, 0xea, 0xaf}},
          {1050, {0xdc, 0xb2, 0x82}},
          {1400, {0xd1, 0x9a, 0x5c}},
          {1750, {0xca, 0x8e, 0x72}},
          {2100, {0x9b, 0x59, 0x3b}},
          {2450, {0xde, 0xc8, 0xbd}},
          {2800, {0xe3, 0xe4, 0xe9}},
          {3150, {0xdb, 0xd9, 0xef}},
          {3500, {0xce, 0xcd, 0xf5}},
          {3850, {0xb7, 0xb9, 0xff}},
          {7000, {0xff, 0xff, 0xff}},
      }},
      .shadow = {63, {60, 10, 10}},
      .highlight = {255, {0, 0, 0}},
      .doShading = true,
      .useMinAlt = true,
      .name = MsgToken<439>},
     {// Imhof Type 7, geomteric 1.35 9
      .colors = {{
          {0, {153, 178, 169}},
          {368, {180, 205, 181}},
          {496, {225, 233, 192}},
          {670, {255, 249, 196}},
          {905, {255, 249, 196}},
          {1222, {255, 219, 173}},
          {1650, {254, 170, 136}},
          {2227, {253, 107, 100}},
          {3007, {255, 255, 255}},
      }},
      .shadow = {63, {0, 0, 64}},
      .highlight = {255, {0, 0, 0}},
      .doShading = true,
      .useMinAlt = true,
      .name =
          []() {
            return _T("Imhof 7");
          }},
     {// Imhof Type 4, geomteric 1.5 8
      .colors = {{
          {0, {175, 224, 203}},
          {264, {211, 237, 211}},
          {396, {254, 254, 234}},
          {594, {252, 243, 210}},
          {891, {237, 221, 195}},
          {1336, {221, 199, 175}},
          {2004, {215, 170, 148}},
          {3007, {255, 255, 255}},
      }},
      .shadow = {63, {0, 0, 64}},
      .highlight = {255, {0, 0, 0}},
      .doShading = true,
      .useMinAlt = true,
      .name =
          []() {
            return _T("Imhof 4");
          }},
     {// Imhof Type 12, geomteric  1.5 8
      .colors = {{
          {0, {165, 220, 201}},
          {399, {219, 239, 212}},
          {558, {254, 253, 230}},
          {782, {254, 247, 211}},
          {1094, {254, 237, 202}},
          {1532, {254, 226, 207}},
          {2145, {254, 209, 204}},
          {3004, {255, 255, 255}},
      }},
      .shadow = {63, {0, 0, 64}},
      .highlight = {255, {0, 0, 0}},
      .doShading = true,
      .useMinAlt = true,
      .name =
          []() {
            return _T("Imhof 12");
          }},
     {// Imhof Atlas der Schweiz
      .colors = {{
          {0, {47, 101, 147}},
          {368, {58, 129, 152}},
          {496, {117, 148, 153}},
          {670, {155, 178, 140}},
          {905, {192, 190, 139}},
          {1222, {215, 199, 137}},
          {1650, {229, 203, 171}},
          {2227, {246, 206, 171}},
          {3007, {252, 246, 244}},
      }},
      .shadow = {63, {0, 0, 64}},
      .highlight = {255, {0, 0, 0}},
      .doShading = true,
      .useMinAlt = false,
      .name =
          []() {
            return _T("Imhof Atlas");
          }},
     {// ICAO
      .colors = {{
          {0, {180, 205, 181}},
          {199, {180, 205, 181}},
          {200, {225, 233, 192}},
          {499, {225, 233, 192}},
          {500, {255, 249, 196}},
          {999, {255, 249, 196}},
          {1000, {255, 219, 173}},
          {1499, {255, 219, 173}},
          {1500, {254, 170, 136}},
          {1999, {254, 170, 136}},
          {2000, {253, 107, 100}},
          {2499, {253, 107, 100}},
          {2500, {255, 255, 255}},
      }},
      .shadow = {63, {0, 0, 64}},
      .highlight = {32, {255, 255, 16}},
      .doShading = true,
      .useMinAlt = false,
      .name =
          []() {
            return _T("ICAO");
          }},
     {// LKoogle lowlands
      .colors = {{
#ifdef DITHER
          {0, {0xff, 0xff, 0xff}},
          {250, {0xff, 0xff, 0xff}},
#else
          {0, {222, 226, 203}},
          {250, {222, 226, 203}},
#endif
          {500, {180, 180, 180}},
          {750, {170, 170, 170}},
          {1000, {160, 160, 160}},
          {1250, {140, 140, 140}},
          {1500, {130, 130, 130}},
          {1750, {120, 120, 120}},
          {2000, {190, 190, 190}},
          {2250, {215, 215, 215}},
          {2500, {240, 240, 240}},
      }},
      .shadow = {63, {16, 32, 32}},
      .highlight = {255, {0, 0, 0}},
      .doShading = true,
      .useMinAlt = true,
      .name = MsgToken<377>},
     {// LKoogle mountains
      .colors = {{
#ifdef DITHER
          {0, {0xff, 0xff, 0xff}},
          {500, {0xff, 0xff, 0xff}},
#else
          {0, {222, 226, 203}},
          {500, {222, 226, 203}},
#endif
          {1000, {180, 180, 180}},
          {1500, {160, 160, 160}},
          {2000, {140, 140, 140}},
          {2500, {120, 120, 120}},
          {3000, {190, 190, 190}},
          {3500, {215, 215, 215}},
          {4000, {240, 240, 240}},
      }},
      .shadow = {63, {16, 32, 32}},
      .highlight = {255, {0, 0, 0}},
      .doShading = true,
      .useMinAlt = true,
      .name = MsgToken<378>},
     {// Low Alps
      .colors = {{
#ifdef DITHER
          {0, {0xff, 0xff, 0xff}},
          {250, {0xff, 0xff, 0xff}},
#else
          {0, {0x70, 0xc0, 0xa7}},
          {250, {0xca, 0xe7, 0xb9}},
#endif
          {500, {0xf4, 0xea, 0xaf}},
          {750, {0xdc, 0xb2, 0x82}},
          {1000, {0xca, 0x8e, 0x72}},
          {1250, {180, 180, 180}},
          {1500, {160, 160, 160}},
          {1750, {150, 150, 150}},
          {2000, {140, 140, 140}},
          {2250, {130, 130, 130}},
          {2500, {200, 200, 200}},
          {3000, {220, 220, 220}},
          {4000, {240, 240, 240}},
      }},
      .shadow = {63, {16, 32, 32}},
      .highlight = {255, {0, 0, 0}},
      .doShading = true,
      .useMinAlt = true,
      .name = MsgToken<412>},
     {// Alps
      .colors = {{
#ifdef DITHER
          {150, {0xff, 0xff, 0xff}},
          {500, {0xff, 0xff, 0xff}},
#else
          {150, {0x70, 0xc0, 0xa7}},
          {500, {0xca, 0xe7, 0xb9}},
#endif
          {650, {0xf4, 0xea, 0xaf}},
          {950, {0xdc, 0xb2, 0x82}},
          {1100, {0xca, 0x8e, 0x72}},
          {1500, {175, 175, 175}},
          {1750, {165, 165, 165}},
          {2000, {155, 155, 155}},
          {2500, {235, 235, 235}},
          {3000, {245, 245, 245}},
          {4000, {255, 255, 255}},
      }},
      .shadow = {63, {16, 32, 32}},
      .highlight = {255, {0, 0, 0}},
      .doShading = true,
      .useMinAlt = true,
      .name = MsgToken<338>},
     {// YouSee
      .colors = {{
#ifdef DITHER
          {0, {0xff, 0xff, 0xff}},
#else
          {0, {112, 191, 170}},
#endif
          {800, {254, 255, 188}},
          {1900, {194, 135, 93}},
          {2900, {230, 230, 228}},
          {4900, {186, 185, 251}},
          {6000, {255, 255, 255}},
      }},
      .shadow = {63, {16, 32, 32}},
      .highlight = {255, {0, 0, 0}},
      .doShading = true,
      .useMinAlt = false,
      .name =
          []() {
            return _T("YouSee");
          }},
     {// HighContrast
      .colors = {{
#ifdef DITHER
          {0, {0xff, 0xff, 0xff}},
#else
          {0, {235, 255, 235}},
#endif
          {100, {197, 216, 246}},
          {200, {0, 170, 0}},
          {300, {0, 128, 0}},
          {400, {0, 85, 0}},
          {500, {51, 119, 0}},
          {700, {153, 187, 0}},
          {900, {255, 255, 0}},
          {1000, {241, 227, 0}},
          {1300, {199, 142, 0}},
          {1800, {128, 0, 0}},
          {3100, {255, 255, 255}},
          {4900, {160, 191, 237}},
      }},
      .shadow = {63, {60, 60, 60}},
      .highlight = {63, {250, 250, 250}},
      .doShading = true,
      .useMinAlt = true,
      .name = MsgToken<340>},
     {// GA Relative
      .colors = {{
#ifdef DITHER
          {0, {0xff, 0xff, 0xff}},
#else
          {0, {227, 255, 224}},
#endif
          {50, {227, 255, 224}},
          {51, {255, 255, 0}},
          {120, {255, 255, 0}},
          {149, {255, 100, 50}},
          {150, {255, 0, 0}},  // 0m
          {300, {255, 0, 0}},
          {500, {220, 0, 0}},
          {700, {200, 0, 0}},
          {900, {180, 0, 0}},
          {1100, {150, 0, 0}},
          {1300, {120, 0, 0}},
          {3500, {100, 0, 0}},
      }},
      .shadow = {63, {60, 60, 60}},
      .highlight = {255, {0, 0, 0}},
      .doShading = false,
      .useMinAlt = true,
      .name =
          []() {
            return _T("GA Relative");
          }},
     {// LiteAlps
      .colors = {{
#ifdef DITHER
          {0, {0xff, 0xff, 0xff}},
          {250, {0xff, 0xff, 0xff}},
#else
          {0, {0xff, 0xff, 0xff}},
          {250, {0xca, 0xe7, 0xb9}},
#endif
          {400, {0x3d, 0xb3, 0x8b}},
          {550, {0x7f, 0xc7, 0x5c}},
          {700, {0x37, 0xa0, 0x7d}},
          {850, {0xf4, 0xea, 0xaf}},
          {1250, {0xdc, 0xb2, 0x82}},
          {1500, {0xca, 0x8e, 0x72}},
          {1750, {0xde, 0xc8, 0xbd}},
          {2000, {0xe3, 0xe4, 0xe9}},
          {2300, {0xdb, 0xd9, 0xef}},
          {3000, {0xb7, 0xb9, 0xff}},
          {6000, {0xef, 0xef, 0xff}},
      }},
      .shadow = {63, {60, 10, 10}},
      .highlight = {255, {0, 0, 0}},
      .doShading = true,
      .useMinAlt = true,
      .name =
          []() {
            return _T("LiteAlps");
          }},
     {// Low Hills
      .colors = {{
#ifdef DITHER
          {0, {0xff, 0xff, 0xff}},
          {50, {0xff, 0xff, 0xff}},
#else
          {0, {0xff, 0xff, 0xff}},
          {25, {0xff, 0xff, 0xff}},
#endif
          {150, {0xca, 0xe7, 0xb9}},   // very light green
          {200, {0x3d, 0xb3, 0x8b}},   // darker green
          {250, {0x7f, 0xc7, 0x5c}},   // green/yellow
          {300, {0x37, 0xa0, 0x7d}},   // green
          {350, {0xf4, 0xea, 0xaf}},   // yellow
          {400, {0xdc, 0xb2, 0x82}},   // yellow brown
          {550, {0x99, 0x33, 0x00}},   // brown
          {650, {0xca, 0x8e, 0x72}},   // light brown
          {750, {0xde, 0xc8, 0xbd}},   // very light brown
          {1000, {0xe3, 0xe4, 0xe9}},  // light ice
      }},
      .shadow = {63, {60, 10, 10}},
      .highlight = {255, {0, 0, 0}},
      .doShading = true,
      .useMinAlt = true,
      .name =
          []() {
            return _T("Low Hills");
          }},
     {// Low Alps color e-ink
      .colors = {{
          {0, {0xff, 0xff, 0xff}},
          {500, {0x70, 0xc0, 0xa7}},
          {750, {0xca, 0xe7, 0xb9}},
          {1000, {0xf4, 0xea, 0xaf}},
          {1250, {0xdc, 0xb2, 0x82}},
          {1500, {0xca, 0x8e, 0x72}},
          {1750, {180, 180, 180}},
          {2000, {140, 140, 140}},
          {2250, {130, 130, 130}},
          {2500, {200, 200, 200}},
          {3000, {220, 220, 220}},
          {4000, {240, 240, 240}},
      }},
      .shadow = {63, {16, 32, 32}},
      .highlight = {255, {0, 0, 0}},
      .doShading = true,
      .useMinAlt = true,
      .name =
          []() {
            return _T("Low Alps color e-ink");
          }},
     {// Low Alps gray e-ink
      .colors = {{
          {0, {0xff, 0xff, 0xff}},
          {250, {0xff, 0xff, 0xff}},
          {500, {0xf4, 0xea, 0xaf}},
          {750, {0xdc, 0xb2, 0x82}},
          {1000, {0xca, 0x8e, 0x72}},
          {1250, {180, 180, 180}},
          {1500, {160, 160, 160}},
          {1750, {150, 150, 150}},
          {2000, {140, 140, 140}},
          {2250, {130, 130, 130}},
          {2500, {200, 200, 200}},
          {3000, {220, 220, 220}},
          {4000, {240, 240, 240}},
      }},
      .shadow = {63, {16, 32, 32}},
      .highlight = {255, {0, 0, 0}},
      .doShading = true,
      .useMinAlt = true,
      .name = 
          []() {
            return _T("Low Alps gray e-ink");
          }},
});

}  // namespace

const TerrainColorRamp* TerrainColorRamps::begin() noexcept {
  return terrain_color_ramps.begin();
}

const TerrainColorRamp* TerrainColorRamps::end() noexcept {
  return terrain_color_ramps.end();
}

short TerrainColorRamps::size() noexcept {
  static_assert(terrain_color_ramps.size() <= std::numeric_limits<short>::max(),
                "TerrainColorRamps::size() : size exceeds short max value");
  return static_cast<short>(terrain_color_ramps.size());
}

const TerrainColorRamp& TerrainColorRamps::at(short index) noexcept {
  const size_t last = terrain_color_ramps.size() - 1U;

  if (index <= 0) {
    return terrain_color_ramps[0];
  }

  const size_t uindex = static_cast<size_t>(index);
  if (uindex >= terrain_color_ramps.size()) {
    return terrain_color_ramps[last];
  }
  return terrain_color_ramps[uindex];
}

TerrainColor ColorRampLookup(const int16_t h,
                             const TerrainColorRamp& ramp_colors) {
  constexpr uint32_t interp_level = 1 << 16;

  const auto begin = std::begin(ramp_colors.colors);
  const auto end = std::end(ramp_colors.colors);

  // find first value with height > h
  auto it = std::upper_bound(begin, end, h,
                             [](int16_t height, const height_color_t& v) {
                               return height < v.height;
                             });

  if (it != begin) {
    auto prev = std::prev(it);
    if (it != end && it->height != prev->height) {
      // interpolate color
      const uint32_t f =
          (h - prev->height) * interp_level / (it->height - prev->height);
      return linear_interpolation<interp_level>(prev->color, it->color, f);
    }
    else {
      return prev->color;  // last defined color or no need to interpolate
    }
  }

  return begin->color;  // first defined color
}

TerrainColor TerrainShading(const height_color_t& tshadow,
                            const height_color_t& thighlight,
                            const int16_t illum,
                            const TerrainColor& color) {

  constexpr uint32_t interp_level = 128;

  if (illum < 0) {
    // shadow to "tshadow.color"
    const uint32_t x = std::min<uint32_t>(tshadow.height, -illum);
    return linear_interpolation<interp_level>(color, tshadow.color, x);
  }
  else if (illum > 0) {
    // highlight "thighlight.color"
    if (thighlight.height != 255) {
      const uint32_t x = std::min<uint32_t>(thighlight.height, illum / 2);
      return linear_interpolation<interp_level>(color, thighlight.color, x);
    }
  }
  return color;
}

#ifdef GREYSCALE

Luminosity8 terrain_lightness(const Luminosity8& color, double lightness) {
  if (lightness == 1.0) {
    return color;
  }
  return {
    static_cast<uint8_t>(std::min<uint32_t>(color.GetLuminosity() * lightness, 255))
  };
}

#else
RGB8Color terrain_lightness( const RGB8Color& color, float lightness) {
  if (lightness == 1.0F) {
    return color;
  }

  uint8_t r = color.Red();
  uint8_t g = color.Green();
  uint8_t b = color.Blue();
  rgb_lightness(r, g, b, lightness);

  return { r, g, b };
}

#endif

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

// check if ramp_colors is valid (height sorted).
static bool IsValidColorRamp(const TerrainColorRamp& ramp_colors) {
  const auto begin = std::begin(ramp_colors.colors);
  const auto end = std::end(ramp_colors.colors);

  return std::is_sorted(begin, end,
                        [](const height_color_t& a, const height_color_t& b) {
                          return a.height < b.height;
                        });
}

TEST_SUITE("Terrain renderer") {
  TEST_CASE("color ramp") {
    SUBCASE("ramp height sorted") {
      for (const auto& ramp : TerrainColorRamps()) {
        CHECK(IsValidColorRamp(ramp));
      }
    }
  }
}

#endif
