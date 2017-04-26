//
// Created by bruno on 5/12/17.
//

#include "UsbSerialPort.h"
#include "Android/PortBridge.hpp"
#include "Android/UsbSerialHelper.h"

bool UsbSerialPort::CreateBridge() {

  JNIEnv *env = Java::GetEnv();
  if (env && UsbSerialHelper::isEnabled(env)) {
    bridge = UsbSerialHelper::connectDevice(Java::GetEnv(), GetPortName(), _baud);
  }

  return (bridge);
}