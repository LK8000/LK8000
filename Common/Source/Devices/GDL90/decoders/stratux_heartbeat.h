/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GDL90/decoders/stratux_heartbeat.h
 * Author: Bruno de Lacheisserie
 */

#pragma once

#include "../helpers.h"

#include <optional>

namespace gdl90_parser {

// ─────────────────────────────────────────────────────────────────────────────
// Stratux heartbeat  (MSG_STRATUX_HEARTBEAT  0xCC  — Stratux extension)
// Payload is 1 byte: [bits 7..2 protocol version][bit1 GPS valid][bit0 AHRS valid].
// ─────────────────────────────────────────────────────────────────────────────

inline constexpr uint8_t MSG_SX_HEARTBEAT = 0xCC;

struct stratux_heartbeat {
  uint8_t status = 0;

  [[nodiscard]]
  bool ahrs_valid() const noexcept {
    return (status & 0x01u) != 0;
  }

  [[nodiscard]]
  bool gps_valid() const noexcept {
    return (status & 0x02u) != 0;
  }

  [[nodiscard]]
  uint8_t protocol_version() const noexcept {
    return status >> 2;
  }
};

template<> struct msg_id_for<stratux_heartbeat> {
  static constexpr uint8_t value = MSG_SX_HEARTBEAT;
};

namespace detail {

[[nodiscard]]
inline stratux_heartbeat decode_stratux_heartbeat(frame_data payload) {
  if (payload.empty()) {
    throw too_short();
  }

  return {
    .status = payload[0]
  };
}

}  // namespace detail
}  // namespace gdl90_parser
