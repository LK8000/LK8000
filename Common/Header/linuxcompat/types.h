
/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#ifndef	_TYPES_H_
#define _TYPES_H_

#define	BYTE	int8_t
#define BOOL	bool
#define LONG	long

typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned int UNINT32;
typedef unsigned int UINT;

typedef int INT;

typedef struct tagSIZE {
  long cx;
  long cy;
} SIZE;

typedef struct tagPOINT {
  long x;
  long y;
} POINT;

typedef struct tagRECT {
  long left;
  long top;
  long right;
  long bottom;
} RECT;

#endif	

