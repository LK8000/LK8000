/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   UTF16.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 13 December 2020, 00:12
 */

#include "UTF16.h"
#include <cassert>

namespace {

	constexpr inline bool IsHighSurrogate(uint16_t c) {
		return (c & 0xFC00) == 0xD800;
	}

	constexpr inline bool IsLowSurrogate(uint16_t c) {
		return (c & 0xFC00) == 0xDC00;
	}

} // namespace

std::pair<unsigned, const uint16_t *>
NextUTF16(const uint16_t *p) {
	if (!(*p)) {
		return std::make_pair(0U, nullptr);
	}

	unsigned u = *p++;

	if (IsLowSurrogate(u)) {
		assert(false); // can't start at low surrogate.
		return std::make_pair(0U, nullptr);
	}

	if (IsHighSurrogate(u)) {
		if (!(*p)) {
			assert(false); // truncated string.
			return std::make_pair(0U, nullptr);
		}

		unsigned low = *p++;
		if (!IsLowSurrogate(low)) {
			assert(false); // must continue with low surrogate.
			return std::make_pair(0U, nullptr);
		}

		u = 0x10000 + ((u & 0x03FF) << 10) + (low & 0x03FF);
	}

	return std::make_pair(u, p);
}


uint16_t *
UnicodeToUTF16(unsigned ch, uint16_t *q) {
	if (ch < 0x10000) {
		*q++ = ch;
	}
	else {
		ch -= 0x10000;
		*q++ = 0xD800 | ((ch >> 10) & 0x3FF);
		*q++ = 0xDC00 | (ch & 0x3FF);
	}
	return q;
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

TEST_CASE("UTF16") {

	constexpr uint16_t utf16[] = {
		0xD835, 0xDFF6,
		0x0020,
		0xD835, 0xDFF7,
		0x0020,
		0xD835, 0xDFF8,
		0
	};

	constexpr uint32_t utf32[] = { 
		0x0001D7F6, 
		0x00000020,
		0x0001D7F7, 
		0x00000020,
		0x0001D7F8, 
		0 
	};

	SUBCASE("NextUTF16") {
		auto next_utf32 = utf32;
		auto next_utf16 = NextUTF16(utf16);
		while(next_utf16.first) {
			CHECK(next_utf16.first == *(next_utf32++));
			next_utf16 = NextUTF16(next_utf16.second);
		}
	}

	SUBCASE("UnicodeToUTF16") {
		uint16_t utf16_pair[2] = {};
		UnicodeToUTF16(utf32[0], utf16_pair);
		CHECK_EQ(utf16_pair[0], utf16[0]);
		CHECK_EQ(utf16_pair[1], utf16[1]);

		utf16_pair[0] = 0; utf16_pair[1] = 0;
		UnicodeToUTF16(utf32[1], utf16_pair);
		CHECK_EQ(utf16_pair[0], utf16[2]);
		CHECK_EQ(utf16_pair[1], 0);
	}
}

#endif
