/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//______________________________________________________________________________

#include "stringext.h"
#include <cassert>
#include <string_view>
#include <algorithm>

#include <cstring>
#include "array_back_insert_iterator.h"
#include "Util/UTF8.hpp"
#include "unicode/unicode_to_ascii.h"
#include "unicode/UTF16.h"
#include "unicode/CP1252.h"
//______________________________________________________________________________
namespace {

/**
 * @param p utf-8 string
 */
template<typename Tp, 
         typename std::enable_if_t<std::is_same<Tp, char>::value>* = nullptr>
std::pair<unsigned, const Tp *>
NextChar(const Tp *p) {
  return NextUTF8(p);
}

/**
 * @param p utf-16 string
 */
template<typename Tp, 
         typename std::enable_if_t<sizeof(Tp) == sizeof(uint16_t)>* = nullptr>
std::pair<unsigned, const Tp *>
NextChar(const Tp *p) {
  auto pair = NextUTF16((uint16_t*)p);
  const Tp* next = reinterpret_cast<const Tp*>(pair.second);
  return std::make_pair<unsigned, const Tp *>(std::move(pair.first), std::move(next));
}

/**
 * @param p utf-32 string
 */
template<typename Tp, 
         typename std::enable_if_t<sizeof(Tp) == sizeof(uint32_t)>* = nullptr>
std::pair<unsigned, const Tp *>
NextChar(const Tp *p) {
  return std::make_pair(unsigned(*p), p + 1);
}

/**
 * this 2 template function allow Compiler to choose right encoding for wchar_t* string
 *   depending of sizeof wchar_t (utf16 for 2 bytes, utf32 for 4 bytes)
 */
template<typename Tp, 
         typename std::enable_if_t<sizeof(Tp) == sizeof(uint16_t)>* = nullptr>
Tp* UnicodeToWChar(unsigned ch, Tp* q) {
  return reinterpret_cast<Tp*>(UnicodeToUTF16(ch, reinterpret_cast<uint16_t*>(q)));
}

template<typename Tp, 
         typename std::enable_if_t<sizeof(Tp) == sizeof(uint32_t)>* = nullptr>
Tp* UnicodeToWChar(unsigned ch, Tp* q) {
  (*q++) = ch;
  return q;
}

/**
 * Converts Unicode string into US-ASCII string (writing as much as possible
 * characters into @p ascii). Output string will always be terminated by '\0'.
 *
 * Characters are converted into their most similar representation
 * in US-ASCII. Nonconvertable characters are replaced by '?'.
 *
 * Output string will always be terminated by '\0'.
 *
 * @param unicode    input string (must be terminated with '\0')
 * @param ascii      output buffer
 * @param size   output buffer size
 * 
 * @return output string length.
 */
template <typename CharT>
size_t to_usascii(const CharT *string, char *ascii, size_t size) {

  auto out = array_back_inserter(ascii, size - 1); // size - 1 to let placeholder for '\0'

  auto next = NextChar<CharT>(string);
  while (next.first && !out.overflowed()) {

    if (next.first <= 127) {
        out = next.first;
    } else {
      const char* ascii_str = unicode_to_ascii(next.first);
      if (ascii_str) {
        out = ascii_str;
      } else {
        out = '?';
      }
    }

    next = NextChar<CharT>(next.second);
  }
  ascii[out.length()] = '\0'; // add leading '\0'

  return out.length();
}

size_t safe_copy(const char* gcc_restrict src, char* gcc_restrict dst, size_t size) {
  assert(src && dst && size > 0); // invalid src or dst

  char* end = std::next(dst, size - 1); // let place holder for trailing '\0'
  char* p = dst;
  while (p < end && *src) {
    *p++ = *src++;
  }
  *p = 0; /* granted NULL-terminate dst */

  if(p >= end && *src != 0) {
    CropIncompleteUTF8(dst);
  }

  return std::distance(dst, p);
}

template<typename CharT>
CharT* ci_search_substr(CharT* string, CharT* sub_string) {
  
  auto ci_equal = [](CharT ch1, CharT ch2) {
    return to_lower(ch1) == to_lower(ch2);
  };

  std::basic_string_view<CharT> str2(sub_string);
  if (str2.empty()) {
    return nullptr;
  }

  std::basic_string_view<CharT> str1(string);
  if (str1.empty()) {
    return nullptr;
  }

  auto it = std::search(std::begin(str1), std::end(str1), 
                        std::begin(str2), std::end(str2), ci_equal);
  
  if (it != std::end(str1)) {
    return it;
  }
  return nullptr;
}

template<typename InCharT, typename OutCharT, typename NextChar, typename UnicodeTo >
size_t from_to(const InCharT *in, OutCharT *out, size_t size, NextChar next_char, UnicodeTo unicode_to) {
  const auto end = std::next(out, size - 1); // size - 1 to let placeholder for '\0'
  auto p = out;

  auto next = next_char(in);
  while (next.first && out < end) {
    p = unicode_to(next.first, p);
    next = next_char(next.second);
  }
  *p = '\0'; // add leading '\0'

  return std::distance(out, p);
}


template<typename CharT>
size_t max_size_need(const char* string);

template<>
size_t max_size_need<char>(const char* string) {
  return (strlen(string) * 4) + 1;
}

template<>
size_t max_size_need<wchar_t>(const char* string) {
  return strlen(string);
}

template<typename CharT>
std::basic_string<CharT> utf8_to_string(const char* utf8) {
  std::basic_string<CharT> out;
  out.resize(max_size_need<CharT>(utf8));
  size_t size = from_utf8(utf8, out.data(), out.size());
  out.resize(size);
  return out;
}

template<typename CharT>
std::basic_string<CharT> ansi_to_string(const char* ansi) {
  std::basic_string<CharT> out;
  out.resize(max_size_need<CharT>(ansi));
  size_t size = from_ansi(ansi, out.data(), out.size());
  out.resize(size);
  return out;
}

template<typename CharT>
std::basic_string<CharT> from_unknow_charset(const char* string) {
  if (ValidateUTF8(string)) {
    return utf8_to_string<CharT>(string);
  }
  else {
    return ansi_to_string<CharT>(string);
  }
}

} // namespace

