/*
 * macaddr.h
 *
 *  Created on: Sep 29, 2018
 *      Author: sid
 */

#ifndef FANET_RADIO_MACADDR_H_
#define FANET_RADIO_MACADDR_H_


/*
 * 0, 0 == Broadcast
 */

class MacAddr {
 public:
  uint8_t manufacturer = 0U;
  uint16_t id = 0U;

  MacAddr(int manufacturer_addr, int id_addr) : manufacturer(manufacturer_addr), id(id_addr) {};
  MacAddr() = default;  // broadcast address

  MacAddr(const MacAddr& ma) = default;
  MacAddr& operator= (const MacAddr& ma) = default;

  MacAddr(MacAddr&& ma) = default;
  MacAddr& operator= (MacAddr&& ma) = default;

  uint32_t get() const {
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
