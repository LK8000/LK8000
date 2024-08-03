/*
 * frame.cpp
 *
 *  Created on: Sep 29, 2018
 *      Author: sid
 */
#include <cstring>
#include <cmath>
#include <stdexcept>
#include "Util/Clamp.hpp"
#include "frame.h"
#include "macaddr.h"

/*
 * Frame
 */

uint16_t Frame::coord2payload_compressed(double deg) {
  double deg_round = std::round(deg);
  bool deg_odd = static_cast<int>(deg_round) & 1;
  const double decimal = deg - deg_round;
  const int dec_int = Clamp<int>((decimal * 32767), -16383, 16383);

  return ((dec_int & 0x7FFF) | (deg_odd << 15));
}

void Frame::coord2payload_absolut(double lat, double lon, payload_t::back_insert_iterator& out) {
  int32_t lat_i = std::round(lat * 93206.0);
  int32_t lon_i = std::round(lon * 46603.0);

  out = ((uint8_t*)&lat_i)[0];
  out = ((uint8_t*)&lat_i)[1];
  out = ((uint8_t*)&lat_i)[2];

  out = ((uint8_t*)&lon_i)[0];
  out = ((uint8_t*)&lon_i)[1];
  out = ((uint8_t*)&lon_i)[2];
}

payload_t Frame::serialize() const {
  if (src.id <= 0 || src.id >= 0xFFFF || src.manufacturer <= 0 || src.manufacturer >= 0xFE) {
    throw std::runtime_error("invalid fanet id");
  }

  payload_t data;
  data.reserve(MAC_FRM_MIN_HEADER_LENGTH + payload.size());
  auto out_it = data.back_inserter();

  /* header */
  out_it = (ack_requested || dest.id != 0 || dest.manufacturer != 0 || signature != 0)
               << MAC_FRM_HEADER_EXTHEADER_BIT |
           forward << MAC_FRM_HEADER_FORWARD_BIT | (type & MAC_FRM_HEADER_TYPE_MASK);

  src.serialize(out_it);

  /* extended header */
  if (data[0] & 1 << 7)
    out_it = (ack_requested & 3) << MAC_FRM_EXTHEADER_ACK_BIT0 |
             (dest.id != 0 || dest.manufacturer != 0) << MAC_FRM_EXTHEADER_UNICAST_BIT |
             !!signature << MAC_FRM_EXTHEADER_SIGNATURE_BIT;

  /* extheader and unicast -> add destination addr */
  if ((data[0] & 1 << 7) && (data[4] & 1 << 5)) {
    dest.serialize(out_it);
  }

  /* extheader and signature -> add signature */
  if ((data[0] & 1 << 7) && (data[4] & 1 << 4)) {
    out_it = signature & 0x000000FF;
    out_it = (signature >> 8) & 0x000000FF;
    out_it = (signature >> 16) & 0x000000FF;
    out_it = (signature >> 24) & 0x000000FF;
  }

  /* fill payload */
  std::copy(payload.begin(), payload.end(), out_it);

  return data;
}

Frame::Frame(const uint8_t *data, size_t length) {
  int payload_start = MAC_FRM_MIN_HEADER_LENGTH;

  /* header */
  forward = !!(data[0] & (1 << MAC_FRM_HEADER_FORWARD_BIT));
  type = data[0] & MAC_FRM_HEADER_TYPE_MASK;

  src = { data[1], data[3], data[2] };

  /* extended header */
  if (data[0] & 1 << MAC_FRM_HEADER_EXTHEADER_BIT) {
    payload_start++;

    /* ack type */
    ack_requested = (data[4] >> MAC_FRM_EXTHEADER_ACK_BIT0) & 3;

    /* unicast */
    if (data[4] & (1 << MAC_FRM_EXTHEADER_UNICAST_BIT)) {
      dest = { data[5], data[7], data[6] };
      payload_start += MAC_FRM_ADDR_LENGTH;
    }

    /* signature */
    if (data[4] & (1 << MAC_FRM_EXTHEADER_SIGNATURE_BIT)) {
      signature = data[payload_start] | (data[payload_start + 1] << 8) | (data[payload_start + 2] << 16) |
                  (data[payload_start + 3] << 24);
      payload_start += MAC_FRM_SIGNATURE_LENGTH;
    }
  }

  /* payload */
  payload = { &data[payload_start], &data[length] };
}

bool Frame::operator== (const Frame& frm) const {
  if (src != frm.src)
    return false;

  if (dest != frm.dest)
    return false;

  if (type != frm.type)
    return false;

  if (payload != frm.payload)
    return false;

  return true;
}
