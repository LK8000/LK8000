/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   device_priority_index.h
 */

#ifndef _NMEA_device_priority_index_h_
#define _NMEA_device_priority_index_h_
#include "Comm/DeviceDescriptor.h"

struct default_device_priority_index final {
 public:
  default_device_priority_index() = default;

  default_device_priority_index& operator=(const DeviceDescriptor_t& d) {
    _index = d.PortNumber;
    return *this;
  }

  bool operator>=(const DeviceDescriptor_t& d) const {
    return _index >= d.PortNumber;
  }

  bool operator==(unsigned idx) const {
    return _index == idx;
  }

  bool operator!=(unsigned idx) const {
    return _index != idx;
  }

  void reset() {
    _index = NUMDEV;
  }

  bool valid() const {
    return _index < NUMDEV;
  }

  operator unsigned() const {
    return _index;
  }

 private:
  unsigned _index = NUMDEV;
};

#endif  // _NMEA_device_priority_index_h_
