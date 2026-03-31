/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   airspace_config.cpp
 * Author: Bruno de Lacheisserie
 */

#include <string_view>
#include <string>
#include <charconv>
#include "airspace_config.h"
#include "MapWindow.h"
#include "Screen/LKColor.h"
#include "Screen/LKBrush.h"
#include "Screen/LKPen.h"
#include "LKObjects.h"
#include "MessageLog.h"
#include "Util/tstring.hpp"

using namespace std::string_view_literals;

bool airspace_config::operator!=(const airspace_config& other) const {
  if (modes != other.modes || colors != other.colors) {
    return true;
  }
#ifdef HAVE_HATCHED_BRUSH
  if (patterns != other.patterns) {
    return true;
  }
#endif
  return false;
}

void airspace_config::RotateSet(Airspace::Type type) {
  modes[type].rotate_set();
}

bool airspace_config::Display(Airspace::Type type) const {
  return get_mode_value(type, [](const airspace_mode& mode) {
    return mode.display();
  });
}

bool airspace_config::Warning(Airspace::Type type) const {
  return get_mode_value(type, [](const airspace_mode& mode) {
    return mode.warning();
  });
}

bool airspace_config::Display(Airspace::Type cls, Airspace::Type type) const {
  if (type == Airspace::Type::OTHER || type == Airspace::Type::NONE) {
    // OpenAir-V1 or OpenAip airspace source or OpenAir-V2 Airspace without type
    return Display(cls);
  }
  return Display(cls) || Display(type);
}

bool airspace_config::Warning(Airspace::Type cls, Airspace::Type type) const {
  if (type == Airspace::Type::OTHER || type == Airspace::Type::NONE) {
    // OpenAir-V1 or OpenAip airspace source or OpenAir-V2 Airspace without type
    return Warning(cls);
  }
  return Warning(cls) || Warning(type);
}

std::optional<LKColor> airspace_config::Color(Airspace::Type type) const {
  auto it = colors.find(type);
  if (it != colors.end()) {
    return {LKColor(it->second)};
  }
  return Airspace::to_color(type);
}

std::optional<LKColor> airspace_config::Color(Airspace::Type cls,
                                              Airspace::Type type) const {
  return Airspace::lookup(cls, type, [this](Airspace::Type type) {
    return Color(type);
  });
}

void airspace_config::Color(Airspace::Type type,
                            std::optional<RGB8Color> color) {
  if (!color) {
    colors.erase(type);
  }
  else {
    colors[type] = *color;
  }
}

#ifdef HAVE_HATCHED_BRUSH

std::optional<size_t> airspace_config::Pattern(Airspace::Type type) const {
  auto it = patterns.find(type);
  if (it != patterns.end()) {
    return it->second;
  }
  return Airspace::to_pattern(type);
}

std::optional<size_t> airspace_config::Pattern(Airspace::Type cls,
                                               Airspace::Type type) const {
  return Airspace::lookup(cls, type, [this](Airspace::Type type) {
    return Pattern(type);
  });
}

void airspace_config::Pattern(Airspace::Type type,
                              std::optional<size_t> pattern) {
  if (!pattern) {
    patterns.erase(type);
  }
  else {
    patterns[type] = *pattern;
  }
}

#endif

void airspace_config::Reset() {
  modes.clear();
  colors.clear();
#ifdef HAVE_HATCHED_BRUSH
  patterns.clear();
#endif
}

static constexpr auto mode_prefix = "airspace_mode_"sv;
static constexpr auto color_prefix = "airspace_color_"sv;

#ifdef HAVE_HATCHED_BRUSH
static constexpr auto pattern_prefix = "airspace_pattern_"sv;
#endif

/**
 * Legacy profile key migration (pre-refactor format):
 *   AirspaceMode0..18 -> airspace_mode_<Type>
 *   Colour0..18       -> airspace_color_<Type> (index into Colours[] palette)
 *   Brush0..18        -> airspace_pattern_<Type>
 *
 * Old index to new Airspace::Type mapping:
 *   0=OTHER, 1=R, 2=P, 3=Q, 4=A, 5=B, 6=C, 7=D,
 *   8=GP, 9=CTR, 10=W, 11=(AATASK), 12=E,
 *   13=(CLASSF), 14=G, 15=TMZ, 16=RMZ, 17=GSEC, 18=N
 */
static constexpr Airspace::Type legacy_index_to_type[] = {
    Airspace::Type::OTHER,  // 0  = OTHER
    Airspace::Type::R,      // 1  = RESTRICT
    Airspace::Type::P,      // 2  = PROHIBITED
    Airspace::Type::Q,      // 3  = DANGER
    Airspace::Type::A,      // 4  = CLASSA
    Airspace::Type::B,      // 5  = CLASSB
    Airspace::Type::C,      // 6  = CLASSC
    Airspace::Type::D,      // 7  = CLASSD
    Airspace::Type::GP,     // 8  = NOGLIDER
    Airspace::Type::CTR,    // 9  = CTR
    Airspace::Type::W,      // 10 = WAVE
    Airspace::Type::TSKSEC, // 11 = AATASK (internal)
    Airspace::Type::E,      // 12 = CLASSE
    Airspace::Type::F,      // 13 = CLASSF
    Airspace::Type::G,      // 14 = CLASSG
    Airspace::Type::TMZ,    // 15 = CLASSTMZ
    Airspace::Type::RMZ,    // 16 = CLASSRMZ
    Airspace::Type::GSEC,   // 17 = GLIDERSECT
    Airspace::Type::N,      // 18 = CLASSNOTAM
};

