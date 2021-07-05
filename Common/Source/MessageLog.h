/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   MessageLog.h
 * Author: Bruno de Lacheisserie
 *
 * Created on March 21, 2015, 1:22 PM
 */

#ifndef MESSAGELOG_H
#define	MESSAGELOG_H

#include "Compiler.h"
#include <tchar.h>
#include "Util/tstring.hpp"

extern "C" {
void DebugStore(const char *Str, ...)
        gcc_printf(1,2) gcc_nonnull(1);
}

void StartupStore(const TCHAR *Str, ...)
        gcc_printf(1,2) gcc_nonnull(1);

#if defined(TESTBENCH) || !defined(NDEBUG)
  #define TestLog(...) StartupStore(__VA_ARGS__)
#else
  #define TestLog(...)
#endif

#ifndef NDEBUG
  #define DebugLog(...) StartupStore(__VA_ARGS__)
#else
  #define DebugLog(...)
#endif


tstring toHexString(const void* data, size_t size);

void StartupLogFreeRamAndStorage();

#endif	/* MESSAGELOG_H */

