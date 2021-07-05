#include "tstring.hpp"
#include <tchar.h>
#include "ConvertString.hpp"

#define WHITESPACE _T(" \r\t")

tstring &
trim_inplace(tstring &s)
{
  tstring::size_type n;

  n = s.find_first_not_of(WHITESPACE);
  if (n != tstring::npos)
    s.erase(0, n);

  n = s.find_last_not_of(WHITESPACE);
  if (n != tstring::npos)
    s.erase(n + 1, s.length());

  return s;
}

#ifdef _UNICODE
tstring to_tstring(const char* sz) {
  tstring tsz;
  const ACPToWideConverter converter(sz);
  if(converter.IsValid()) {
    tsz = converter;
  }
  return tsz;
//  return static_cast<TCHAR*>(converter);
}

tstring utf8_to_tstring(const char* sz) {
  tstring tsz;
  const UTF8ToWideConverter converter(sz);
  if(converter.IsValid()) {
    tsz = converter;
  }
  return tsz;
//  return static_cast<TCHAR*>(converter);
}

std::string to_utf8(const wchar_t* tsz) {
  std::string sz;
  const WideToUTF8Converter converter(tsz);
  if(converter.IsValid()) {
    sz = converter;
  }
  return sz;
}

#endif