static constexpr auto legacy_mode_prefix = "AirspaceMode"sv;
static constexpr auto legacy_colour_prefix = "Colour"sv;
#ifdef HAVE_HATCHED_BRUSH
static constexpr auto legacy_brush_prefix = "Brush"sv;
#endif

void airspace_config::SaveSettings(settings::writer& writer_settings) const {
  for (const auto& [type, mode] : modes) {
    std::string name(mode_prefix);
    name += Airspace::to_string(type);
    writer_settings(name.c_str(), mode.to_unsigned());
  }

  for (auto& [type, c] : colors) {
    std::string name(color_prefix);
    name += Airspace::to_string(type);
    uint32_t value = (c.Blue() << 16) | (c.Green() << 8) | c.Red();
    writer_settings(name.c_str(), value);
  }

#ifdef HAVE_HATCHED_BRUSH
  for (auto& [type, pattern] : patterns) {
    std::string name(pattern_prefix);
    name += Airspace::to_string(type);
    writer_settings(name.c_str(), pattern);
  }
#endif
}

bool airspace_config::LoadSettings(const std::string_view& key,
                                   const char* value) {
  try {
    // New format keys: airspace_mode_<Type>, airspace_color_<Type>, airspace_pattern_<Type>
    if (key.starts_with(mode_prefix)) {
      std::string_view sv_type = key.substr(mode_prefix.size());
      auto type = Airspace::from_string(sv_type);
      modes[type] = value;
      return true;
    }

    if (key.starts_with(color_prefix)) {
      std::string_view sv_type = key.substr(color_prefix.size());
      auto type = Airspace::from_string(sv_type);
      uint32_t c = std::stoul(value);
      colors[type] = {
        static_cast<uint8_t>(c & 0xFF),
        static_cast<uint8_t>((c >> 8) & 0xFF),
        static_cast<uint8_t>((c >> 16) & 0xFF)
      };
      return true;
    }

#ifdef HAVE_HATCHED_BRUSH
    if (key.starts_with(pattern_prefix)) {
      std::string_view sv_type = key.substr(pattern_prefix.size());
      auto type = Airspace::from_string(sv_type);
      patterns[type] = std::stoul(value);
      return true;
    }
#endif

    // Legacy format migration: AirspaceModeN, ColourN, BrushN
    if (key.starts_with(legacy_mode_prefix)) {
      auto sv_idx = key.substr(legacy_mode_prefix.size());
      unsigned idx = 0;
      auto [ptr, ec] = std::from_chars(sv_idx.data(), sv_idx.data() + sv_idx.size(), idx);
      if (ec == std::errc() && idx < std::size(legacy_index_to_type)) {
        auto type = legacy_index_to_type[idx];
        if (type != Airspace::Type::NONE) {
          modes[type] = value;
        }
        return true;
      }
    }

    if (key.starts_with(legacy_colour_prefix)) {
      auto sv_idx = key.substr(legacy_colour_prefix.size());
      unsigned idx = 0;
      auto [ptr, ec] = std::from_chars(sv_idx.data(), sv_idx.data() + sv_idx.size(), idx);
      if (ec == std::errc() && idx < std::size(legacy_index_to_type)) {
        auto type = legacy_index_to_type[idx];
        if (type != Airspace::Type::NONE) {
          // Old format stored an index into MapWindow::Colours[] palette
          unsigned color_idx = std::stoul(value);
          if (color_idx < std::size(MapWindow::Colours)) {
            colors[type] = MapWindow::Colours[color_idx];
          }
        }
        return true;
      }
    }

#ifdef HAVE_HATCHED_BRUSH
    if (key.starts_with(legacy_brush_prefix)) {
      auto sv_idx = key.substr(legacy_brush_prefix.size());
      unsigned idx = 0;
      auto [ptr, ec] = std::from_chars(sv_idx.data(), sv_idx.data() + sv_idx.size(), idx);
      if (ec == std::errc() && idx < std::size(legacy_index_to_type)) {
        auto type = legacy_index_to_type[idx];
        if (type != Airspace::Type::NONE) {
          patterns[type] = std::stoul(value);
        }
        return true;
      }
    }
#endif
  }
  catch (std::exception& e) {
    DebugLog(
        _T("Failed to load airspace config setting: key=%s, value=%s, ")
        _T("error=%s"),
        to_tstring(key).c_str(), to_tstring(value).c_str(),
        to_tstring(e.what()).c_str());
  }
  return false;
}
