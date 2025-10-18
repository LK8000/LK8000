/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   from_device_data.h
 */

#ifndef _NMEA_from_device_data_h_
#define _NMEA_from_device_data_h_
#include "device_priority_index.h"

/**
 * This class stores a value associated with a device, updating only if the new
 * data come from device with higher or equal priority (as determined by the
 * IndexT type).
 *
 * @tparam ValueT The type of the value to store.
 * @tparam IndexT The type used for device priority indexing (default:
 * default_device_priority_index).
 */
template <typename ValueT, typename IndexT = default_device_priority_index>
class from_device_data final {
 public:
  from_device_data() = default;

  bool update(const DeviceDescriptor_t& d, ValueT value) {
    if (_index >= d) {
      _index = d;
      _value = std::forward<ValueT>(value);
      _last_update.Update();
      return true;
    }
    return false;
  }

  /**
   * Checks whether the specified duration has passed since the last
   * update.
   *
   * @param duration the duration in milliseconds
   */
  bool check_expired(unsigned duration) const {
    return _last_update.Check(duration);
  }

  void reset(std::optional<unsigned> idx = {}) {
    if (!idx || (_index == idx.value())) {
      _index.reset();
      _value = {};
      _last_update.Reset();
    }
  }

  bool available() const {
    return _index.valid();
  }

  const ValueT& value() const {
    return _value;
  }

  const IndexT& index() const {
    return _index;
  }

  explicit operator ValueT() const {
    return _value;
  }

 private:
  IndexT _index = {};
  ValueT _value = {};
  PeriodClock _last_update; // to check time elapsed since last value updated
};

#endif  // _NMEA_from_device_data_h_
