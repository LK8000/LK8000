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

// Trim whitespace (returns trimmed view)
template<typename CharT>
std::basic_string_view<CharT>&
trim_inplace(std::basic_string_view<CharT>& sv) {
  while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.front()))) {
    sv.remove_prefix(1);
  }
  while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.back()))) {
    sv.remove_suffix(1);
  }
  return sv;
}

template <typename CharT>
void replace_all(std::basic_string<CharT>& string, const std::basic_string_view<CharT>& old_string,
                 const std::basic_string_view<CharT>& new_string) {
  size_t start_pos = 0;
  while ((start_pos = string.find(old_string, start_pos)) != std::string::npos) {
    string.replace(start_pos, old_string.length(), new_string);
    start_pos += new_string.length();  // Handles case where 'to' is a substring of 'from'
  }
}

} // namespace

std::string& trim_inplace(std::string &s) {
  return trim_inplace<std::string::value_type>(s);
}

std::wstring& trim_inplace(std::wstring &s) {
  return trim_inplace<std::wstring::value_type>(s);
}

std::string_view& trim_inplace(std::string_view &s) {
  return trim_inplace<std::string::value_type>(s);
}

std::wstring_view& trim_inplace(std::wstring_view &s) {
  return trim_inplace<std::wstring::value_type>(s);
}


void replace_all(std::string& string, const std::string_view& old_string, const std::string_view& new_string) {
  replace_all<std::string::value_type>(string, old_string, new_string);
}

void replace_all(std::wstring& string, const std::wstring_view& old_string, const std::wstring_view& new_string) {
  replace_all<std::wstring::value_type>(string, old_string, new_string);
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
