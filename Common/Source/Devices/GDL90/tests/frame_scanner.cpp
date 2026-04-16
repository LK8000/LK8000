/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GDL90/tests/frame_scanner.cpp
 * Author: Bruno de Lacheisserie
 */

#include "options.h"
#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#include "test_helpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// frame_scanner
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("frame_scanner — two concatenated frames with noise") {
  auto f = make_traffic_frame();

  std::vector<uint8_t> stream;
  stream.push_back(0xFF);  // noise before
  stream.insert(stream.end(), f.begin(), f.end());
  stream.insert(stream.end(), f.begin(), f.end());
  stream.push_back(0xAB);  // noise after

  gdl90_parser::frame_scanner sc;
  int count = 0;
  sc.push(stream, [&](auto) {
    ++count;
  });

  CHECK_EQ(count, 2);
}

TEST_CASE("frame_scanner — feeding in small chunks") {
  auto frame = make_traffic_frame();

  gdl90_parser::frame_scanner sc;
  int count = 0;
  for (size_t i = 0; i < frame.size(); ++i) {
    sc.push({&frame[i], 1}, [&](auto) {
      ++count;
    });
  }

  CHECK_EQ(count, 1);
}

TEST_CASE("frame_scanner — reset between two sessions") {
  auto f = make_traffic_frame();
  gdl90_parser::frame_scanner sc;
  int count = 0;

  sc.push(f, [&](auto) {
    ++count;
  });
  CHECK_EQ(count, 1);

  sc.reset();
  sc.push(f, [&](auto) {
    ++count;
  });
  CHECK_EQ(count, 2);
}

TEST_CASE("frame_scanner — ignores undersized fragments") {
  const std::array<uint8_t, 6> stream{0x7E, 0x53, 0x7E, 0x7E, 0x00, 0x7E};

  gdl90_parser::frame_scanner sc;
  int count = 0;
  sc.push(stream, [&](auto) {
    ++count;
  });

  CHECK_EQ(count, 0);
}

#endif  // DOCTEST_CONFIG_DISABLE
