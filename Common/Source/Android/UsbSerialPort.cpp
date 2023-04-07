//
// Created by bruno on 5/12/17.
//

#include "UsbSerialPort.h"
#include "Android/PortBridge.hpp"
#include "Android/UsbSerialHelper.h"

PortBridge* UsbSerialPort::CreateBridge() {

  JNIEnv *env = Java::GetEnv();
  if (env && UsbSerialHelper::isEnabled(env)) {
    return UsbSerialHelper::connectDevice(env, GetPortName(), _baud);
  }

  return nullptr;
}