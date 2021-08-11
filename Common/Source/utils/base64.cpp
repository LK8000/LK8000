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

#include "base64.h"

namespace {

	constexpr char base64_chars[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	constexpr char base64url_chars[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789-_";


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

} // namespace

std::string base64url_encode(const uint8_t* bytes, size_t size, bool padding) {
	return encode(base64url_chars, bytes, size, padding);
}

std::string base64_encode(const uint8_t* bytes, size_t size, bool padding) {
	return encode(base64_chars, bytes, size, padding);
}
