#include "tstring.hpp"
#include <tchar.h>
#include "ConvertString.hpp"

namespace {

struct white_space {
  operator const wchar_t*() const { return L" \r\t\n\v\f"; }
  operator const char*() const { return " \r\t\n\v\f"; }
};

template <typename CharT>
std::basic_string<CharT>&
trim_inplace(std::basic_string<CharT>& s) {
  s.erase(0, s.find_first_not_of(white_space()));
  s.erase(s.find_last_not_of(white_space()) + 1);
  return s;
}

} // namespace

std::string& trim_inplace(std::string &s) {
  return trim_inplace<std::string::value_type>(s);
}

std::wstring& trim_inplace(std::wstring &s) {
  return trim_inplace<std::wstring::value_type>(s);
}

#ifdef _UNICODE
std::wstring to_wstring(const char* sz) {
  std::wstring tsz;
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
