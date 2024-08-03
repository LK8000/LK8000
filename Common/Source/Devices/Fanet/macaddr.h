/*
 * macaddr.h
 *
 *  Created on: Sep 29, 2018
 *      Author: sid
 */

#ifndef FANET_RADIO_MACADDR_H_
#define FANET_RADIO_MACADDR_H_

#include "payload.h"

/*
 * 0, 0 == Broadcast
 */

class MacAddr {
 public:
  uint8_t manufacturer = 0U;
  uint16_t id = 0U;

  constexpr MacAddr(int manufacturer_addr, int id_addr) 
          : manufacturer(manufacturer_addr), id(id_addr) {}

  constexpr MacAddr(int manufacturer_addr, int id_msb, int id_lsb)
          : manufacturer(manufacturer_addr), id(id_lsb | (id_msb << 8)) {}

  constexpr explicit MacAddr(uint32_t addr)
          : manufacturer(addr & 0x000000FF), id((addr & 0x00FFFF00) >> 8) {}

  MacAddr() = default;  // broadcast address

  constexpr MacAddr(const MacAddr& ma) = default;
  MacAddr& operator= (const MacAddr& ma) = default;

  constexpr MacAddr(MacAddr&& ma) = default;
  MacAddr& operator= (MacAddr&& ma) = default;

  void serialize(payload_t::back_insert_iterator& out) const {
    out = manufacturer;
    out = id & 0x00FF;
    out = (id >> 8) & 0x00FF;
  }

  constexpr uint32_t get() const {
    return (manufacturer & 0x000000FF) | ((id & 0x0000FFFF) << 8);
  }

  bool operator==(const MacAddr& rhs) const {
    return ((id == rhs.id) && (manufacturer == rhs.manufacturer));
  }

  bool operator!=(const MacAddr& rhs) const {
    return ((id != rhs.id) || (manufacturer != rhs.manufacturer));
  }
};

#endif /* FANET_RADIO_MACADDR_H_ */
