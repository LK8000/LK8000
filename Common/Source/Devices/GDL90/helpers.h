/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GDL90/helpers.h
 * Author: Bruno de Lacheisserie
 */

#pragma once

#include "types.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace gdl90_parser {
namespace detail {

using frame_data = std::span<const uint8_t>;

[[nodiscard]]
constexpr uint16_t crc16_table_entry(uint8_t byte) noexcept {
  uint16_t crc = static_cast<uint16_t>(byte) << 8;
  for (int i = 0; i < 8; ++i) {
    crc = (crc & 0x8000) ? static_cast<uint16_t>((crc << 1) ^ 0x1021)
                         : static_cast<uint16_t>(crc << 1);
  }
  return crc;
}

inline constexpr auto crc16_table = []() constexpr noexcept {
  std::array<uint16_t, 256> table{};
  for (int i = 0; i < 256; ++i) {
    table[i] = crc16_table_entry(static_cast<uint8_t>(i));
  }
  return table;
}();

[[nodiscard]]
inline uint16_t crc16(frame_data data) noexcept {
  uint16_t crc = 0;
  for (uint8_t byte : data) {
    crc = crc16_table[(crc >> 8) & 0xFF] ^
          static_cast<uint16_t>((crc << 8) ^ byte);
  }
  return crc;
}

[[nodiscard]]
inline std::vector<uint8_t> unstuff(frame_data raw) {
  std::vector<uint8_t> out;
  out.reserve(raw.size());
  for (size_t i = 0; i < raw.size(); ++i) {
    if (raw[i] == ESCAPE_BYTE) {
      if (++i >= raw.size()) {
        throw invalid_framing();
      }
      out.push_back(raw[i] ^ ESCAPE_XOR);
    }
    else {
      out.push_back(raw[i]);
    }
  }
  return out;
}

[[nodiscard]]
constexpr uint16_t read_u16_le(frame_data data, size_t offset) noexcept {
  return static_cast<uint16_t>(data[offset]) |
         (static_cast<uint16_t>(data[offset + 1]) << 8);
}

[[nodiscard]]
constexpr uint16_t read_u16_be(frame_data data, size_t offset) noexcept {
  return (static_cast<uint16_t>(data[offset]) << 8) |
         static_cast<uint16_t>(data[offset + 1]);
}

[[nodiscard]]
constexpr int16_t read_s16_be(frame_data data, size_t offset) noexcept {
  return static_cast<int16_t>(read_u16_be(data, offset));
}

[[nodiscard]]
constexpr uint32_t read_u24_be(frame_data data, size_t offset) noexcept {
  return (static_cast<uint32_t>(data[offset]) << 16) |
         (static_cast<uint32_t>(data[offset + 1]) << 8) |
         static_cast<uint32_t>(data[offset + 2]);
}

[[nodiscard]]
inline int32_t decode_lat_lon(uint32_t raw24) noexcept {
  if (raw24 & 0x800000) {
    return static_cast<int32_t>(raw24 | 0xFF000000u);
  }
  return static_cast<int32_t>(raw24);
}

[[nodiscard]]
inline int32_t decode_altitude(uint16_t raw12) noexcept {
  if (raw12 == 0xFFF) {
    return -1000;  // invalid sentinel
  }
  return static_cast<int32_t>(raw12) * 25 - 1000;
}

}  // namespace detail
}  // namespace gdl90_parser