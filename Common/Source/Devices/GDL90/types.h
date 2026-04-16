/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GDL90/types.h
 * Author: Bruno de Lacheisserie
 */

#pragma once

#include <cstdint>
#include <exception>

namespace gdl90_parser {

// ─────────────────────────────────────────────────────────────────────────────
// Framing constants
// ─────────────────────────────────────────────────────────────────────────────

inline constexpr uint8_t FLAG_BYTE   = 0x7E;
inline constexpr uint8_t ESCAPE_BYTE = 0x7D;
inline constexpr uint8_t ESCAPE_XOR  = 0x20;

class need_more_data : public std::exception {
 public:
  const char* what() const noexcept override {
    return "gdl90: need more data to complete frame";
  }
};

class bad_crc : public std::exception {
 public:
  const char* what() const noexcept override {
    return "gdl90: bad CRC";
  }
};

class too_short : public std::exception {
 public:
  const char* what() const noexcept override {
    return "gdl90: frame too short";
  }
};

class invalid_framing : public std::exception {
 public:
  const char* what() const noexcept override {
    return "gdl90: invalid framing";
  }
};

// ─────────────────────────────────────────────────────────────────────────────
// Compile-time msg_id allowlist — primary template (undefined by default).
// Each decoder file provides a specialization for its own message type.
// ─────────────────────────────────────────────────────────────────────────────

template<typename T>
struct msg_id_for;

}  // namespace gdl90_parser
