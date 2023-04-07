/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   IOIOUartPort.h
 * Author: Bruno de Lacheisserie
 */

#ifndef ANDROID_USBSERIALPORT_H
#define ANDROID_USBSERIALPORT_H

#include "AndroidPort.h"

class UsbSerialPort  : public AndroidPort {
public:
  UsbSerialPort(int idx, const char* szName, unsigned baud, BitIndex_t BitSize) : AndroidPort(idx, szName), _baud(baud), _bit(BitSize) { }

protected:
  PortBridge* CreateBridge() override;

  const unsigned _baud;
  BitIndex_t _bit;
};

#endif //ANDROID_USBSERIALPORT_H
