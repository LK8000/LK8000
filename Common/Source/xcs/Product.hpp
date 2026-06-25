/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2015 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef XCSOAR_ANDROID_PRODUCT_HPP
#define XCSOAR_ANDROID_PRODUCT_HPP

#include "Compiler.h"
#include <string>
#include <cstdint>

#ifdef ANDROID
extern bool has_cursor_keys;
extern bool has_keyboard;
extern std::string android_unique_device_id;
#endif

#if defined(ANDROID) && (defined(__arm__) || defined(__aarch64__))
extern bool is_nook, is_dithered, is_eink_colored;
#endif

/**
 * Returns whether the application is running on Nook Simple Touch
 */
#ifdef __arm__
gcc_const
#else
constexpr
#endif
static inline bool
IsNookSimpleTouch()
{
#if defined(ANDROID) && defined(__arm__)
  return is_nook;
#else
  return false;
#endif
}

inline constexpr std::string_view GetDeviceIdAlphabet() {
  // The alphabet excludes visually similar characters (0, 1, O, I, l) to avoid user confusion 
  // when displaying device IDs.
  return "23456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
}

inline std::string EncodeDeviceId(uint64_t value) {
  constexpr auto alphabet = GetDeviceIdAlphabet();
  constexpr size_t alphabet_size = alphabet.size();
  if (value == 0) {
    // Invalid device ID, return empty string to indicate error
    return "";
  }
  std::string result;
  while (value > 0) {
    result.push_back(alphabet[value % alphabet_size]);
    value /= alphabet_size;
  }
  return result;
}

inline bool IsValidDeviceId(const std::string& id) {
  constexpr auto alphabet = GetDeviceIdAlphabet();
  if (id.empty()) {
    return false;
  }
  for (unsigned char c : id) {
    if (alphabet.find(c) == std::string_view::npos) {
      return false;
    }
  }
  return true;
}

const std::string& GetUniqueDeviceId();

#endif
