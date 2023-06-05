// Copyright (c) 2021, Bruno de Lacheisserie
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of  nor the names of its contributors may be used to
//    endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "options.h"
#include "base64.h"
#include <array>

namespace {

std::string encode(const char (&chars_table)[65], const uint8_t* bytes, size_t size, bool padding) {

	constexpr unsigned m1 = 63 << 18;
	constexpr unsigned m2 = 63 << 12;
	constexpr unsigned m3 = 63 << 6;

	std::string ret;
	auto p = std::back_inserter(ret);

	size_t a = 0;
	for (; size > 2; size -= 3 ) {
		uint32_t d = bytes[a++] << 16;
		d |= bytes[a++] << 8;
		d |= bytes[a++];
		
		p = chars_table[ ( d & m1 ) >> 18 ];
		p = chars_table[ ( d & m2 ) >> 12 ];
		p = chars_table[ ( d & m3 ) >>  6 ];
		p = chars_table[ d & 63 ];
	}
	if (size == 2 ) {
		uint32_t d = bytes[a++] << 16;
		d |= bytes[a++] << 8;
		
		p = chars_table[ ( d & m1 ) >> 18 ];
		p = chars_table[ ( d & m2 ) >> 12 ];
		p = chars_table[ ( d & m3 ) >>  6 ];
		if (padding) {
			p = '=';
		}
	}
	else if( size == 1 ) {
		uint32_t d = bytes[a++] << 16;
		
		p = chars_table[ ( d & m1 ) >> 18 ];
		p = chars_table[ ( d & m2 ) >> 12 ];
		if (padding) {
			p = '=';
			p = '=';
		}
	}
	return ret;
}

std::vector<uint8_t> decode(const std::array<uint8_t, 255>& reverse_table, const std::string_view& String) {
	std::vector<uint8_t> result;
	result.reserve((String.size() * 3) / 4);

	std::vector<uint8_t> quad;
	quad.reserve(4);
	for (auto c : String) {
		uint8_t b = reverse_table[c];
		if (b != 0xFF) { // ignore invalid chars ( <cr> <lf> '=' ...)
			quad.push_back(b);
		}
		if (quad.size() == 4) {
			result.push_back((quad[0] << 2) + ((quad[1] & 0x30) >> 4));
			result.push_back(((quad[1] & 0xf) << 4) + ((quad[2] & 0x3c) >> 2));
			result.push_back(((quad[2] & 0x3) << 6) + quad[3]);

			quad.clear();
		}
	}

	if (quad.size() > 1) {
		result.push_back((quad[0] << 2) + ((quad[1] & 0x30) >> 4));
	}
	if (quad.size() > 2) {
		result.push_back(((quad[1] & 0xf) << 4) + ((quad[2] & 0x3c) >> 2));
	}
	return result;
}

template <std::size_t ... Is>
constexpr std::array<uint8_t, sizeof...(Is)>
create_array(std::index_sequence<Is...>) {
	// cast Is to void to remove the warning: unused value
	return { {(static_cast<void>(Is), static_cast<uint8_t>(0xFF))...} };
}

constexpr std::array<uint8_t, 255> 
reverse_table(const char(&chars_table)[65]) {
	auto table = create_array(std::make_index_sequence<255>());
	for (uint8_t i = 0; i < 64U; i++) {
		table[chars_table[i]] = i;
	}
	return table;
}

constexpr char base64_chars[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";

constexpr auto base64_reverse_table = reverse_table(base64_chars);

constexpr char base64url_chars[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789-_";

constexpr auto base64url_reverse_table = reverse_table(base64url_chars);

} // namespace

std::string base64url_encode(const uint8_t* bytes, size_t size, bool padding) {
	return encode(base64url_chars, bytes, size, padding);
}

std::vector<uint8_t> base64url_decode(const std::string_view& String) {
	return decode(base64url_reverse_table, String);
}

std::string base64_encode(const uint8_t* bytes, size_t size, bool padding) {
	return encode(base64_chars, bytes, size, padding);
}

std::vector<uint8_t> base64_decode(const std::string_view& String) {
	return decode(base64_reverse_table, String);
}


#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

inline static 
std::vector<uint8_t> to_vector(const std::string_view& s) {
	return {s.begin(), s.end()};
}

std::string base64_encode(const std::string_view& s, bool padding = true) {
	return base64_encode(reinterpret_cast<const uint8_t*>(s.data()), s.size(), padding);
}

TEST_CASE("base64") {
	using namespace std::literals;

	SUBCASE("encode") {
		CHECK_EQ(base64_encode("Hello World"), "SGVsbG8gV29ybGQ="sv);
		CHECK_EQ(base64_encode("Hello World", false), "SGVsbG8gV29ybGQ"sv);

		CHECK_EQ(base64_encode(""), ""sv);
		CHECK_EQ(base64_encode("f"), "Zg=="sv);
		CHECK_EQ(base64_encode("fo"), "Zm8="sv);
		CHECK_EQ(base64_encode("foo"), "Zm9v"sv);
		CHECK_EQ(base64_encode("foob"), "Zm9vYg=="sv);
		CHECK_EQ(base64_encode("fooba"), "Zm9vYmE="sv);
		CHECK_EQ(base64_encode("foobar"), "Zm9vYmFy"sv);
	}

	SUBCASE("decode") {
		CHECK_EQ(base64_decode("SGVsbG8gV29ybGQ"sv), to_vector("Hello World"));
		CHECK_EQ(base64_decode("SGVsbG8gV29ybGQ="sv), to_vector("Hello World"));

		CHECK_EQ(base64_decode(""), to_vector(""));
		CHECK_EQ(base64_decode("Zg=="), to_vector("f"));
		CHECK_EQ(base64_decode("Zm8="), to_vector("fo"));
		CHECK_EQ(base64_decode("Zm9v"), to_vector("foo"));
		CHECK_EQ(base64_decode("Zm9vYg=="), to_vector("foob"));
		CHECK_EQ(base64_decode("Zm9vYmE="), to_vector("fooba"));
		CHECK_EQ(base64_decode("Zm9vYmFy"), to_vector("foobar"));

		CHECK_EQ(base64_decode("Zm9v\r\nYmFy"), to_vector("foobar"));
	}
}

#endif // DOCTEST_CONFIG_DISABLE
