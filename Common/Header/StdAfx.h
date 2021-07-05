// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__4F78E65A_F28C_412D_9648_C8F491327F80__INCLUDED_)
#define AFX_STDAFX_H__4F78E65A_F28C_412D_9648_C8F491327F80__INCLUDED_

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include <math.h>
#include <malloc.h>
#include <commctrl.h>
#include <stdlib.h>

#include "mingw32compat/gmtime_r.h"

#ifndef ASSERT
#define ASSERT(x) assert(x)
#endif

typedef unsigned int uint;
typedef unsigned char byte;

#if (WINDOWSPC>0)
#define VK_APP1     0x31
#define VK_APP2     0x32
#define VK_APP3     0x33
#define VK_APP4     0x34
#define VK_APP5     0x35
#define VK_APP6     0x36
#else
#define VK_APP1     0xC1
#define VK_APP2     0xC2
#define VK_APP3     0xC3
#define VK_APP4     0xC4
#define VK_APP5     0xC5
#define VK_APP6     0xC6
#endif


#endif // !defined(AFX_STDAFX_H__4F78E65A_F28C_412D_9648_C8F491327F80__INCLUDED_)
