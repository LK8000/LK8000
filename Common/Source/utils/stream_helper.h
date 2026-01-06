/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   stream_helper.h
 * Author: Bruno de Lacheisserie
 */
#pragma once
#include <istream>
#include "utils/charset_helper.h"

/**
 * @file stream_helper.h
 * @brief Utilities for robust line-oriented input handling supporting CR, LF, and CRLF.
 *
 * Provides `lk::getline` that reads a line treating '\n', '\r', and '\r\n' as terminators,
 * and `lk::getline_unknown_charset` that converts from an unknown charset to the target
 * string type using `from_unknown_charset`.
 */

/** Utilities for stream reading in the `lk` namespace. */
namespace lk {

/**
 * Read a single line from a byte-oriented input stream accepting any common line ending.
 *
 * This function treats LF ('\n'), CRLF ("\r\n") and CR ('\r') as line terminators.
 * The line terminator is not included in `out`.
 *
 * @param is  Input stream to read from. The stream position will be advanced.
 * @param out Output string where the line content (without terminator) is stored.
 * @return `true` if a line was read; `false` on EOF with no characters read.
 */
inline bool getline(std::istream& is, std::string& out) {
  out.clear();

  using int_t = typename std::istream::int_type;
  while (true) {
    int_t c = is.get();
    if (c == std::char_traits<char>::eof()) {
      return !out.empty();
    }
    char ch = static_cast<char>(c);
    if (ch == '\n') {
      return true;
    }
    if (ch == '\r') {
      // consume optional LF after CR (handle CRLF)
      if (is.peek() == '\n') {
        is.get();
      }
      return true;
    }
    out.push_back(ch);
  }
}

template <typename _CharT, typename _Traits, typename _Alloc>
inline bool getline_unknown_charset(std::istream& is,
          std::basic_string<_CharT, _Traits, _Alloc>& out) {
  std::string unknown_out;

  if (lk::getline(is, unknown_out)) {
    out = from_unknown_charset(unknown_out.c_str());
    return true;
  }
  return false;
}

}  // namespace lk