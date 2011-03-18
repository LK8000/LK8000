/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//______________________________________________________________________________

#include "StdAfx.h"
#include "stringext.h"

//______________________________________________________________________________


// return Unicode string length, -1 on conversion error
int ascii2unicode(const char* ascii, WCHAR* unicode, int maxChars)
{
  int res = MultiByteToWideChar(CP_ACP, 0, ascii, -1, unicode, maxChars);

  if (res > 0)
    return(res - 1);
  
  // for safety reasons, return empty string  
  if (maxChars >= 1)
    unicode[0] = 0;
  return(-1);
}

// return ASCII string length, -1 on conversion error (insufficient buffer e.g.)
int unicode2ascii(const WCHAR* unicode, char* ascii, int maxChars)
{
  int res = WideCharToMultiByte(CP_ACP, 0, unicode, -1, ascii, maxChars, NULL, NULL);
  
  if (res > 0)
    return(res - 1);
  
  // for safety reasons, return empty string  
  if (maxChars >= 1)
    ascii[0] = '\0';
  return(-1);
}

// return UTF8 string length, -1 on conversion error (insufficient buffer e.g.)
int unicode2utf(const WCHAR* unicode, char* utf, int maxChars)
{
  int res = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, utf, maxChars, NULL, NULL);
  
  if (res > 0)
    return(res - 1);
  
  // for safety reasons, return empty string  
  if (maxChars >= 1)
    utf[0] = '\0';
  return(-1);
}

// return Unicode string length, -1 on conversion error (insufficient buffer e.g.)
int utf2unicode(const char* utf, WCHAR* unicode, int maxChars)
{
  int res = MultiByteToWideChar(CP_UTF8, 0, utf, -1, unicode, maxChars);
  
  if (res > 0)
    return(res - 1);
  
  // for safety reasons, return empty string  
  if (maxChars >= 1)
    unicode[0] = '\0';
  return(-1);
}
