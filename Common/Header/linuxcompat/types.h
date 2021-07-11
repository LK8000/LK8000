/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#ifndef	_LINUXCOMPAT_TYPES_H_
#define _LINUXCOMPAT_TYPES_H_

#include "tchar.h"
#include "stdint.h"
#include "ScreenCoordinate.h"
#include "wingdi.h"

/* we use typedef instead of define to avoid hell of macro conflict */

typedef uint8_t	BYTE;
typedef int32_t LONG; //windows is LLP64 system so long is 32bit signed integer

typedef bool BOOL;

constexpr BOOL FALSE = false;
constexpr BOOL TRUE  = true;

typedef uint16_t WORD;

typedef uint32_t UINT;
typedef int32_t INT;

typedef char* LPTSTR, LPSTR;
typedef const char* LPCTSTR;

#endif	// _LINUXCOMPAT_TYPES_H_
