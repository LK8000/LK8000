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

#include <bitset>

class airspace_mode {
 public:
  airspace_mode() = default;
  airspace_mode(const char* v) : _value(strtoul(v, nullptr, 10)) {}

  bool warning() const {
    return _value.test(_warning);
  }

  bool display() const {
    return _value.test(_display);
  }

  void reset() {
    // Default airspace state: display + warning enabled.
    _value.set();
  }

  void operator=(const char* v) {
    (*this) = airspace_mode(v);
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

  bool operator==(const airspace_mode& other) const {
    return _value == other._value;
  }

 private:
  using value_type = std::bitset<2>;
  value_type _value = ~value_type(0);  // default state: display + warning enabled

  constexpr static size_t _warning = 1U;
  constexpr static size_t _display = 0U;
};


#endif  // _AIRSPACE_AIRSPACEMODE_H_
