/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   charset_helper.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 02 February 2022
 */

#ifndef _utils_charset_helper_h_
#define _utils_charset_helper_h_

#include "Util/UTF8.hpp"
#include "Util/tstring.hpp"
#include "utils/stringext.h"
#include <cstring>

/**
 * To Convert UTF8 string to std::basic_string
 */
template<typename CharT>
inline
std::basic_string<CharT> utf8_to_string(const char* utf8) {
  std::basic_string<CharT> out;
  out.resize(from_utf8(utf8, (CharT*)nullptr, 0) + 1);
  size_t size = from_utf8(utf8, out.data(), out.size());
  out.resize(size);
  return out;
}

/**
 * To Convert ANSI string to std::basic_string
 */
template<typename CharT>
inline
std::basic_string<CharT> ansi_to_string(const char* ansi) {
  std::basic_string<CharT> out;
  out.resize(from_ansi(ansi, (CharT*)nullptr, 0) + 1);
  size_t size = from_ansi(ansi, out.data(), out.size());
  out.resize(size);
  return out;
}

/**
 * To detect charset from input string and convert to std::basic_string
 */
template<typename CharT>
inline
std::basic_string<CharT> from_unknown_charset(const char* string) {
  if (ValidateUTF8(string)) {
    return utf8_to_string<CharT>(string);
  }
  else {
    return ansi_to_string<CharT>(string);
  }
}

/**
 * To detect charset from input string and convert to "CharT* out"
 *
 * @string: input string
 * @out: target string
 * @size: size of target string including trailing '\0'
 */
template<typename CharT>
inline
size_t from_unknown_charset(const char* string, CharT* out, size_t size) {
  if (ValidateUTF8(string)) {
    return from_utf8(string, out, size);
  }
  else {
    return from_ansi(string, out, size);
  }
}

/**
 * To detect charset from input string and convert to "CharT out[size]"
 */
template<typename CharT, size_t size>
inline
size_t from_unknown_charset(const char* string, CharT (&out)[size]) {
  return from_unknown_charset(string, out, size);
}


/**
 * Helper to do implicit charset transcoding
 */
class from_unknown_charset_t {
public:
  explicit from_unknown_charset_t(const char* string) : input_string(string) {}

  operator std::string() const {
    return from_unknown_charset<std::string::value_type>(input_string);
  }

  operator std::wstring() const {
    return from_unknown_charset<std::wstring::value_type>(input_string);
  }

private:
  const char* input_string;
};

/**
 * To detect charset from input string and convert to std::basic_string
 *  
 * same as "std::basic_string<CharT> from_unknown_charset(const char*)
 * but with automatic template parameter substitution.
 */
inline
from_unknown_charset_t from_unknown_charset(const char* string) {
  return  from_unknown_charset_t(string);
}

/**
 * To convert UTF8 string to tstring (aka std::string<TCHAR>)
 */
inline
tstring from_utf8(const char *utf8) {
  return utf8_to_string<tstring::value_type>(utf8);
}

/**
 * To convert ANSI string to tstring (aka std::string<TCHAR>)
 */
inline
tstring from_ansi(const char *ansi) {
  return ansi_to_string<tstring::value_type>(ansi);
}

/**
 * To convert ANSI string to std::string
 */
inline
std::string ansi_to_utf8(const char *ansi) {
  return ansi_to_string<std::string::value_type>(ansi);
}

#endif // _utils_charset_helper_h_
