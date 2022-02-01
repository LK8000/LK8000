/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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
#include <stdarg.h>
#include "Util/tstring.hpp"

/**
 * it's for debug build only.
 *  - file are cleared by first use.
 */
void DebugStore(const char *Str, ...)
        gcc_printf(1,2) gcc_nonnull(1);

/**
 * add string to Runtime.log
 */
inline void StartupStore(const TCHAR *Str, ...)
        gcc_printf(1,2) gcc_nonnull(1);

/**
 * add string to Runtime.log only if #TESTBENCH is defined or if #NDEBUG is not defined
 */
inline void TestLog(const TCHAR* fmt, ...)
        gcc_printf(1,2) gcc_nonnull(1);

/**
 * add string to Runtime.log only if #NDEBUG is not defined
 */
inline void DebugLog(const TCHAR* fmt, ...)
        gcc_printf(1,2) gcc_nonnull(1);


void StartupStoreV(const TCHAR *Str, va_list ap);


void StartupStore(const TCHAR* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    StartupStoreV(fmt, ap);
    va_end(ap);
}

void TestLog(const TCHAR* fmt, ...) {
#if defined(TESTBENCH) || !defined(NDEBUG)
    va_list ap;
    va_start(ap, fmt);
    StartupStoreV(fmt, ap);
    va_end(ap);
#endif
}

void DebugLog(const TCHAR* fmt, ...) {
#ifndef NDEBUG
    va_list ap;
    va_start(ap, fmt);
    StartupStoreV(fmt, ap);
    va_end(ap);
#endif
}


tstring toHexString(const void* data, size_t size);

void StartupLogFreeRamAndStorage();

#endif	/* MESSAGELOG_H */

