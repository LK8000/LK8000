/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//______________________________________________________________________________

#include "stringext.h"
#include <cassert>
#include <string_view>
#include <algorithm>
#include "array_back_insert_iterator.h"
#include "Util/UTF8.hpp"
#include "unicode/unicode_to_ascii.h"
#include "unicode/UTF16.h"
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
  while (p < end) {
    if ((*p++ = *src++) == 0) {
      break; // stop copy if null terminator is found
    }
  }
  *end = 0; /* granted NULL-terminate dst */

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

} // namespace

size_t to_usascii(const char* utf8, char* ascii, size_t size) {
  return to_usascii<char>(utf8, ascii, size);
}

size_t to_usascii(const wchar_t* unicode, char* ascii, size_t size) {
  return to_usascii<wchar_t>(unicode, ascii, size);
}


size_t to_utf8(const wchar_t *unicode, char *utf8, size_t size) {

  const auto end = std::next(utf8, size - 1); // size - 1 to let placeholder for '\0'
  auto out = utf8;

  auto next = NextChar<wchar_t>(unicode);
  while (next.first && out < end) {
    out = UnicodeToUTF8(next.first, out);
    next = NextChar<wchar_t>(next.second);
  }
  *out = '\0'; // add leading '\0'

  return std::distance(utf8, out);
}

size_t from_utf8(const char *utf8, wchar_t *unicode, size_t size) {

  const auto end = std::next(unicode, size - 1); // size - 1 to let placeholder for '\0'
  auto out = unicode;

  auto next = NextUTF8(utf8);
  while (next.first && out < end) {
    out = UnicodeToWChar(next.first, out);
    next = NextUTF8(next.second);
  }
  *out = '\0'; // add leading '\0'

  return std::distance(unicode, out);
}


size_t to_utf8(const char *string, char *utf8, size_t size) {
  return safe_copy(string, utf8, size);
}

size_t from_utf8(const char *utf8, char *string, size_t size) {
  return safe_copy(utf8, string, size);
}


const char* ci_search_substr(const char* string, const char* sub_string) {
  return ci_search_substr<const char>(string, sub_string);
}

const wchar_t* ci_search_substr(const wchar_t* string, const wchar_t* sub_string) {
  return ci_search_substr<const wchar_t>(string, sub_string);
}
