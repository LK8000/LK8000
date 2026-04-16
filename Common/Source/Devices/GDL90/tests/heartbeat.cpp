/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GDL90/tests/heartbeat.cpp
 * Author: Bruno de Lacheisserie
 */

#include "options.h"
#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#include "test_helpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Heartbeat (MSG_HEARTBEAT  0x00)
// ─────────────────────────────────────────────────────────────────────────────

// Known payload; CRC precomputed with encode_frame
static const uint8_t kHeartbeat[] = {
    0x7E,
    0x00,        // msg id
    0x81, 0x41,  // status1=0x81 (GPS valid), status2=0x41
    0xDB, 0xD0,  // timestamp
    0x00, 0x00,  // message counts
    0xB1, 0x83,  // CRC little-endian
    0x7E};

TEST_CASE("heartbeat — valid decode") {
  auto r = gdl90_parser::decode_frame(kHeartbeat);

  auto* h = std::get_if<gdl90_parser::heartbeat>(&r);
  REQUIRE(h != nullptr);

  CHECK_EQ(h->status_byte1, 0x81);
  CHECK(h->gps_pos_valid());
  CHECK_FALSE(h->maintenance_req());
}

TEST_CASE("heartbeat — corrupted CRC") {
  std::vector<uint8_t> bad(std::begin(kHeartbeat), std::end(kHeartbeat));
  bad[bad.size() - 2] ^= 0xFF;
  CHECK_THROWS_AS(auto r = gdl90_parser::decode_frame(bad), gdl90_parser::bad_crc);
}

TEST_CASE("heartbeat — frame too short") {
  std::vector<uint8_t> tiny{0x7E, 0x00, 0x7E};
  CHECK_THROWS_AS(auto r = gdl90_parser::decode_frame(tiny), gdl90_parser::too_short);
}

#endif  // DOCTEST_CONFIG_DISABLE
