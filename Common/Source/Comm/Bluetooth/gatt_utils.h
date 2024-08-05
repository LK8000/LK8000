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

#include "utils/uuid.h"

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

static_assert(gatt_uuid(0x1234) == uuid_t("00001234-0000-1000-8000-00805F9B34FB"), "gatt_uuid() failed.");

} // bluetooth

#endif // _Comm_Bluetooth_gatt_utils_h_
