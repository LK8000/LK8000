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

#ifdef UNICODE

size_t unicode_to_utf8(const wchar_t *unicode, char *utf8, size_t size) {

  const auto end = std::next(utf8, size - 1); // size - 1 to let placeholder for '\0'
  auto out = utf8;
  while (*unicode && out < end) {
    out = UnicodeToUTF8(*(unicode++), out);
  }
  *out = '\0';

  return std::distance(utf8, out);
}

size_t utf8_to_unicode(const char *utf8, wchar_t *unicode, size_t size) {

  auto out = array_back_inserter(unicode, size - 1); // size - 1 to let placeholder for '\0'

  auto next = NextUTF8(utf8);
  while (next.second && !out.overflowed()) {
    out = next.first;
    next = NextUTF8(next.second);
  }
  unicode[out.length()] = L'\0';

  return out.length();
}
#endif

} // namespace

size_t to_usascii(const char* utf8, char* ascii, size_t size) {
  return to_usascii<char>(utf8, ascii, size);
}

size_t to_usascii(const wchar_t* unicode, char* ascii, size_t size) {
  return to_usascii<wchar_t>(unicode, ascii, size);
}

size_t TCHAR2utf(const TCHAR *string, char *utf8, size_t size) {
#if defined(_UNICODE)
  return unicode_to_utf8(string, utf8, size);
#else
  assert(ValidateUTF8(string));
  _tcsncpy(utf8, string, size);
  utf8[size - 1] = '\0';

  return strlen(utf8);
#endif
}

size_t utf2TCHAR(const char *utf8, TCHAR *string, size_t size) {
  assert(ValidateUTF8(utf8));

#if defined(_UNICODE)
  return utf8_to_unicode(utf8, string, size);
#else
  _tcsncpy(string, utf8, size);
  string[size - 1] = '\0';
  return size;
#endif
}
