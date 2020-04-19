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

#include <tchar.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*____________________________________________________________________________*/

int TCHAR2ascii(const TCHAR* unicode, char* ascii, int maxChars);
int TCHAR2utf(const TCHAR* unicode, char* utf, int maxChars);
int utf2TCHAR(const char* utf, TCHAR* unicode, int maxChars);

int unicode2ascii(const wchar_t* unicode, char* ascii, int maxChars);

#ifdef __cplusplus
}

size_t to_usascii(const char* utf8, char* ascii, size_t size);
size_t to_usascii(const wchar_t* unicode, char* ascii, size_t size);

template<typename CharT, size_t size>
size_t to_usascii(const CharT* source, char (&ascii)[size]) {
  return to_usascii(source, ascii, size);
}

#endif /* __cplusplus */
#endif /* __stringext_h__ */
