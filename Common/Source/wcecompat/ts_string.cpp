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
#include <stdlib.h>
#include <windows.h>
#include "ts_string.h"

#include "utils/heapcheck.h"


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
