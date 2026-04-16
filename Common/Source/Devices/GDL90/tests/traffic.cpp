/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GDL90/tests/traffic.cpp
 * Author: Bruno de Lacheisserie
 */

#include "options.h"
#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#include "test_helpers.h"

#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
// Traffic / Ownship report (MSG_TRAFFIC 0x14, MSG_OWNSHIP 0x0A)
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("traffic — main fields") {
  auto frame = make_traffic_frame();
  auto r = gdl90_parser::decode_frame(frame);
  auto t = std::get_if<gdl90_parser::traffic_report>(&r);
  REQUIRE(t != nullptr);

  SUBCASE("ICAO address") {
    CHECK_EQ(t->icao_address, 0xABCDEFu);
    CHECK_EQ(t->address_type, gdl90_parser::AddressType::ADSB_ICAO);
  }
  SUBCASE("position") {
    CHECK(std::abs(t->latitude - 48.0) < 0.01);
    CHECK(std::abs(t->longitude - 2.5) < 0.01);
  }
  SUBCASE("altitude and flight state") {
    CHECK_EQ(t->pressure_alt, 5000);
    CHECK(t->air_ground);  // airborne
  }
  SUBCASE("speed") {
    CHECK_EQ(t->horiz_velocity, 120);
  }
  SUBCASE("callsign") {
    CHECK(t->has_callsign);
    CHECK_EQ(std::strncmp(t->callsign, "TESTFLT", 7), 0);
  }
}

TEST_CASE("traffic — negative latitude (southern hemisphere)") {
  auto frame = make_traffic_frame(0x123456, -33.87, 151.21);  // Sydney
  auto r = gdl90_parser::decode_frame(frame);
  auto t = std::get_if<gdl90_parser::traffic_report>(&r);
  REQUIRE(t != nullptr);
  CHECK(t->latitude < 0.0);
  CHECK(t->longitude > 0.0);
}

#endif  // DOCTEST_CONFIG_DISABLE
