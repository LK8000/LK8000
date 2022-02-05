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

/**
 * required size to store unicode character with utf8 encoding 
 */ 
template<typename CharT, 
         typename std::enable_if_t<std::is_same_v<CharT, char>>* = nullptr>
size_t unicode_alloc_size(unsigned ch) {
  if (gcc_likely(ch < 0x80)) {
    return 1;
  } 
  if (gcc_likely(ch < 0x800)) {
    return 2;
  }
  if (ch < 0x10000) {
    return 3;
  }
  if (ch < 0x200000) {
    return 4;
  }
  if (ch < 0x4000000) {
    return 5;
  }
  if (ch < 0x80000000) {
    return 6;
  }
  // error
  return 0;
}

/**
 * required size to store unicode character with utf16 encoding 
 */ 
template<typename CharT, 
         typename std::enable_if_t<sizeof(CharT) == sizeof(uint16_t)>* = nullptr>
size_t unicode_alloc_size(unsigned ch) {
    if (ch < 0x10000) {
    return 1;
  }
  return 2;
}

/**
 * required size to store unicode character with utf32 encoding 
 */ 
template<typename CharT, 
         typename std::enable_if_t<sizeof(CharT) == sizeof(uint32_t)>* = nullptr>
size_t unicode_alloc_size(unsigned ch) {
  return 1;
}

template<typename InCharT, typename OutCharT, typename NextChar, typename UnicodeTo >
size_t from_to(const InCharT *in, OutCharT *out, size_t size, NextChar next_char, UnicodeTo unicode_to) {
  const auto end = std::next(out, size);
  auto p = out;

  size_t required_size = 0;
  auto next = next_char(in);
  while (next.first) {
    size_t char_size = unicode_alloc_size<OutCharT>(next.first);
    if (p && (p + char_size) < end) {
      p = unicode_to(next.first, p);
    }
    required_size += char_size;
    next = next_char(next.second);
  }
  if (p) {
    *p = '\0'; // add leading '\0'
  }

  return required_size;
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
  return from_to(string, utf8, size, NextUTF8, UnicodeToUTF8);
}

size_t from_utf8(const char *utf8, char *string, size_t size) {
  return from_to(utf8, string, size, NextUTF8, UnicodeToUTF8);
}

size_t from_ansi(const char *ansi, wchar_t *unicode, size_t size) {
  return from_to(ansi, unicode, size, NextACP,  UnicodeToWChar<wchar_t>);
}

size_t from_ansi(const char *ansi, char *utf8, size_t size) {
  return from_to(ansi, utf8, size, NextACP,  UnicodeToUTF8);
}

const char* ci_search_substr(const char* string, const char* sub_string) {
  return ci_search_substr<const char>(string, sub_string);
}

const wchar_t* ci_search_substr(const wchar_t* string, const wchar_t* sub_string) {
  return ci_search_substr<const wchar_t>(string, sub_string);
}
