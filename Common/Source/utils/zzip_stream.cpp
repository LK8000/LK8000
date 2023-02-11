/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   zzip_file.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 21 novembre 2017, 20:31
 */

#include "zzip_stream.h"

#include <cstring>

#include <memory>
#include "utils/stl_utils.h"
#include "utils/charset_helper.h"
#include "utils/array_back_insert_iterator.h"
#include "Util/UTF8.hpp"

zzip_stream::zzip_stream(const TCHAR *szFile, const char *mode) {
  open(szFile, mode);
}

bool zzip_stream::open(const TCHAR *szFile, const char *mode) {
  assert(!(*this)); // open new file without closing previous ?!
  close(); // close previous opened file.

  _cs = charset::detect;

  if (!szFile) {
    return false; // invalid file path
  }
  if (!mode) {
    return false; // invalid open mode
  }

  _fp.reset(openzip(szFile, mode));
  if (!_fp) {
    // failed to open file
    return false;
  }

  if (traits_type::not_eof(underflow())) {
    // try to detect charset using utf8 BOM
    size_t read_size = std::distance(gptr(), egptr());
    if (read_size > 3) {
      if (_buffer[0] == (char)0xEF && _buffer[1] == (char)0xBB && _buffer[2] == (char)0xBF) {
        // file start with BOM switch charset to utf8
        _cs = charset::utf8;
        setg(_buffer+3, _buffer+3, egptr());
      }
    }

    if (_cs == charset::detect) {
      _cs = charset::unknown;
    }
  }
  return true;
}

int zzip_stream::underflow() {
  assert(_fp);
  assert(gptr() == egptr());

  // read next chunk
  zzip_ssize_t read_size = zzip_read(_fp.get(), _buffer, std::size(_buffer));
  if (read_size <= 0) {
    return traits_type::eof();
  } else {
    setg(_buffer, _buffer, _buffer + read_size);
  }
  return traits_type::to_int_type(*_buffer);
}

bool zzip_stream::read_line_raw(char *string, size_t size) {
  assert(_fp); // file  must be open before read
  if (!_fp) {
    return false;
  }

  auto out_it = array_back_inserter(string, size - 1); // size - 1 to let placeholder for '\0'

  auto ic = sbumpc();
  if (!traits_type::not_eof(ic)) {
    return false;
  }

  do {
    char c = traits_type::to_char_type(ic);
    if (c == '\r') {
      // Unix or Windows line ending
      if (sgetc() == '\n') {
        // Windows line ending
        sbumpc();
      }
      break;
    } else if(c == '\n') {
      break;
    } else {
        out_it = c;
    }
    ic = sbumpc();

  } while (traits_type::not_eof(ic));

  string[out_it.length()] = '\0'; // add leading '\0'

  if(out_it.overflowed()) {
    printf("read_line_raw overflow %u > %u\n", (unsigned)(out_it.length() + out_it.skipped()), (unsigned)(size - 1));
  }

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
    utf8String = ansi_to_utf8(string);

    if(utf8String.size() >= (size - 1)) {
      printf("read_line overflow %u > %u\n", (unsigned)utf8String.size(), (unsigned)(size - 1));
    }

    size_t str_len = std::min(utf8String.size(), size-1);
    (*std::copy_n(utf8String.data(), str_len, string)) = '\0';
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
    from_ansi(raw_string.begin(), string, size);
  } else {
    from_utf8(raw_string.begin(), string, size);
  }
  return true;
}
#endif
