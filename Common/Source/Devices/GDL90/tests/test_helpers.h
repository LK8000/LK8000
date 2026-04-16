/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GDL90/tests/test_helpers.h
 * Author: Bruno de Lacheisserie
 */

#pragma once

/* Shared test fixtures for GDL90 unit tests.
 * This header must only be included from test .cpp files (inside
 * #ifndef DOCTEST_CONFIG_DISABLE guards).
 */

#include "../gdl90.h"
#include "../frame_scanner.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// Encode a raw payload into a complete GDL90 frame (for testing)
// ─────────────────────────────────────────────────────────────────────────────

[[nodiscard]]
inline std::vector<uint8_t> encode_frame(uint8_t msg_id,
                                         gdl90_parser::detail::frame_data payload) {
  using namespace gdl90_parser;

  std::vector<uint8_t> content;
  content.reserve(1 + payload.size() + 2);
  content.push_back(msg_id);
  content.insert(content.end(), payload.begin(), payload.end());
  uint16_t crc = detail::crc16(content);
  content.push_back(static_cast<uint8_t>(crc & 0xFF));
  content.push_back(static_cast<uint8_t>(crc >> 8));

  // Byte-stuff and wrap in FLAG bytes
  std::vector<uint8_t> frame;
  frame.push_back(FLAG_BYTE);
  for (uint8_t b : content) {
    if (b == FLAG_BYTE || b == ESCAPE_BYTE) {
      frame.push_back(ESCAPE_BYTE);
      frame.push_back(b ^ ESCAPE_XOR);
    }
    else {
      frame.push_back(b);
    }
  }
  frame.push_back(FLAG_BYTE);
  return frame;
}

// ─────────────────────────────────────────────────────────────────────────────
// Build a traffic report frame from individual fields
// ─────────────────────────────────────────────────────────────────────────────

[[nodiscard]]
inline std::vector<uint8_t> make_traffic_frame(uint32_t icao = 0xABCDEF,
                                               double lat = 48.0,
                                               double lon = 2.5,
                                               int32_t alt_ft = 5000,
                                               uint16_t speed_kt = 120,
                                               const char* cs = "TESTFLT ") {
  std::array<uint8_t, 27> p{};

  p[0] = 0x00;  // address type ADSB_ICAO
  p[1] = (icao >> 16) & 0xFF;
  p[2] = (icao >> 8) & 0xFF;
  p[3] = icao & 0xFF;

  auto encode24 = [](double deg) -> uint32_t {
    return static_cast<uint32_t>(
               static_cast<int32_t>(deg * (1 << 23) / 180.0)) &
           0xFFFFFF;
  };
  uint32_t rlat = encode24(lat);
  p[4] = (rlat >> 16) & 0xFF;
  p[5] = (rlat >> 8) & 0xFF;
  p[6] = rlat & 0xFF;

  uint32_t rlon = encode24(lon);
  p[7] = (rlon >> 16) & 0xFF;
  p[8] = (rlon >> 8) & 0xFF;
  p[9] = rlon & 0xFF;

  uint16_t alt_raw = static_cast<uint16_t>((alt_ft + 1000) / 25);
  p[10] = (alt_raw >> 4) & 0xFF;
  p[11] = static_cast<uint8_t>((alt_raw & 0x0F) << 4) | 0x08;  // airborne
  p[12] = 0x08;  // NIC=0, NACp=8

  uint16_t speed_raw = std::min<uint16_t>(speed_kt, 0xFFE);  // cap at 4094
  p[13] = (speed_raw >> 4) & 0xFF;
  p[14] = static_cast<uint8_t>((speed_raw & 0x0F) << 4) | 0x08;  // invalid vertical speed
  p[15] = 0x00;

  for (int i = 0; i < 8; ++i) {
    p[18 + i] = static_cast<uint8_t>(cs[i]);
  }

  return encode_frame(gdl90_parser::MSG_TRAFFIC, p);
}
