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

#include "options.h"
#include "Compiler.h"
#include "printf.h"
#include <iterator>
#include <cstring>

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

TEST_CASE("testing the lk::snprintf function") {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"        


    const char* input = "κόσμε";

    SUBCASE("utf8 no truncate") {
        char out[12];
        lk::snprintf(out, "%s", input);
        CHECK(ValidateUTF8(out));
        CHECK(strlen(out) < std::size(out));
        CHECK(strcmp(input, out) == 0);
    }

    SUBCASE("utf8 truncate") {
        char out[11];
        lk::snprintf(out, "%s", input);
        CHECK(ValidateUTF8(out));
        CHECK(strlen(out) < std::size(out));
        CHECK(strcmp("κόσμ", out) == 0);
    }

#ifdef UNICODE
    const wchar_t* winput = L"κόσμε";

    SUBCASE("unicode no truncate") {
        wchar_t out[6];
        lk::snprintf(out, L"%s", winput);
        CHECK(wcslen(out) < std::size(out));
        CHECK(wcscmp(winput, out) == 0);
    }

    SUBCASE("unicode truncate") {
        wchar_t out[5];
        lk::snprintf(out, L"%s", winput);
        CHECK(wcslen(out) < std::size(out));
        CHECK(wcscmp(L"κόσμ", out) == 0);
    }
#endif

#pragma GCC diagnostic pop        

}

#endif
