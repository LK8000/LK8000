/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//______________________________________________________________________________

#include "stringext.h"
#include <math.h>
#include <string.h>
#include <algorithm>
#include <assert.h>
#include "utils/array_back_insert_iterator.h"
#include "Util/UTF8.hpp"
#include "unicode/unicode_to_ascii.h"
//______________________________________________________________________________
namespace {

gcc_pure std::pair<unsigned, const char *> 
NextChar(const char *p) {
  return NextUTF8(p);
}

gcc_pure std::pair<unsigned, const wchar_t *>
NextChar(const wchar_t *p) {
  return std::make_pair(unsigned(*p), p + 1);
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

  auto next = NextChar(string);
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

    next = NextChar(next.second);
  }
  ascii[out.length()] = '\0'; // add leading '\0'

  return out.length();
}

template<typename CharT>
size_t safe_copy(const CharT* gcc_restrict src, CharT* gcc_restrict dst, size_t size) {
  assert(src && dst && size > 0); // invalid src or dst

  char* end = std::next(dst, size - 1); // let place holder for trailing '\0'
  char* p = dst;
  while (p < end) {
    if ((*p++ = *src++) == 0) {
      break; // stop copy if null terminator is found
    }
  }

  assert(p < end || *src == 0); // out string to small
  *end = 0; /* granted NULL-terminate dst */

  return std::distance(dst, p);
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
  while (*unicode && out < end) {
    out = UnicodeToUTF8(*(unicode++), out);
  }
  *out = '\0';

  return std::distance(utf8, out);
}

size_t from_utf8(const char *utf8, wchar_t *unicode, size_t size) {

  auto out = array_back_inserter(unicode, size - 1); // size - 1 to let placeholder for '\0'

  auto next = NextUTF8(utf8);
  while (next.second && !out.overflowed()) {
    out = next.first;
    next = NextUTF8(next.second);
  }
  unicode[out.length()] = L'\0';

  return out.length();
}


size_t to_utf8(const char *string, char *utf8, size_t size) {
  return safe_copy(string, utf8, size);
}

size_t from_utf8(const char *utf8, char *string, size_t size) {
  return safe_copy(utf8, string, size);
}
