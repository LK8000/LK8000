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

#include <cstring>

#include "utils/make_unique.h"
#include "utils/stl_utils.h"
#include "utils/stringext.h"

#include "Util/UTF8.hpp"

#include "Poco/Latin1Encoding.h"
#include "Poco/TextConverter.h"
#include "Poco/UTF8Encoding.h"

zzip_stream::zzip_stream(const TCHAR *szFile, const char *mode) {
  open(szFile, mode);
}

bool zzip_stream::open(const TCHAR *szFile, const char *mode) {

  close(); // close previous opened file.

  _end = _current_char = nullptr;
  _end_of_file = false;
  _cs = charset::detect;

  if (!szFile) {
    return false; // invalid file path
  }
  if (!mode) {
    return false; // invalid open mode
  }

  _fp = openzip(szFile, mode);
  if (!_fp) {
    // failed to open file
    return false;
  }

  read_buffer();

  if (!_end_of_file) {
    // try to detect charset using utf8 BOM
    size_t read_size = std::distance(_current_char, _end);
    if (read_size > 3) {
      if (_buffer[0] == (char)0xEF && _buffer[1] == (char)0xBB &&
          _buffer[2] == (char)0xBF) {
        // string start with BOM switch charset to utf8
        _cs = charset::utf8;
        _current_char += 3; // skip BOM
        if (_current_char == _end) {
          read_buffer(); // that can happen if we have empty utf8 file with BOM
        }
      }
    }
    if (_cs == charset::detect) {
      _cs = charset::unknown;
    }
  }
  return true;
}

void zzip_stream::read_buffer() {
  assert(_fp);
  assert(_current_char == _end);

  _current_char = _end = _buffer; // reset buffer iterator

  // read next chunk
  zzip_ssize_t read_size = zzip_read(_fp, _buffer, array_size(_buffer));
  if (read_size <= 0) {
    _end_of_file = true; // end of file reached
  } else {
    _end = _buffer + read_size;
  }
}

char zzip_stream::read_char() {
  assert(_current_char != _end);
  char c = *(_current_char++);
  if (_current_char == _end) {
    read_buffer();
  }
  return c;
}

bool zzip_stream::read_line_raw(char *string, size_t size) {
  assert(_fp); // file  must be open before read
  if (!_fp) {
    return false;
  }

  if (_end_of_file) {
    return false;
  }

  char *out_it = string;
  const char *out_end = string + size;
  bool end_of_line = false; // true when line ending or end of file is found

  while (!end_of_line && !_end_of_file) {

    char c = read_char();
    if (c == '\r') {
      // Unix or Windows line ending
      if (!_end_of_file && (*_current_char) == '\n') {
        // Windows line ending
        read_char();
      }
      end_of_line = true;
    } else if (c == '\n') {
      // Mac line ending
      end_of_line = true;
    } else {
      *(out_it++) = c;

      if (out_it == out_end) {
        assert(false); // output string to small
        return false;
      }
    }
  }

  *(out_it) = '\0';

  if (_cs == charset::unknown && !ValidateUTF8(string)) {
    _cs = charset::latin1;
  }

  return true;
}

bool zzip_stream::read_line(char *string, size_t size) {

  if (!read_line_raw(string, size)) {
    return false;
  }

  if (_cs == charset::latin1) {
    // from Latin1 (ISO-8859-1) To Utf8

    utf8String.clear();

    Poco::Latin1Encoding Latin1Encoding;
    Poco::UTF8Encoding utf8Encoding;

    Poco::TextConverter converter(Latin1Encoding, utf8Encoding);
    converter.convert(string, strlen(string), utf8String);
    if (utf8String.size() < (size - 1)) {
      // copy all charaters and append zero terminator.
      (*std::copy(utf8String.begin(), utf8String.end(), string)) = '\0';
    } else {
      assert(false); // output string to small
      return false;
    }
  }
  return true;
}

#ifdef UNICODE

bool zzip_stream::read_line(wchar_t *string, size_t size) {

  raw_string.GrowDiscard(size);
  if (!read_line_raw(raw_string.begin(), size)) {
    return false;
  }

  if (_cs == charset::latin1) {
    // from Latin1 (ANSI) To UNICODE
    int nChars = MultiByteToWideChar(CP_ACP, 0, raw_string.begin(), -1, string, size);
    if (!nChars) {
      // output string to small,
      assert(false);
      return false;
    }
  } else {
    utf2unicode(raw_string.begin(), string, size);
  }
  return true;
}
#endif
