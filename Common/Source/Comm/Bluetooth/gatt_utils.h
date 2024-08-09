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

#include <unordered_map>
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

template<typename Type>
class service_table_t {
  using characteristic_t = std::unordered_map<uuid_t, Type, uuid_hash>;
  using service_t = std::unordered_map<uuid_t, characteristic_t, uuid_hash>;
  using value_type = typename service_t::value_type;

 public:
  service_table_t(std::initializer_list<value_type> list) : table(list) {}

  const Type* get(const uuid_t& service, const uuid_t& characteristic) const {
    auto it_service = table.find(service);
    if (it_service != table.end()) {
      auto it_characteristic = it_service->second.find(characteristic);
      if (it_characteristic != it_service->second.end()) {
        return &it_characteristic->second;
      }
    }
    return nullptr;
  }

 private:
  service_t table;
};


} // bluetooth

#endif // _Comm_Bluetooth_gatt_utils_h_
