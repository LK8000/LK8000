/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: externs.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef EXTERNS_H
#define EXTERNS_H


// options first, then all dependencies
#include "Compiler.h"
#include "options.h"

#ifdef __linux__
#include "StdLinux.h"
#else
#include "StdAfx.h"
#endif
#ifdef __cplusplus
#include "Screen/Features.hpp"
#endif
#include "compatibility.h"


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
#include "Task.h"
#include "Comm/device.h"
#include "Globals.h"
#include "LKLanguage.h"
#include "Window/WndMain.h"
#include "LKCpu.h"

// Include assert for LK testbench
#include "LKAssert.h"
#include "MathFunctions.h"

#include "ScreenGeometry.h"
//
// Common Functions
//
void InstallSystem();
bool CheckRootDir();
bool CheckDataDir();
bool CheckLanguageDir();
bool CheckPolarsDir();
bool CheckFilesystemWritable();

bool Debounce(int debouncetime);
bool Debounce();

void DoStatusMessage(const TCHAR* text, const TCHAR* data = nullptr, const bool playsound = true) gcc_nonnull(1);

extern std::unique_ptr<WndMain> main_window;


//
// Various
//
using std::min;
using std::max;

#endif
