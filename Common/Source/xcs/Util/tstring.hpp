#ifndef TSTRING_HPP
#define TSTRING_HPP

#include <math.h>
#include <string>

#ifdef _UNICODE
#include <tchar.h>
typedef std::wstring tstring;

tstring to_tstring( const char* sz);

tstring utf8_to_tstring(const char* sz);

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

#endif

tstring &
trim_inplace(tstring &s);

#endif