size_t to_usascii(const char* utf8, char* ascii, size_t size) {
  return to_usascii<char>(utf8, ascii, size);
}

size_t to_usascii(const wchar_t* unicode, char* ascii, size_t size) {
  return to_usascii<wchar_t>(unicode, ascii, size);
}


size_t to_utf8(const wchar_t *unicode, char *utf8, size_t size) {
  return from_to(unicode, utf8, size, NextChar<wchar_t>,  UnicodeToUTF8);
}

size_t from_utf8(const char *utf8, wchar_t *unicode, size_t size) {
  return from_to(utf8, unicode, size, NextUTF8,  UnicodeToWChar<wchar_t>);
}

size_t to_utf8(const char *string, char *utf8, size_t size) {
  return safe_copy(string, utf8, size);
}

size_t from_utf8(const char *utf8, char *string, size_t size) {
  return safe_copy(utf8, string, size);
}

tstring from_utf8(const char *utf8) {
  return utf8_to_string<tstring::value_type>(utf8);
}

size_t from_ansi(const char *ansi, wchar_t *unicode, size_t size) {
  return from_to(ansi, unicode, size, NextACP,  UnicodeToWChar<wchar_t>);
}

size_t from_ansi(const char *ansi, char *utf8, size_t size) {
  return from_to(ansi, utf8, size, NextACP,  UnicodeToUTF8);
}

tstring from_ansi(const char *ansi) {
  return ansi_to_string<tstring::value_type>(ansi);
}

std::string ansi_to_utf8(const char *ansi) {
  return ansi_to_string<std::string::value_type>(ansi);
}

const char* ci_search_substr(const char* string, const char* sub_string) {
  return ci_search_substr<const char>(string, sub_string);
}

const wchar_t* ci_search_substr(const wchar_t* string, const wchar_t* sub_string) {
  return ci_search_substr<const wchar_t>(string, sub_string);
}

/**
 * Helper to do implicit charset transcoding
 */
class from_unknow_charset_t {
public:
  explicit from_unknow_charset_t(const char* string) : input_string(string) {}

  operator std::string() const {
    return from_unknow_charset<std::string::value_type>(input_string);
  }

  operator std::wstring() const {
    return from_unknow_charset<std::wstring::value_type>(input_string);
  }

private:
  const char* input_string;
};

from_unknow_charset_t from_unknow_charset(const char* string) {
  return  from_unknow_charset_t(string);
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

TEST_CASE("to_tchar_string") {
    const std::string test_utf8("10€ƒ¶Æ");
    const std::wstring test_unicode(L"10€ƒ¶Æ");

  	SUBCASE("UTF8 -> UTF8") {
      std::string utf8 = from_unknow_charset("10€ƒ¶Æ");
      CHECK_EQ(utf8, test_utf8);
    }

  	SUBCASE("ANSI -> UTF8") {
      std::string utf8 = from_unknow_charset("10\x80\x83\xB6\xC6");
      CHECK_EQ(utf8, test_utf8);
    }

  	SUBCASE("UTF8 -> UNICODE") {
      std::wstring unicode = from_unknow_charset("10€ƒ¶Æ");
      CHECK_EQ(unicode, test_unicode);
    }

  	SUBCASE("ANSI -> UNICODE") {
      std::wstring unicode = from_unknow_charset("10\x80\x83\xB6\xC6");
      CHECK_EQ(unicode, test_unicode);
    }
}

#endif
