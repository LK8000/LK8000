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

#include "xcs/Compiler.h"
#include <tchar.h>
#include <stdarg.h>
#include <iostream>
#include "Util/tstring.hpp"

/**
 * it's for debug build only.
 *  - file are cleared by first use.
 */
void DebugStore(const char* fmt, ...)
        gcc_printf(1,2) gcc_nonnull(1);

/**
 * add string to Runtime.log
 */
inline void StartupStore(const TCHAR* fmt, ...)
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


void StartupStoreV(const TCHAR* fmt, va_list ap);


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

/**
 * stream to write log line to RUNTIME.LOG.
 */
template<typename _CharT, typename _Traits = std::char_traits<_CharT>>
class startup_store_streambuf : public std::basic_streambuf<_CharT, _Traits> {

    using char_type = typename std::basic_streambuf<_CharT, _Traits>::char_type;
    using traits_type = typename std::basic_streambuf<_CharT, _Traits>::traits_type;
    using int_type = typename traits_type::int_type;

    std::basic_string<char_type> string;

protected:
    int_type overflow(int_type c) override {
        if (c == '\n') {
            StartupStore(_T("%s"), to_tstring(string).c_str());
            string.clear();
        } else {
            string.push_back( c );
        }
        return traits_type::not_eof(c);
    }
};

template<typename _CharT, typename _Traits = std::char_traits<_CharT>>
class startup_store_ostream : public std::basic_ostream<_CharT, _Traits> {
public:
	startup_store_ostream () : std::ostream(&_streambuf) { }

protected:
	startup_store_streambuf<_CharT, _Traits> _streambuf;
};

tstring toHexString(const void* data, size_t size);

void StartupLogFreeRamAndStorage();

#endif	/* MESSAGELOG_H */
