/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GDL90/tests/foreflight.cpp
 * Author: Bruno de Lacheisserie
 */

#include "options.h"
#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#include "test_helpers.h"

#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
// ForeFlight ID (MSG_FOREFLIGHT_ID  0x65)
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("foreflight ID — decodes serial and short name") {
  std::array<uint8_t, 38> payload{};
  payload[0] = 0x00;  // ID subtype
  payload[1] = 0x01;  // version
  payload[2] = 0x11;
  payload[3] = 0x22;
  payload[4] = 0x33;
  payload[5] = 0x44;
  payload[6] = 0x55;
  payload[7] = 0x66;
  payload[8] = 0x77;
  payload[9] = 0x88;
  std::memcpy(&payload[10], "Stratux ", 8);

  auto frame = encode_frame(gdl90_parser::MSG_FOREFLIGHT_ID, payload);
  auto r = gdl90_parser::decode_frame(frame);
  auto* f = std::get_if<gdl90_parser::foreflight_id>(&r);
  REQUIRE(f != nullptr);
  CHECK_EQ(f->sub_id, 0x00);
  CHECK_EQ(f->device_serial, 0x1122334455667788ULL);
  CHECK_EQ(std::strncmp(f->device_name, "Stratux", 7), 0);
}

#endif  // DOCTEST_CONFIG_DISABLE
