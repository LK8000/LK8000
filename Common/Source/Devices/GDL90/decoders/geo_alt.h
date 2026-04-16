/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GDL90/decoders/geo_alt.h
 * Author: Bruno de Lacheisserie
 */

#pragma once

#include "../helpers.h"

#include <limits>
#include <optional>

namespace gdl90_parser {

// ─────────────────────────────────────────────────────────────────────────────
// Ownship geometric altitude (MSG_OWNSHIP_GEO_ALT  0x0B)
// ─────────────────────────────────────────────────────────────────────────────

inline constexpr uint8_t MSG_OWNSHIP_GEO_ALT = 0x0B;

struct ownship_geo_alt {
  static constexpr int32_t invalid_geo_altitude =
      std::numeric_limits<int32_t>::min();

  int32_t geo_altitude;  // feet MSL (WGS-84), invalid_geo_altitude = invalid
  uint16_t vfom;         // vertical figure of merit (meters)
  bool vertical_warning = false;

  std::optional<int32_t> get_geo_altitude() const noexcept {
    return geo_altitude != invalid_geo_altitude ? std::optional{geo_altitude}
                                                : std::nullopt;
  }
};

template<> struct msg_id_for<ownship_geo_alt> {
  static constexpr uint8_t value = MSG_OWNSHIP_GEO_ALT;
};

namespace detail {

[[nodiscard]]
inline ownship_geo_alt decode_geo_alt(frame_data payload) {
  if (payload.size() < 4) {
    throw too_short();
  }

  ownship_geo_alt message{};
  const uint16_t raw = read_u16_be(payload, 0);
  if (raw == 0x7FFF) {
    message.geo_altitude = ownship_geo_alt::invalid_geo_altitude;
  }
  else {
    message.geo_altitude = static_cast<int32_t>(static_cast<int16_t>(raw)) * 5;
  }
  message.vfom = read_u16_be(payload, 2);

  if (payload.size() > 4) { // vertical warning is optional (added in v2.4)
    message.vertical_warning = (payload[4] & 0x01) != 0;
  }
  return message;
}

}  // namespace detail
}  // namespace gdl90_parser
