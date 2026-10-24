/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   gatt_utils.h
 * Author: Bruno de Lacheisserie
 */
#ifndef _Comm_Bluetooth_gatt_utils_h_
#define _Comm_Bluetooth_gatt_utils_h_

#include <vector>
#include "utils/uuid.h"

#undef uuid_t

namespace bluetooth {

/**
 * Converts a Bluetooth SIG defined short Id to a full GATT UUID.
 */
constexpr static
uuid_t gatt_uuid(uint16_t id) {
  constexpr uuid_t base_uuid = "00000000-0000-1000-8000-00805F9B34FB";
  uint64_t msb = (static_cast<uint64_t>(id) << 32) | base_uuid.msb();
  return { msb, base_uuid.lsb() };
}

static_assert(gatt_uuid(0x1234) == "00001234-0000-1000-8000-00805F9B34FB", "gatt_uuid() failed.");

struct gatt_sensor_data {
  gatt_sensor_data(uuid_t&& s, uuid_t&& c, std::vector<uint8_t>&& _data)
      : service(s), characteristic(c), data(std::move(_data)) {}

  uuid_t service;
  uuid_t characteristic;
  std::vector<uint8_t> data;
};

// Nordic UART Service (NUS) -- a de-facto standard raw-serial-over-BLE
// service used by many GPS/instrument modules (not Bluetooth SIG assigned,
// hence the full 128-bit UUIDs rather than gatt_uuid()). TX is notify-only,
// from the peripheral's perspective -- i.e. what this device (the central)
// receives data on; RX (6e400002) is peripheral-write-only and unused here,
// same as the reference gattlib NUS example this was matched against.
constexpr uuid_t NUS_SERVICE = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
constexpr uuid_t NUS_TX_CHARACTERISTIC = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";    
constexpr uuid_t NUS_RX_CHARACTERISTIC = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";


} // bluetooth

#endif // _Comm_Bluetooth_gatt_utils_h_
