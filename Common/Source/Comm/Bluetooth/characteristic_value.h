/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   characteristic_value.h
 * Author: Bruno de Lacheisserie
 */
#ifndef _Comm_Bluetooth_characteristic_value_h_
#define _Comm_Bluetooth_characteristic_value_h_
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <utility>
#include "OS/ByteOrder.hpp"

template <typename value_type,
          typename unsigned_value_type = std::make_unsigned_t<value_type>,
          size_t value_size = sizeof(unsigned_value_type)>
class characteristic_value {
 public:
  characteristic_value(const std::vector<uint8_t>& value) : _value(value) {}

  value_type get(size_t offset = 0U) const {
    auto value = to_native(offset);
    // Sign-extend if value_type is signed and value_size < sizeof(value_type)
    if constexpr (sizeof(unsigned_value_type) > value_size &&
                  std::is_signed_v<value_type>) {
      const unsigned_value_type sign_bit = static_cast<unsigned_value_type>(1)
                                           << (value_size * 8 - 1);
      if (value & sign_bit) {
        value |= (~static_cast<unsigned_value_type>(0)) << (value_size * 8);
      }
    }
    return static_cast<value_type>(value);
  }

 private:
  unsigned_value_type to_native(size_t offset) const {
    if (_value.size() < offset + value_size) {
      throw std::out_of_range("not enough bytes to read value");
    }
    if constexpr (IsLittleEndian()) {
      return to_little(offset, std::make_index_sequence<value_size>{});
    }
    else {
      return to_big(offset, std::make_index_sequence<value_size>{});
    }
  }

  template <std::size_t... I>
  constexpr unsigned_value_type to_little(std::size_t offset,
                                          std::index_sequence<I...>) const {
    return (
        ((static_cast<unsigned_value_type>(_value[offset + I]) << (8 * I))) |
        ...);
  }

  template <std::size_t... I>
  constexpr unsigned_value_type to_big(std::size_t offset,
                                       std::index_sequence<I...>) const {
    return (((static_cast<unsigned_value_type>(_value[offset + I])
              << (8 * (sizeof(unsigned_value_type) - 1 - I)))) |
            ...);
  }

  const std::vector<uint8_t>& _value;
};

#endif  // _Comm_Bluetooth_characteristic_value_h_
