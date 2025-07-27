/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   airspace_mode.h
 * Author: Bruno de Lacheisserie
 */
#ifndef _AIRSPACE_AIRSPACEMODE_H_
#define _AIRSPACE_AIRSPACEMODE_H_

#include <array>
#include <bitset>
#include "Airspace.h"

class airspace_mode {
 public:

  bool warning() const {
    return _value.test(_warning);
  }

  bool display() const {
    return _value.test(_display);
  }

  void reset() {
    _value.set();
  }

  void operator=(const char* v) {
    _value = strtoul(v, nullptr, 10);
  }

  uint32_t to_unsigned() const {
    return _value.to_ulong();
  }

  void rotate_set() {
    if (_value.none()) { 
      _value.set();
    }
    else {
      _value >>= 1;
    }
  }

 private:
  std::bitset<2> _value;

  constexpr static size_t _warning = 1U;
  constexpr static size_t _display = 0U;
};

using airspace_mode_array = std::array<airspace_mode, AIRSPACECLASSCOUNT>;

#endif  // _AIRSPACE_AIRSPACEMODE_H_
