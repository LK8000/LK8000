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
#include "utf8/unchecked.h"
#include "utils/array_back_insert_iterator.h"
#include "Util/UTF8.hpp"
#include "unicode/unicode_to_ascii.h"
//______________________________________________________________________________


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Converts ASCII string encoded in system code page into Unicode string.
/// \return Unicode string length, -1 on conversion error
int ascii2unicode(const char* ascii, wchar_t* unicode, int maxChars) {
    // The conversion from ASCII to Unicode and vice versa are quite trivial. By design, the first 128 Unicode
    // values are the same as ASCII (in fact, the first 256 are equal to ISO-8859-1).

    wchar_t* end = std::copy_n(ascii, std::max(maxChars-1, (int)strlen(ascii)), unicode);
    *end = L'\0';
    return std::distance(unicode, end);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Converts Unicode string into ASCII encoded in system code page.
/// \return ASCII string length, -1 on conversion error (insufficient buffer e.g.)
int unicode2ascii(const wchar_t* unicode, char* ascii, int maxChars) {
    // The conversion from ASCII to Unicode and vice versa are quite trivial. By design, the first 128 Unicode
    // values are the same as ASCII (in fact, the first 256 are equal to ISO-8859-1).

    char* end = std::copy_n(unicode, std::max(maxChars-1, (int)wcslen(unicode)), ascii);
    *end = '\0';
    return std::distance(ascii, end);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Converts Unicode string into UTF-8 encoded string.
/// \return UTF8 string size [octets], -1 on conversion error (insufficient buffer e.g.)
int unicode2utf(const wchar_t* unicode, char* utf, int maxChars)
{
  #ifndef SYS_UTF8_CONV

  // we will use our own UTF16->UTF8 conversion (WideCharToMultiByte(CP_UTF8)
  // is not working on some Win CE systems)
  size_t len = wcslen(unicode);

  auto iter = array_back_inserter(utf, maxChars - 1);

  iter = utf8::unchecked::utf16to8(unicode, unicode + len, iter);

  if (!iter.overflowed()) {
    utf[iter.length()] = '\0';
    return(iter.length());
  }

  // for safety reasons, return empty string
  if (maxChars >= 1)
    utf[0] = '\0';
  return(-1);

  #else /* just for fallback return to old code, to be removed after tests */

  int res = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, utf, maxChars, NULL, NULL);

  if (res > 0)
    return(res - 1);

  // for safety reasons, return empty string
  if (maxChars >= 1)
    utf[0] = '\0';
  return(-1);

  #endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Converts UTF-8 encoded string into Unicode encoded string.
/// \return Unicode string size [TCHARs], -1 on conversion error (insufficient buffer e.g.)
int utf2unicode(const char* utf, wchar_t* unicode, int maxChars)
{
  #ifndef SYS_UTF8_CONV

  // we will use our own UTF16->UTF8 conversion (MultiByteToWideChar(CP_UTF8)
  // is not working on some Win CE systems)
  size_t len = strlen(utf);

  // first check if UTF8 is correct (utf8to16() may not be called on invalid string)
  if (utf8::find_invalid(utf, utf + len) == (utf + len)) {
    auto iter = array_back_inserter(unicode, maxChars - 1);

    iter = utf8::unchecked::utf8to16(utf, utf + len, iter);

    unicode[iter.length()] = '\0';
    return(iter.length());
  }

  // for safety reasons, return empty string
  if (maxChars >= 1)
    unicode[0] = '\0';
  return(-1);

  #else /* just for fallback return to old code, to be removed after tests */

  int res = MultiByteToWideChar(CP_UTF8, 0, utf, -1, unicode, maxChars);

  if (res > 0)
    return(res - 1);

  // for safety reasons, return empty string
  if (maxChars >= 1)
    unicode[0] = '\0';
  return(-1);

  #endif
}

gcc_pure
static std::pair<unsigned, const TCHAR *>
NextChar(const TCHAR *p)
{
#ifdef _UNICODE
  return std::make_pair(unsigned(*p), p + 1);
#else
  return NextUTF8(p);
#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Converts Unicode string into US-ASCII string (writing as much as possible
/// characters into @p ascii). Output string will always be terminated by '\0'.
///
/// Characters are converted into their most similar representation
/// in US-ASCII. Nonconvertable characters are replaced by '?'.
///
/// Output string will always be terminated by '\0'.
///
/// @param unicode    input string (must be terminated with '\0')
/// @param ascii      output buffer
/// @param maxChars   output buffer size
///
/// @retval  1  all characters copied
/// @retval -1  some characters could not be copied due to buffer size
///
int TCHAR2usascii(const TCHAR* unicode, char* ascii, int outSize)
{
  auto out = array_back_inserter(ascii, outSize - 1); // size - 1 to let placeholder for '\0'

  auto next = NextChar(unicode);
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
  return (out.overflowed() ? -1 : 1);
} // TCHAR2usascii()



int ascii2TCHAR(const char* ascii, TCHAR* unicode, int maxChars) {
#if defined(_UNICODE)
    return  ascii2unicode(ascii, unicode, maxChars);
#else
    size_t len = std::min(_tcslen(ascii), (size_t)maxChars);
    _tcsncpy(unicode, ascii, maxChars);
    unicode[maxChars-1] = '\0';
    return len;
#endif
}

int TCHAR2ascii(const TCHAR* unicode, char* ascii, int maxChars) {
#if defined(_UNICODE)
    return  unicode2ascii(unicode, ascii, maxChars);
#else
    size_t len = std::min(_tcslen(unicode), (size_t)maxChars);
    _tcsncpy(ascii, unicode, maxChars);
    ascii[maxChars-1] = '\0';

    return len;
#endif
}

int TCHAR2utf(const TCHAR* unicode, char* utf, int maxChars) {
#if defined(_UNICODE)
    return  unicode2utf(unicode, utf, maxChars);
#elif defined(_MBCS)
    wchar_t temp[maxChars];
    size_t len = mbstowcs(temp, unicode, maxChars);
    if(len!=(size_t)-1) {
        return unicode2utf(temp, utf, maxChars);
    }
    // if error, return simple copy
    len = std::min(_tcslen(unicode), (size_t)maxChars);
    _tcsncpy(utf, unicode, maxChars);
    return len;
#else
    size_t len = std::min(_tcslen(unicode), (size_t)maxChars-1);
    _tcsncpy(utf, unicode, maxChars);
    utf[maxChars-1] = '\0';

    return len;
#endif
}

int utf2TCHAR(const char* utf, TCHAR* unicode, int maxChars){
#if defined(_UNICODE)
    return  utf2unicode(utf, unicode, maxChars);
#elif defined(_MBCS)
    wchar_t temp[maxChars];
    memset(unicode, 0, maxChars*sizeof(TCHAR));
    utf2unicode(utf, temp, maxChars);
    return wcstombs(unicode, temp, maxChars);
#else
    assert(ValidateUTF8(utf));
    size_t len = std::min(_tcslen(utf), (size_t)maxChars);
    _tcsncpy(unicode, utf, maxChars);
    unicode[maxChars-1] = '\0';
    return len;
#endif
}
