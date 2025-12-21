#ifndef __NEWRES_H__
#define __NEWRES_H__

#ifdef RC_INVOKED
#ifndef _INC_WINDOWS
#define _INC_WINDOWS
	#include "winuser.h"           // extract from windows header
	#include "winver.h"
#endif
#endif

#ifdef IDC_STATIC
#undef IDC_STATIC
#endif
#define IDC_STATIC      (-1)

#endif //__NEWRES_H__
