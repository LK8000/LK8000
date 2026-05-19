/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id$
 */

#ifndef _SETTINGS_READ_H_
#define _SETTINGS_READ_H_

#include <concepts>
#include <type_traits>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <string_view>
#include "utils/strcpy.h"

namespace settings {
namespace detail {

template <typename T>
concept unsigned_enum = std::is_enum_v<T> && std::is_unsigned_v<std::underlying_type_t<T>>;

template <typename T>
concept signed_enum = std::is_enum_v<T> && std::is_signed_v<std::underlying_type_t<T>>;

/**
 * any signed numeric type
 */
template <typename T>
concept signed_number =
    std::floating_point<T> || std::signed_integral<T> || signed_enum<T>;

/**
 * any unsigned numeric type
 */
template <typename T>
concept unsigned_number = std::unsigned_integral<T> || unsigned_enum<T>;

/**
 * any type that can be assigned from a C-style string
 */
template <typename T>
concept cstring_assignable =
    std::assignable_from<T&, const char*> && !std::is_pointer_v<T>;

/**
 * read 'signed number' type
 */
template <signed_number T>
void read_value(const char* curvalue, T& lookupvalue) {
  lookupvalue = static_cast<T>(std::strtol(curvalue, nullptr, 10));
}

/**
 * read 'unsigned number' type
 */
template <unsigned_number T>
void read_value(const char* curvalue, T& lookupvalue) {
  lookupvalue = static_cast<T>(std::strtoul(curvalue, nullptr, 10));
}

/**
 * read utf8 string
 */
template <size_t size>
void read_value(const char* curvalue, char (&lookupvalue)[size]) {
  // saved pref is longer than variable used to store value.
  assert(strlen(curvalue) < size);
  lk::strcpy(lookupvalue, curvalue);
}

/**
 * read unicode string
 */
template <size_t size>
void read_value(const char* curvalue, wchar_t (&lookupvalue)[size]) {
  from_utf8(curvalue, lookupvalue, size);
}

/**
 * generic : all type having operator=(const char*)
 */
template <cstring_assignable T>
void read_value(const char* curvalue, T& lookupvalue) {
  lookupvalue = curvalue;
}

}  // namespace detail

/**
 * default read specialisation
 * @return true if 'curname' is equal to 'lookupname'
 */
template <typename T>
bool read(std::string_view curname, const char* curvalue, const char* lookupname,
          T& lookupvalue) {
  if (curname != lookupname) {
    return false;
  }
  detail::read_value<T>(curvalue, lookupvalue);
  return true;
}

/**
 * read specialisation for 'T[size]' pref (aka TCHAR[size]
 * @return true if 'curname' is equal to 'lookupname'
 */
template <typename T, size_t size>
bool read(std::string_view curname, const char* curvalue, const char* lookupname,
          T (&lookupvalue)[size]) {
  if (curname != lookupname) {
    return false;
  }
  detail::read_value<size>(curvalue, lookupvalue);
  return true;
}

}  // namespace settings
#endif
