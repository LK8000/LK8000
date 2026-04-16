/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GDL90/decoders/heartbeat.h
 * Author: Bruno de Lacheisserie
 */

#pragma once

#include "../helpers.h"

namespace gdl90_parser {

// ─────────────────────────────────────────────────────────────────────────────
// Heartbeat message (MSG_HEARTBEAT  0x00)
// ─────────────────────────────────────────────────────────────────────────────

inline constexpr uint8_t MSG_HEARTBEAT = 0x00;

struct heartbeat {
  uint8_t status_byte1;  // GPS pos valid, maintenance req, IDENT, ...
  uint8_t status_byte2;
  uint16_t timestamp;    // seconds since 00:00 UTC (17-bit, bit16 in status_byte2 bit7)
  uint16_t message_counts;  // uplink + basic/long UAT

  bool gps_pos_valid() const noexcept {
    return (status_byte1 & 0x80) != 0;
  }
  bool maintenance_req() const noexcept {
    return (status_byte1 & 0x40) != 0;
  }
  bool ident() const noexcept {
    return (status_byte1 & 0x20) != 0;
  }
  bool addr_type() const noexcept {
    return (status_byte1 & 0x10) != 0;
  }
  bool gps_batt_low() const noexcept {
    return (status_byte1 & 0x08) != 0;
  }
  bool ratcs() const noexcept {
    return (status_byte1 & 0x04) != 0;
  }
  bool timestamp_ms_bit() const noexcept {
    return (status_byte2 & 0x80) != 0;
  }
  // Full 17-bit UTC time-of-day in seconds since 00:00 UTC
  uint32_t utc_seconds() const noexcept {
    return static_cast<uint32_t>(timestamp) |
           (timestamp_ms_bit() ? (1u << 16) : 0u);
  }
};

template<> struct msg_id_for<heartbeat> {
  static constexpr uint8_t value = MSG_HEARTBEAT;
};

namespace detail {

[[nodiscard]]
inline heartbeat decode_heartbeat(frame_data payload) {
  if (payload.size() < 6) {
    throw too_short();
  }

  return {
    .status_byte1 = payload[0],
    .status_byte2 = payload[1],
    .timestamp = read_u16_le(payload, 2),
    .message_counts = read_u16_le(payload, 4)
  };
}

}  // namespace detail
}  // namespace gdl90_parser
