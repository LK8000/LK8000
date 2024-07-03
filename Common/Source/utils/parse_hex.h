// Copyright (c) 2024, Bruno de Lacheisserie
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

#ifndef _utils_parse_hex_h_
#define _utils_parse_hex_h_

#include <cstdint>
#include <stdexcept>

namespace hex {

constexpr
uint8_t digit(char c) {
  return ('0' <= c && c <= '9')   ? c - '0'
         : ('a' <= c && c <= 'f') ? 10 + c - 'a'
         : ('A' <= c && c <= 'F') ? 10 + c - 'A'
                                  : throw std::domain_error("invalid hex digit");
}

constexpr
uint8_t to_uint8_t(const char* ptr) {
  return (digit(ptr[0]) << 4) | digit(ptr[1]);
}

constexpr
uint16_t to_uint16_t(const char* ptr) {
  return (to_uint8_t(ptr) << 8) | to_uint8_t(ptr + 2);
}

constexpr
uint32_t to_uint32_t(const char* ptr) {
  return (to_uint16_t(ptr) << 16) | to_uint16_t(ptr + 4);
}

} // hex

#endif  // _utils_parse_hex_h_
