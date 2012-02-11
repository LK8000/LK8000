/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: externs.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef EXTERNS_H
#define EXTERNS_H


#ifndef __MINGW32__
 #if defined(CECORE)
 #include "winbase.h"
 #endif
 #if (WINDOWSPC<1)
 #include "projects.h"
 #endif
#else
 #include "wcecompat/ts_string.h"
 #include "mingw32compat/StdAfx.h"
#endif

// options first, then all dependencies
#include "options.h"
#include "Sizes.h"
#include "Defines.h"
#include "Enums.h"
#include "Units.h"
#include "lk8000.h"

#include "Statistics.h"
#include "Dialogs.h"
#include "ContestMgr.h"
#include "device.h"

#include "Globals.h"

// Include header for heap allocation checking
#include "utils/heapcheck.h"

// Include assert for LK testbench
#include "LKAssert.h"

//
// Functions
//
extern TCHAR 		*gmfpathname();
extern TCHAR		*gmfbasename();
extern int		GetGlobalModelName();
extern void		SmartGlobalModelType();
extern short		InstallSystem();
extern bool		CheckRootDir();
extern bool		CheckDataDir();
extern bool		CheckLanguageDir();
extern bool		CheckPolarsDir();
extern bool		CheckRegistryProfile();
extern void		ConvToUpper( TCHAR *);
extern bool		Debounce(int debouncetime);
extern bool		Debounce();

#endif

