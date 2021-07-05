/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
/*____________________________________________________________________________*/

#ifndef __stringext_h__
#define __stringext_h__

/*____________________________________________________________________________*/

#include <stddef.h>
#include <wctype.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

/*____________________________________________________________________________*/

size_t to_utf8(const wchar_t* string, char* utf, size_t size);
size_t from_utf8(const char* utf, wchar_t* string, size_t size);


#ifdef __cplusplus
}

size_t to_utf8(const char* string, char* utf, size_t size);
size_t from_utf8(const char* utf, char* string, size_t size);

template<typename CharT, size_t size>
size_t to_utf8(const CharT* string, char (&utf8)[size]) {
   static_assert(size > 0, "invalid output size");
   return to_utf8(string, utf8, size);
}

template<typename CharT, size_t size>
size_t from_utf8(const char* utf8, CharT (&string)[size]) {
   static_assert(size > 0, "invalid output size");
   return from_utf8(utf8, string, size);
}

size_t to_usascii(const char* utf8, char* ascii, size_t size);
size_t to_usascii(const wchar_t* unicode, char* ascii, size_t size);

template<typename CharT, size_t size>
size_t to_usascii(const CharT* source, char (&ascii)[size]) {
  return to_usascii(source, ascii, size);
}

inline wchar_t to_lower(wchar_t c) {
  return towlower(c);
}

inline char to_lower(char c) {
  return tolower(c);
}

const char* ci_search_substr(const char* string, const char* sub_string);
const wchar_t* ci_search_substr(const wchar_t* string, const wchar_t* sub_string);

#endif /* __cplusplus */
#endif /* __stringext_h__ */
