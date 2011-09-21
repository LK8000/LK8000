/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef EXTERNS_H
#define EXTERNS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "options.h"
#include "Defines.h"
#include "Sizes.h"
#include "lk8000.h"
#include "Parser.h"
#include "Calculations.h"
#include "MapWindow.h"
#include "Task.h"
#include "Statistics.h"
#include "Dialogs.h"
#include "Utils2.h"
#include "ContestMgr.h"
#include "device.h"

/*
typedef enum {psInitInProgress=0, psInitDone=1, psFirstDrawDone=2, psNormalOp=3} StartupState_t;
// 0: not started at all
// 1: everything is alive
// 2: done first draw
// 3: normal operation
*/
#include "Enums.h"
#include "Globals.h"


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
extern bool		Debounce();

#endif

