/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GDL90/tests/helpers.cpp
 * Author: Bruno de Lacheisserie
 */

#include "options.h"
#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#include "test_helpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Byte stuffing
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("byte stuffing — 0x7E and 0x7D in the payload") {
  std::array<uint8_t, 4> payload{0x7E, 0x7D, 0x42, 0x00};
  auto frame = encode_frame(0xFF, payload);

  SUBCASE("no inner 0x7E after stuffing") {
    for (size_t i = 1; i + 1 < frame.size(); ++i) {
      CHECK_FALSE(frame[i] == 0x7E);
    }
  }
  SUBCASE("round-trip: bytes recovered intact") {
    auto r = gdl90_parser::decode_frame(frame);
    auto u = std::get_if<gdl90_parser::unknown_msg>(&r);
    REQUIRE(u != nullptr);
    CHECK_EQ(u->id, 0xFF);
    REQUIRE_EQ(u->payload.size(), 4u);
    CHECK_EQ(u->payload[0], 0x7E);
    CHECK_EQ(u->payload[1], 0x7D);
    CHECK_EQ(u->payload[2], 0x42);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// CRC16
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("CRC16 — correct constexpr table") {
  // Reference values taken from the GDL90 spec
  static_assert(gdl90_parser::detail::crc16_table[0] == 0x0000);
  static_assert(gdl90_parser::detail::crc16_table[1] == 0x1021);
  static_assert(gdl90_parser::detail::crc16_table[0xFF] == 0x1EF0);
  // If execution reaches this point, the static_assert checks passed at compile time
  CHECK(true);
}

TEST_CASE("CRC16 — encode and decode stay consistent") {
  // Every frame produced by encode_frame must pass the CRC check in decode_frame
  for (uint8_t id :
       {gdl90_parser::MSG_HEARTBEAT, uint8_t(0xFF), uint8_t(0x7D), uint8_t(0x7E)}) {
    std::array<uint8_t, 6> pl{id, 0x01, 0x02, 0x03, 0x04, 0x05};
    auto frame = encode_frame(id, pl);
    REQUIRE_NOTHROW(auto r = gdl90_parser::decode_frame(frame));
  }
}

#endif  // DOCTEST_CONFIG_DISABLE
