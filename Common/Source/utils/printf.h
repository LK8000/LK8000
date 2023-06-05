/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   printf.h
 * Author: Bruno de Lacheisserie
 *
 * Created on February 09, 2022, 23:22 PM
 */

#ifndef _UTILS_PRINTF_H_
#define _UTILS_PRINTF_H_

#include "Util/UTF8.hpp"
#include <cstdio>
#include <cstring>
#include <iterator>
#ifdef UNICODE
  #include <wchar.h>
#endif

namespace lk {

#ifdef UNICODE

    template<typename ...Args> 
    inline
    int snprintf (wchar_t* stream, size_t count, const wchar_t* format, Args... args) {
        size_t len = _snwprintf(stream, count, format, std::forward<Args>(args)...);
        /** For all functions other than snprintf, if len = count, len characters are
         *  stored in buffer, no null-terminator is appended, and len is returned. 
         *  If len > count, count characters are stored in buffer, no null-terminator
         *  is appended, and a negative value is returned.*/
        if (len >= count) {
            stream[count - 1] = L'\0';
            len = count - 1;
        }
        return len;
    }

#endif

    template<typename ...Args> 
    inline
    int snprintf (char* stream, size_t count, const char* format, Args... args) {
        size_t len = ::snprintf(stream, count, format, std::forward<Args>(args)...);
        /** The snprintf function truncates the output when len is greater than or 
         * equal to count, by placing a null-terminator at buffer[count-1]. The value 
         * returned is len, the number of characters that would have been output if 
         * count was large enough. */
        if(len >= count) {
            CropIncompleteUTF8(stream);
            len = ::strlen(stream);
        }
        return len;
    }

    template<typename ChartT, size_t count, typename ...Args> 
    inline
    int snprintf(ChartT (&stream)[count], const ChartT* format, Args... args) {
        return lk::snprintf(stream, count, format, std::forward<Args>(args)...);
    }

} // lk

#endif // _UTILS_PRINTF_H_
