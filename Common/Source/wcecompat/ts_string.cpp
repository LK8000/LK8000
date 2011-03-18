/*  wcecompat: Windows CE C Runtime Library "compatibility" library.
 *
 *  Copyright (C) 2001-2002 Essemer Pty Ltd.  All rights reserved.
 *  http://www.essemer.com.au/
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <memory.h>
#include <string.h>
#include <windows.h>
#include "ts_string.h"
#include "StdAfx.h"

// return Unicode string length, -1 on conversion error
int ascii2unicode(const char* ascii, WCHAR* unicode)
{
  return(ascii2unicode(ascii, unicode, 0xffffff));
}

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

// return ASCII string length, -1 on conversion error
int unicode2ascii(const WCHAR* unicode, char* ascii)
{
  return(unicode2ascii(unicode, ascii, 0xffffff));
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

//
// ascii/unicode typesafe versions of strcat
//

char* ts_strcat(char* dest, const unsigned short* src)
{
	char* p = dest;
	while (*p != '\0')
		p++;
	unicode2ascii((const wchar_t *)src, p);
	return dest;
}

unsigned short* ts_strcat(unsigned short* dest, const char* src)
{
	unsigned short* p = dest;
	while (*p != '\0')
		p++;
	ascii2unicode(src, (wchar_t *)p);
	return dest;
}


//
// ascii/unicode typesafe versions of strdup
//

char* ts_strdup_unicode_to_ascii(const unsigned short* str)
{
	char* result = (char*)malloc(wcslen((const wchar_t *)str)+1);
	if (result == NULL)
		return NULL;
	unicode2ascii((const wchar_t *)str, result);
	return result;
}

unsigned short* ts_strdup_ascii_to_unicode(const char* str)
{
	unsigned short* result = (unsigned short*)malloc((strlen(str)+1)*2);
	if (result == NULL)
		return NULL;
	ascii2unicode(str, (wchar_t *)result);
	return result;
}
