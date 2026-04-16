/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GDL90/decoders/stratux.h
 * Author: Bruno de Lacheisserie
 */

#pragma once

#include "../helpers.h"

namespace gdl90_parser {

// ─────────────────────────────────────────────────────────────────────────────
// Stratux status (MSG_STRATUX_STATUS  0x53)
// ─────────────────────────────────────────────────────────────────────────────

inline constexpr uint8_t MSG_STRATUX_STATUS = 0x53;

struct stratux_status {
  uint8_t protocol_version;
  uint8_t message_version;
  uint8_t version_major;
  uint8_t version_minor;
  uint8_t version_build_type;
  uint8_t version_build;
  uint8_t settings_flags;
  uint8_t status_flags;
  uint8_t connected_hardware_flags;
  uint8_t gps_satellites_locked;
  uint8_t gps_satellites_tracked;
  uint16_t uat_traffic_targets;
  uint16_t es_traffic_targets;
  uint16_t uat_messages_per_minute;
  uint16_t es_messages_per_minute;
  double cpu_temp_c;
  uint8_t num_towers;
};

template<> struct msg_id_for<stratux_status> {
  static constexpr uint8_t value = MSG_STRATUX_STATUS;
};

namespace detail {

[[nodiscard]]
inline stratux_status decode_stratux_status(frame_data payload) {
  if (payload.size() < 28) {
    throw too_short();
  }
  if (payload[0] != 'X') {
    throw invalid_framing();
  }

  return {
    .protocol_version = payload[1],
    .message_version = payload[2],
    .version_major = payload[3],
    .version_minor = payload[4],
    .version_build_type = payload[5],
    .version_build = payload[6],
    .settings_flags = payload[11],
    .status_flags = payload[12],
    .connected_hardware_flags = payload[14],
    .gps_satellites_locked = payload[15],
    .gps_satellites_tracked = payload[16],
    .uat_traffic_targets = read_u16_be(payload, 17),
    .es_traffic_targets = read_u16_be(payload, 19),
    .uat_messages_per_minute = read_u16_be(payload, 21),
    .es_messages_per_minute = read_u16_be(payload, 23),
    .cpu_temp_c = read_u16_be(payload, 25) / 10.0,
    .num_towers = payload[27]
  };
}

}  // namespace detail
}  // namespace gdl90_parser
