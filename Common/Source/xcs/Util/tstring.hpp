#ifndef TSTRING_HPP
#define TSTRING_HPP

#include <math.h>
#include <string>
#include <algorithm>
#include "Util/CharUtil.hpp"

#ifdef _UNICODE
#include <tchar.h>
typedef std::wstring tstring;

tstring to_tstring( const char* sz);

tstring utf8_to_tstring(const char* sz);

std::string to_utf8(const wchar_t* sz);

#else
typedef std::string tstring;

inline
tstring to_tstring( const char* sz) {
  return sz;
}

inline
tstring utf8_to_tstring(const char* sz) {
  return sz;
}

inline
std::string to_utf8(const char* sz) {
  return sz;
}

#endif

tstring &
trim_inplace(tstring &s);


/**
 * Convert ASCII character (0x00..0x7f) to lower case.
 * it ignores the system locale.
 */
inline
tstring to_lower_ascii(const tstring& source) {
    tstring lower_text;
    std::transform(source.begin(), source.end(), std::back_inserter(lower_text), ToLowerASCII);
    return lower_text;
}

#endif
