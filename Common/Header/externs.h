/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: externs.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef EXTERNS_H
#define EXTERNS_H

#ifdef __linux__
#include "StdLinux.h"
#else
#include "StdAfx.h"
#endif

// options first, then all dependencies
#include "Compiler.h"
#ifdef __cplusplus
#include "Screen/Features.hpp"
#endif
#include "compatibility.h"

#include "options.h"
#include "utils/filesystem.h"
#include "Sizes.h"
#include "Defines.h"
#include "Enums.h"
#include "Units.h"
#include "lk8000.h"
#include "Macro.h"

#include "Screen/LKColor.h"
#include "Screen/LKPen.h"

#include "Parser.h"
#include "Calculations.h"
#include "Statistics.h"
#include "LKAirspace.h"
#include "Airspace.h"
#include "MapWindow.h"
#include "Utils.h"
#include "ContestMgr.h"
#include "ComPort.h"
#include "Task.h"
#include "device.h"
#include "Globals.h"
#include "LKLanguage.h"
#include "Window/WndMain.h"
#include "LKCpu.h"

// Include header for heap allocation checking
// #include "utils/heapcheck.h"

// Include assert for LK testbench
#include "LKAssert.h"
#include "MathFunctions.h"
//
// Common Functions
//
extern short	InstallSystem();
extern bool		CheckRootDir();
extern bool		CheckDataDir();
extern bool		CheckLanguageDir();
extern bool		CheckPolarsDir();
extern bool		CheckRegistryProfile();
extern bool		Debounce(int debouncetime);
extern bool		Debounce();

extern void DoStatusMessage(const TCHAR* text, const TCHAR* data = NULL, const bool playsound = true);

extern WndMain MainWindow;


//
// Various
//
using std::min;
using std::max;

#endif

