#ifndef TSTRING_HPP
#define TSTRING_HPP

#include <math.h>
#include <string>
#include <algorithm>
#include <iterator>
#include "Util/CharUtil.hpp"


inline
std::string to_string(const char* sz) {
  return sz;
}

inline
std::string to_string(std::string str) {
  return str;
}

#ifdef _UNICODE
#include <tchar.h>
typedef std::wstring tstring;
typedef std::wstring_view tstring_view;

#ifdef _GLIBCXX_HAVE_BROKEN_VSWPRINTF
// workarround for mingw32ce
template<typename T, size_t size> 
inline std::wstring format_string(const wchar_t* fmt, int v) {
  TCHAR out[size] = {};
  swprintf(out, fmt, v);
  return out;
}

/**
 * integral type only ( implicit cast to int)
 */
template<typename T, typename std::enable_if_t<std::is_integral_v<T>>* = nullptr>
inline std::wstring to_wstring(T v) {
  return format_string<int, 16>(L"%d", static_cast<int>(v));
}

/**
 * floating point type only ( implicit cast to double)
 */
template<typename T, typename std::enable_if_t<std::is_floating_point_v<T>>* = nullptr>
inline std::wstring to_wstring(T v) {
  constexpr size_t size = std::numeric_limits<T>::max_exponent10 + 20;
  return format_string<int, size>(L"%f", static_cast<double>(v));
}

#endif

std::wstring to_wstring( const char* sz);

template<typename T>
inline tstring to_tstring(T v) {
  using namespace std;
  return to_wstring(v);
}

inline
tstring to_tstring(const std::string& str) {
  return to_tstring(str.c_str());
}


tstring utf8_to_tstring(const char* sz);

inline
tstring utf8_to_tstring(const std::string& sz) {
  return utf8_to_tstring(sz.c_str());
}

std::string to_utf8(const wchar_t* sz);

#else
typedef std::string tstring;
typedef std::string_view tstring_view;

template<typename T>
inline tstring to_tstring(T v) {
  using namespace std;
  return to_string(v);
}

inline
tstring utf8_to_tstring(const char* sz) {
  return sz;
}

inline
tstring utf8_to_tstring(std::string sz) {
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


/**
 * compare ASCII Alpha ('a'..'z') ignoring case
 * use binary compare for all other
 */
template<typename CharT>
struct ci_equal_to {
  bool operator()(CharT ca, CharT cb) const {
    return UpperAlpha(ca) == UpperAlpha(cb);
  }

  CharT UpperAlpha(CharT c) const {
    return (c >= 'a' && c <= 'z') ? (c - ('a' - 'A')) : c;
  }
};

template<typename StrT>
struct ci_equal {
  using CharT = typename StrT::value_type;
  bool operator()(const StrT& a, const StrT& b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), ci_equal_to<CharT>());
  }
};

#endif
