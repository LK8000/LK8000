
/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#ifndef	_TYPES_H_
#define _TYPES_H_

#include "tchar.h"
#include "stdint.h"
#include "ScreenCoordinate.h"
#include "wingdi.h"

#define	BYTE	unsigned char
#define BOOL	bool
#define LONG	long

#define FALSE false
#define TRUE true

//typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned int UNINT32;
typedef unsigned int UINT;

typedef int INT;

typedef char* LPTSTR, LPSTR;
typedef const char* LPCTSTR;

#endif	

