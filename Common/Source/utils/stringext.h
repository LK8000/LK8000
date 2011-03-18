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

#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

/*____________________________________________________________________________*/
  
// return number of elements in the array
#define countof(array) (sizeof(array)/sizeof(array[0]))

/*____________________________________________________________________________*/
  
int ascii2unicode(const char* ascii, wchar_t* unicode, int maxChars);
int unicode2ascii(const wchar_t* unicode, char* ascii, int maxChars);
int unicode2utf(const wchar_t* unicode, char* utf, int maxChars);
int utf2unicode(const char* utf, wchar_t* unicode, int maxChars);


#ifdef __cplusplus
}
#endif
  
#endif /* __stringext_h__ */
