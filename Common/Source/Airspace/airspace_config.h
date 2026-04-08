/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   airspace_config.h
 * Author: Bruno de Lacheisserie
 */

#pragma once
#include <unordered_map>
#include <optional>
#include "Airspace.h"
#include "airspace_mode.h"
#include "Settings/write.h"
#include "Screen/PortableColor.hpp"
#include "Screen/Features.hpp"

class LKBrush;
class LKPen;

class airspace_config {
  using map_mode_t = std::unordered_map<Airspace::Type, airspace_mode>;
  using map_color_t = std::unordered_map<Airspace::Type, RGB8Color>;
#ifdef HAVE_HATCHED_BRUSH
  using map_pattern_t = std::unordered_map<Airspace::Type, size_t>;
#endif

 public:

  bool operator!=(const airspace_config& other) const;

  void RotateSet(Airspace::Type type);

  bool Display(Airspace::Type type) const;
  bool Display(Airspace::Type cls, Airspace::Type type) const;

  bool Warning(Airspace::Type type) const;
  bool Warning(Airspace::Type cls, Airspace::Type type) const;

  std::optional<LKColor> Color(Airspace::Type type) const;
  std::optional<LKColor> Color(Airspace::Type cls, Airspace::Type type) const;

  void Color(Airspace::Type type, std::optional<RGB8Color> color);

#ifdef HAVE_HATCHED_BRUSH
  std::optional<size_t> Pattern(Airspace::Type type) const;
  std::optional<size_t> Pattern(Airspace::Type cls, Airspace::Type type) const;

  void Pattern(Airspace::Type type, std::optional<size_t> pattern);
#endif

  void Reset();

  void SaveSettings(settings::writer& writer_settings) const;
  bool LoadSettings(const std::string_view& key, const char *value);

 private:
  map_mode_t modes;
  map_color_t colors;
#ifdef HAVE_HATCHED_BRUSH
  map_pattern_t patterns;
#endif

  template<typename Func>
  bool get_mode_value(Airspace::Type type, Func func) const {
    auto it = modes.find(type);
    if (it != modes.end()) {
      return func(it->second);
    }
    return true;
  }
};
