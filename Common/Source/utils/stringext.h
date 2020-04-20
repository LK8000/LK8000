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

#ifdef __cplusplus
extern "C" {
#endif

/*____________________________________________________________________________*/

int ascii2TCHAR(const char* ascii, TCHAR* unicode, int maxChars);
int TCHAR2ascii(const TCHAR* unicode, char* ascii, int maxChars);
int TCHAR2utf(const TCHAR* unicode, char* utf, int maxChars);
int utf2TCHAR(const char* utf, TCHAR* unicode, int maxChars);

int TCHAR2usascii(const TCHAR* unicode, char* ascii, int outSize);

int ascii2unicode(const char* ascii, wchar_t* unicode, int maxChars);
int unicode2ascii(const wchar_t* unicode, char* ascii, int maxChars);
int unicode2utf(const wchar_t* unicode, char* utf, int maxChars);
int utf2unicode(const char* utf, wchar_t* unicode, int maxChars);

#ifdef __cplusplus
}
#endif

#endif /* __stringext_h__ */
