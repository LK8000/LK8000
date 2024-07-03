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

#ifndef _utils_uuid_h_
#define _utils_uuid_h_

#include "parse_hex.h"
#include <array>
#include "OS/ByteOrder.hpp"

#ifdef uuid_t
// yhis is required to solve conflict on win32 platform
#undef uuid_t
#endif

/*
    RFC 9562 - Universally Unique IDentifiers (UUIDs)

    The formal definition of the UUID string representation is provided by
    the following ABNF:

    UUID     = 4hexOctet "-"
               2hexOctet "-"
               2hexOctet "-"
               2hexOctet "-"
               6hexOctet
    hexOctet = HEXDIG HEXDIG
    DIGIT    = %x30-39
    HEXDIG   = DIGIT / "A" / "B" / "C" / "D" / "E" / "F
 */

class uuid_t {
 public:
  uuid_t() = default;
  constexpr uuid_t(uuid_t&&) = default;
  constexpr uuid_t(const uuid_t&) = default;

  constexpr uuid_t(const char* s) 
      : uuid_t(uuid_msb(s), uuid_lsb(s)) {}

  constexpr uuid_t(uint64_t msb, uint64_t lsb)
      : _msb(msb),
        _lsb(lsb)
  {}

  bool operator==(uuid_t uuid) const {
    return uuid._msb == _msb && uuid._lsb == _lsb;
  }

  size_t hash() const {
      return std::hash<uint64_t>{}(_msb) ^ std::hash<uint64_t>{}(_lsb);
  }

 private:
  uint64_t _msb;
  uint64_t _lsb;

  constexpr static
  uint64_t uuid_msb(const char* str) {
    uint64_t msb = hex::to_uint32_t(str);  // 8 digit
    msb <<= 16;
    msb |= hex::to_uint16_t(str + 9);  // 4 digit
    msb <<= 16;
    msb |= hex::to_uint16_t(str + 14);  // 4 digit
    return msb;
  }

  constexpr static
  uint64_t uuid_lsb(const char* str) {
    uint64_t lsb = hex::to_uint16_t(str + 19);  // 4 digit
    lsb <<= 16;
    lsb |= hex::to_uint16_t(str + 24);  // 4 digit
    lsb <<= 32;
    lsb |= hex::to_uint32_t(str + 28);  // 8 digit
    return lsb;
  }
};

struct uuid_hash {
  size_t operator()(uuid_t uuid) const {
    return uuid.hash();
  }
};

#endif  // _utils_uuid_h_
