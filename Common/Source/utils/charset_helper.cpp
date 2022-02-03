/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   charset_helper.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 02 February 2022
 */
#include "options.h"
#include "Compiler.h"
#include "charset_helper.h"

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

TEST_CASE("to_tchar_string") {
    const std::string test_utf8("10€ƒ¶Æ");
    const std::wstring test_unicode(L"10€ƒ¶Æ");

  	SUBCASE("UTF8 -> UTF8") {
      std::string utf8 = from_unknown_charset("10€ƒ¶Æ");
      CHECK_EQ(utf8, test_utf8);
    }

  	SUBCASE("ANSI -> UTF8") {
      std::string utf8 = from_unknown_charset("10\x80\x83\xB6\xC6");
      CHECK_EQ(utf8, test_utf8);
    }

  	SUBCASE("UTF8 -> UNICODE") {
      std::wstring unicode = from_unknown_charset("10€ƒ¶Æ");
      CHECK_EQ(unicode, test_unicode);
    }

  	SUBCASE("ANSI -> UNICODE") {
      std::wstring unicode = from_unknown_charset("10\x80\x83\xB6\xC6");
      CHECK_EQ(unicode, test_unicode);
    }
}

#endif
