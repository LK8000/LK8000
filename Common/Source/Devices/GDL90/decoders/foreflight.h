/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GDL90/decoders/foreflight.h
 * Author: Bruno de Lacheisserie
 */

#pragma once

#include "../helpers.h"

#include <cstdint>
#include <optional>

namespace gdl90_parser {

// ─────────────────────────────────────────────────────────────────────────────
// ForeFlight ID (MSG_FOREFLIGHT_ID  0x65)
// ─────────────────────────────────────────────────────────────────────────────

inline constexpr uint8_t MSG_FOREFLIGHT_ID = 0x65;

struct foreflight_id {
  static constexpr uint64_t invalid_device_serial = 0;

  uint8_t sub_id;
  uint64_t device_serial;  // invalid_device_serial = not available
  char device_name[9];     // 8 chars + NUL

  std::optional<uint64_t> get_device_serial() const noexcept {
    return device_serial != invalid_device_serial ? std::optional{device_serial}
                                                  : std::nullopt;
  }
};

template<> struct msg_id_for<foreflight_id> {
  static constexpr uint8_t value = MSG_FOREFLIGHT_ID;
};

namespace detail {

[[nodiscard]]
inline foreflight_id decode_foreflight(frame_data payload) {
  if (payload.empty()) {
    throw too_short();
  }

  foreflight_id message{};
  message.sub_id = payload[0];
  if (message.sub_id != 0) {
    return message;
  }

  if (payload.size() < 18) {
    throw too_short();
  }

  for (int i = 0; i < 8; ++i) {
    message.device_serial = (message.device_serial << 8) | payload[2 + i];
    message.device_name[i] = static_cast<char>(payload[10 + i]);
  }
  message.device_name[8] = '\0';
  return message;
}

}  // namespace detail
}  // namespace gdl90_parser
