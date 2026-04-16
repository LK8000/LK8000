/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GDL90/decoders/ahrs.h
 * Author: Bruno de Lacheisserie
 */

#pragma once

#include "../helpers.h"

#include <optional>

namespace gdl90_parser {

// ─────────────────────────────────────────────────────────────────────────────
// Levil AHRS  (MSG_LEVIL_AHRS  0x4C  — Levil / Stratux extension)
// Raw wire values; compare against the invalid_* sentinels before use,
// or call the typed getters which return std::nullopt when invalid.
// Angles/rates are ×10, g-load is ×100, airspeed is ×10.
// pressure_alt is encoded as (baro_alt_ft + 5000).
// ─────────────────────────────────────────────────────────────────────────────

inline constexpr uint8_t MSG_LEVIL_AHRS = 0x4C;

struct levil_ahrs {
  static constexpr int16_t invalid = static_cast<int16_t>(0x7FFF);
  static constexpr uint16_t invalid_alt = 0xFFFF;
  static constexpr int32_t pressure_alt_offset = 5000;

  int16_t roll = invalid;               // ×10°
  int16_t pitch = invalid;              // ×10°
  int16_t heading = invalid;            // ×10° magnetic
  int16_t slip_skid = invalid;          // ×10°
  int16_t yaw_rate = invalid;           // ×10°/s
  int16_t g_load = invalid;             // ×100 g
  int16_t airspeed = invalid;           // ×10 kt
  uint16_t pressure_alt = invalid_alt;  // encoded feet (baro_alt + 5000)
  int16_t vert_velocity = invalid;      // ft/min

  // Typed getters — pitch and roll always return a value (divide raw by 10);
  // all other getters return std::nullopt when the field is invalid.
  double get_roll() const noexcept {
    return roll / 10.0;
  }
  double get_pitch() const noexcept {
    return pitch / 10.0;
  }
  std::optional<double> get_heading() const noexcept {
    return heading != invalid ? std::optional{heading / 10.0} : std::nullopt;
  }
  std::optional<double> get_slip_skid() const noexcept {
    return slip_skid != invalid ? std::optional{slip_skid / 10.0}
                                : std::nullopt;
  }
  std::optional<double> get_yaw_rate() const noexcept {
    return yaw_rate != invalid ? std::optional{yaw_rate / 10.0} : std::nullopt;
  }
  std::optional<double> get_g_load() const noexcept {
    return g_load != invalid ? std::optional{g_load / 100.0} : std::nullopt;
  }
  std::optional<double> get_airspeed() const noexcept {
    return airspeed != invalid ? std::optional{airspeed / 10.0} : std::nullopt;
  }
  std::optional<int32_t> get_pressure_alt() const noexcept {
    return pressure_alt != invalid_alt
               ? std::optional{static_cast<int32_t>(pressure_alt) -
                               pressure_alt_offset}
               : std::nullopt;
  }
  std::optional<int16_t> get_vert_velocity() const noexcept {
    return vert_velocity != invalid ? std::optional{vert_velocity}
                                    : std::nullopt;
  }
};

template<> struct msg_id_for<levil_ahrs> {
  static constexpr uint8_t value = MSG_LEVIL_AHRS;
};

namespace detail {

[[nodiscard]]
inline levil_ahrs decode_levil_ahrs(frame_data payload) {
  if (payload.size() < 21) {
    throw too_short();
  }
  if (payload[0] != 0x45) {
    throw invalid_framing();
  }

  return {
    .roll = read_s16_be(payload, 3),
    .pitch = read_s16_be(payload, 5),
    .heading = read_s16_be(payload, 7),
    .slip_skid = read_s16_be(payload, 9),
    .yaw_rate = read_s16_be(payload, 11),
    .g_load = read_s16_be(payload, 13),
    .airspeed = read_s16_be(payload, 15),
    .pressure_alt = read_u16_be(payload, 17),
    .vert_velocity = read_s16_be(payload, 19)
  };
}

}  // namespace detail
}  // namespace gdl90_parser
