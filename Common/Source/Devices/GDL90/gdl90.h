/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GDL90/gdl90.h
 * Author: Bruno de Lacheisserie
 */

#pragma once
/**
 * GDL90 decoder library
 *
 * Usage:
 *   auto result = gdl90_parser::decode_frame(raw_bytes);
 *   std::visit(gdl90_parser::MessageVisitor{...}, result);
 *
 * Supported messages:
 *   0x00  Heartbeat
 *   0x0A  Ownship Report
 *   0x0B  Ownship Geo Altitude
 *   0x14  Traffic Report
 *   0x53  Stratux Status
 *   0x65  Foreflight ID
 *   0x4C  Levil AHRS report (Levil / Stratux extension)
 *   0xCC  Stratux heartbeat/status (Stratux extension)
 *
 * Standards:
 *   - GDL 90 Data Interface Specification (Garmin 560-1058-00 Rev A)
 *   - RTCA DO-282B
 */

#include "types.h"
#include "helpers.h"
#include "decoders/heartbeat.h"
#include "decoders/traffic.h"
#include "decoders/geo_alt.h"
#include "decoders/foreflight.h"
#include "decoders/stratux.h"
#include "decoders/ahrs.h"
#include "decoders/stratux_heartbeat.h"

#include <array>
#include <concepts>
#include <span>
#include <variant>
#include <vector>

namespace gdl90_parser {

// ─────────────────────────────────────────────────────────────────────────────
// Catch-all for unrecognised message IDs
// ─────────────────────────────────────────────────────────────────────────────

struct unknown_msg {
  uint8_t id;
  std::vector<uint8_t> payload;
};

// ─────────────────────────────────────────────────────────────────────────────
// Decode result variant  (all message types + catch-all)
// ─────────────────────────────────────────────────────────────────────────────

using message = std::variant<heartbeat,         // MSG_HEARTBEAT         (0x00)
                             ownship_report,    // MSG_OWNSHIP           (0x0A)
                             traffic_report,    // MSG_TRAFFIC           (0x14)
                             ownship_geo_alt,   // MSG_OWNSHIP_GEO_ALT   (0x0B)
                             foreflight_id,     // MSG_FOREFLIGHT_ID     (0x65)
                             stratux_status,    // MSG_STRATUX_STATUS    (0x53)
                             stratux_heartbeat, // MSG_SX_HEARTBEAT      (0xCC)
                             levil_ahrs,        // MSG_LEVIL_AHRS        (0x4C)
                             unknown_msg>;

namespace detail {

using decoder_fn = message (*)(frame_data);

template <auto Fn>
concept MessageDecoder = requires(frame_data payload) {
  {Fn(payload)}->std::convertible_to<message>;
};

template <auto Fn>
requires MessageDecoder<Fn>
inline message make_decoder(frame_data payload) {
  return Fn(payload);
}

struct decoder_entry {
  uint8_t id;
  decoder_fn fn;
};

inline constexpr auto k_decoders = std::to_array<decoder_entry>({
    {MSG_TRAFFIC, make_decoder<decode_traffic_report>},
    {MSG_HEARTBEAT, make_decoder<decode_heartbeat>},
    {MSG_OWNSHIP, make_decoder<decode_ownship_report>},
    {MSG_OWNSHIP_GEO_ALT, make_decoder<decode_geo_alt>},
    {MSG_STRATUX_STATUS, make_decoder<decode_stratux_status>},
    {MSG_FOREFLIGHT_ID, make_decoder<decode_foreflight>},
    {MSG_SX_HEARTBEAT, make_decoder<decode_stratux_heartbeat>},
    {MSG_LEVIL_AHRS, make_decoder<decode_levil_ahrs>},
});

[[nodiscard]]
inline message dispatch(uint8_t id, frame_data payload) {
  for (const auto& [entry_id, fn] : k_decoders) {
    if (entry_id == id) {
      return fn(payload);
    }
  }
  return unknown_msg{id, {payload.begin(), payload.end()}};
}

}  // namespace detail

[[nodiscard]]
inline message decode_frame(detail::frame_data raw) {
  if (raw.size() < 5) {
    throw too_short();
  }
  if (raw.front() != FLAG_BYTE || raw.back() != FLAG_BYTE) {
    throw invalid_framing();
  }

  const auto stuffed = raw.subspan(1, raw.size() - 2);
  const auto bytes = detail::unstuff(stuffed);

  if (bytes.size() < 3) {
    throw too_short();
  }

  const size_t payload_len = bytes.size() - 2;
  const uint16_t computed = detail::crc16({bytes.data(), payload_len});
  const uint16_t received = detail::read_u16_le(bytes, payload_len);
  if (computed != received) {
    throw bad_crc();
  }

  const uint8_t msg_id = bytes[0];
  return detail::dispatch(msg_id, {bytes.data() + 1, payload_len - 1});
}

}  // namespace gdl90_parser
