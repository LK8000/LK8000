/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GDL90/tests/stratux.cpp
 * Author: Bruno de Lacheisserie
 */

#include "options.h"
#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#include "test_helpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Stratux status (MSG_STRATUX_STATUS  0x53)
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("stratux status — decodes metrics") {
  std::array<uint8_t, 28> payload{};
  payload[0] = 'X';
  payload[1] = 1;
  payload[2] = 1;
  payload[3] = 2;
  payload[4] = 5;
  payload[5] = 1;
  payload[6] = 9;
  payload[11] = 0x01;
  payload[12] = 0xF4;
  payload[14] = 0x05;
  payload[15] = 7;
  payload[16] = 9;
  payload[17] = 0x00;
  payload[18] = 0x12;
  payload[19] = 0x00;
  payload[20] = 0x34;
  payload[21] = 0x00;
  payload[22] = 0x56;
  payload[23] = 0x00;
  payload[24] = 0x78;
  payload[25] = 0x01;
  payload[26] = 0x90;
  payload[27] = 4;

  auto frame = encode_frame(gdl90_parser::MSG_STRATUX_STATUS, payload);
  auto r = gdl90_parser::decode_frame(frame);
  auto* s = std::get_if<gdl90_parser::stratux_status>(&r);
  REQUIRE(s != nullptr);
  CHECK_EQ(s->protocol_version, 1);
  CHECK_EQ(s->message_version, 1);
  CHECK_EQ(s->version_major, 2);
  CHECK_EQ(s->version_minor, 5);
  CHECK_EQ(s->version_build_type, 1);
  CHECK_EQ(s->version_build, 9);
  CHECK_EQ(s->gps_satellites_locked, 7);
  CHECK_EQ(s->gps_satellites_tracked, 9);
  CHECK_EQ(s->uat_messages_per_minute, 0x0056);
  CHECK_EQ(s->es_messages_per_minute, 0x0078);
  CHECK(s->cpu_temp_c == doctest::Approx(40.0));
  CHECK_EQ(s->num_towers, 4);
}

#endif  // DOCTEST_CONFIG_DISABLE
