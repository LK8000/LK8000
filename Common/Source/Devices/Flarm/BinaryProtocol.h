#ifndef Devices_Flarm_BinaryProtocol_h__
#define Devices_Flarm_BinaryProtocol_h__

#include <cstdint>
#include <vector>
#include "Comm/DeviceDescriptor.h"
#include "ComPort.h"

namespace Flarm {

// byte ID
constexpr uint8_t STARTFRAME = 0x73;
constexpr uint8_t ESCAPE = 0x78;
constexpr uint8_t ESC_ESC = 0x55;
constexpr uint8_t ESC_START = 0x31;

constexpr uint8_t EOF_ = 0x1A;

// message ID
constexpr uint8_t ACK = 0xA0;
constexpr uint8_t NACK = 0xB7;
constexpr uint8_t PING = 0x01;
constexpr uint8_t SETBAUDRATE = 0x02;
constexpr uint8_t FLASHDOWNLOAD = 0x11;
constexpr uint8_t EXIT = 0x12;

constexpr uint8_t SELECTRECORD = 0x20;
constexpr uint8_t GETRECORDINFO = 0x21;
constexpr uint8_t GETIGCDATA = 0x22;

inline uint8_t highbyte(uint16_t a) {
  return (((a) >> 8) & 0xFF);
}

inline uint8_t lowbyte(uint16_t a) {
  return ((a) & 0xFF);
}

template <typename T>
struct crc_t;

template <>
struct crc_t<uint8_t> {
  static uint16_t update(uint16_t crc, uint8_t data) {
    crc = crc ^ ((uint16_t)data << 8);
    for (int i = 0; i < 8; i++) {
      if (crc & 0x8000) {
        crc = (crc << 1) ^ 0x1021;
      }
      else {
        crc <<= 1;
      }
    }
    return crc;
  }
};

template <>
struct crc_t<uint16_t> {
  static uint16_t update(uint16_t crc, uint16_t data) {
    crc = crc_t<uint8_t>::update(crc, lowbyte(data));
    crc = crc_t<uint8_t>::update(crc, highbyte(data));
    return crc;
  }
};

template <typename T>
uint16_t crc_update(uint16_t crc, T data) {
  return crc_t<T>::update(crc, data);
}

template <typename T>
uint16_t crc_update(uint16_t crc, const T* data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    crc = crc_t<T>::update(crc, data[i]);
  }
  return crc;
}

template <typename iterator, typename T>
struct escape_t;

template <typename iterator>
struct escape_t<iterator, uint8_t> {
  static void update(iterator& out, uint8_t data) {
    switch (data) {
      case ESCAPE:
        *out++ = ESCAPE;
        *out++ = ESC_ESC;
        break;
      case STARTFRAME:
        *out++ = ESCAPE;
        *out++ = ESC_START;
        break;
      default:
        *out++ = data;
        break;
    }
  }
};

template <typename iterator>
struct escape_t<iterator, uint16_t> {
  static void update(iterator& out, uint16_t data) {
    escape_t<iterator, uint8_t>::update(out, lowbyte(data));
    escape_t<iterator, uint8_t>::update(out, highbyte(data));
  }
};

template <typename iterator, typename T>
void escape_data(iterator& out, T data) {
  escape_t<iterator, T>::update(out, data);
}

template <typename iterator, typename T>
void escape_data(iterator& out, const T* data, size_t size) {
  if (data) {
    for (size_t i = 0; i < size; i++) {
      escape_data<iterator, T>(out, data[i]);
    }
  }
}

inline std::vector<uint8_t> BuildFrame(uint16_t SeqNo, uint8_t Type,
                                       uint8_t* payload,
                                       uint16_t payload_size) {
  uint16_t Length = payload_size + 8;

  uint16_t crc = 0;
  crc = crc_update(crc, Length);
  crc = crc_update(crc, uint8_t(1));  // Version
  crc = crc_update(crc, SeqNo);
  crc = crc_update(crc, Type);
  crc = crc_update(crc, payload, payload_size);

  std::vector<uint8_t> data;
  auto out_it = std::back_inserter(data);

  out_it = STARTFRAME;
  escape_data(out_it, Length);
  escape_data(out_it, uint8_t(1));  // Version
  escape_data(out_it, SeqNo);
  escape_data(out_it, Type);
  escape_data(out_it, crc);
  escape_data(out_it, payload, payload_size);

  return data;
}

inline void SendFrame(DeviceDescriptor_t* d, uint16_t SeqNo, uint8_t Type,
                      uint8_t* payload, uint16_t payload_size) {
  if (d && d->Com) {
    auto data = BuildFrame(SeqNo, Type, payload, payload_size);
    d->Com->Write(data.data(), data.size());
  }
}

inline void Ping(DeviceDescriptor_t* d, uint16_t SeqNo) {
  SendFrame(d, SeqNo, PING, nullptr, 0);
}

inline void SelectRecord(DeviceDescriptor_t* d, uint16_t SeqNo,
                         uint8_t RecordIndex) {
  SendFrame(d, SeqNo, SELECTRECORD, &RecordIndex, 1);
}

inline void GetRecordInfo(DeviceDescriptor_t* d, uint16_t SeqNo) {
  SendFrame(d, SeqNo, GETRECORDINFO, nullptr, 0);
}

inline void GetIGCData(DeviceDescriptor_t* d, uint16_t SeqNo) {
  SendFrame(d, SeqNo, GETIGCDATA, nullptr, 0);
}

inline void Exit(DeviceDescriptor_t* d, uint16_t SeqNo) {
  SendFrame(d, SeqNo, EXIT, nullptr, 0);
}

}  // namespace Flarm

#endif  // Devices_Flarm_BinaryProtocol_h__
