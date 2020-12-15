/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
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
