/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   zzip_file.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 21 novembre 2017, 20:31
 */

#include "zzip_stream.h"

#include <string.h>

#include "stl_utils.h"
#include "make_unique.h"
#include "stringext.h"

#include "Util/UTF8.hpp"

#include "Poco/Latin1Encoding.h"
#include "Poco/UTF8Encoding.h"
#include "Poco/TextConverter.h"


bool zzip_stream::read_line_raw(char* string, size_t size) {
  assert(*this);
  if (!(*this)) {
    return false;
  }

  if (_begin == nullptr) {
    _end = _begin = _buffer;
  }

  char* out_it = string;
  const char* out_end = string + size;
  bool found = false;

  while (!found && out_it != out_end) {
    if (_begin == _end) {
      zzip_ssize_t read_size = zzip_read(*this, _buffer, array_size(_buffer));
      if (read_size <= 0) {
        return false;
      }
      _begin = _buffer;
      _end = _buffer + read_size;

      // try to detect charset using utf8 BOM
      if (read_size > 3 && _cs == charset::detect) {
        if (_buffer[0] == (char) 0xEF && _buffer[1] == (char) 0xBB && _buffer[2] == (char) 0xBF) {
          // string start with BOM switch charset to utf8
          _cs = charset::utf8;
          _begin += 3; // skip BOM
        }
      } 
      if (_cs == charset::detect) {
        _cs = charset::unknown;
      }
    }

    while (_begin != _end) {
      if (*(_begin) == '\n') {
        // Mac line ending
        ++_begin;
        found = true;
        break;
      }

      if (*(_begin) == '\r') {
        // Unix or Windows line ending
        ++_begin;
        if (_begin != _end && *(_begin) == '\n') {
          // Windows line ending
          ++_begin;
        }
        found = true;
        break;
      } else if (*(_begin) == '\n') {
        // Mac line ending
        ++_begin;
      }
      *(out_it++) = *(_begin++);
    }
  }

  assert(out_it != out_end); // output string too small.
  if (out_it != out_end) {
    *(out_it) = '\0';
  } else {
    out_it[size - 1] = '\0';
  }

  if (_cs == charset::unknown && !ValidateUTF8(string)) {
    _cs = charset::latin1;
  }

  return true;
}

bool zzip_stream::read_line(char* string, size_t size) {

  if(!read_line_raw(string, size)) {
    return false;
  }

  if (_cs == charset::latin1) {
    // from Latin1 (ISO-8859-1) To Utf8

    utf8String.clear();
    
    Poco::Latin1Encoding Latin1Encoding;
    Poco::UTF8Encoding utf8Encoding;

    Poco::TextConverter converter(Latin1Encoding, utf8Encoding);
    converter.convert(string, strlen(string), utf8String);
    assert(utf8String.size() <= size); // out string to small
    strncpy(string, utf8String.c_str(), size - 1);
    string[size - 1] = '\0';
  }
  return true;
}

#ifdef UNICODE

bool zzip_stream::read_line(wchar_t* string, size_t size) {

  raw_string.GrowDiscard(size);
  if (!read_line_raw(raw_string.begin(), size)) {
    return false;
  }

  if (_cs == charset::latin1) {    
    // from Latin1 (ANSI) To UNICODE
    int nChars = MultiByteToWideChar(CP_ACP, 0, raw_string.begin(), -1, string, size);
    if (!nChars) {
      // out string to small, 
      assert(false);
      return false;
    }
  } else {
    utf2TCHAR(raw_string.begin(), string, size);
  }
  return true;
}
#endif
