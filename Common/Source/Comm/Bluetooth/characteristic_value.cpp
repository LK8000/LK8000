/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   characteristic_value.cpp
 * Author: Bruno de Lacheisserie
 */
#include "options.h"
#include "characteristic_value.h"
#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

TEST_CASE("characteristic_value get() tests") {

    SUBCASE("unsigned 16-bit") {
        std::vector<uint8_t> bytes = {0x34, 0x12}; // 0x1234
        characteristic_value<uint16_t> cv(bytes);

        REQUIRE(cv.get() == 0x1234);
    }

    SUBCASE("signed 16-bit positive") {
        std::vector<uint8_t> bytes = {0x34, 0x12}; // 0x1234 = 4660
        characteristic_value<int16_t> cv(bytes);

        REQUIRE(cv.get() == 0x1234);
    }

    SUBCASE("signed 16-bit negative") {
        std::vector<uint8_t> bytes = {0xCC, 0xFF}; // 0xFFCC = -52 in int16
        characteristic_value<int16_t> cv(bytes);

        REQUIRE(cv.get() == -52);
    }

    SUBCASE("unsigned 32-bit positive") {
        // value = 0x12345678 in little-endian
        std::vector<uint8_t> bytes = { 0x78, 0x56, 0x34, 0x12 };
        characteristic_value<uint32_t> cv(bytes);

        REQUIRE(cv.get() == 0x12345678);
    }

    SUBCASE("signed 32-bit negative") {
        std::vector<uint8_t> bytes = {0x78, 0x56, 0x34, 0xF0}; // 0xF0345678 = negative
        characteristic_value<int32_t> cv(bytes);
        REQUIRE(cv.get() == static_cast<int32_t>(0xF0345678));
    }

    SUBCASE("Not enough bytes") {
        std::vector<uint8_t> bytes = { 0x12 };
        characteristic_value<uint16_t> cv(bytes);
        
        CHECK_THROWS_AS(cv.get(), std::out_of_range);
    }

    SUBCASE("Multiple values in one buffer") {
        // two 16-bit values: 0x1234, 0x5678
        std::vector<uint8_t> bytes = { 0x34, 0x12, 0x78, 0x56 };
        characteristic_value<uint16_t> cv(bytes);

        REQUIRE(cv.get(0) == 0x1234);
        REQUIRE(cv.get(2) == 0x5678);
    }

    SUBCASE("unsigned 24-bit positve") {
        // value = 0x123456 in little-endian
        std::vector<uint8_t> bytes = { 0x56, 0x34, 0x12 };
        characteristic_value<uint32_t, uint32_t, 3> cv(bytes);

        REQUIRE(cv.get() == 0x123456);
    }

    SUBCASE("signed 24-bit negative") {
        std::vector<uint8_t> bytes = {0xCC, 0xFF, 0xFF}; // 0xFFCC = -52 in int16
        characteristic_value<int32_t, uint32_t, 3> cv(bytes);

        REQUIRE(cv.get() == -52);
    }
}

#endif
