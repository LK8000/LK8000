/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   BluetoothSensor.h
 * Author: Bruno de Lacheisserie
 */

#ifndef ANDROID_BLUETOOTHSENSOR_H
#define ANDROID_BLUETOOTHSENSOR_H

#include "Thread/Mutex.hpp"
#include "Comm/Bluetooth/GattSensor.h"

class PortBridge;

class BluetoothSensor : public GattSensor {
 public:
  using GattSensor::GattSensor;

  /* override GattSensor transport */
 protected:
  bool Connect() override;
  void Disconnect() override;
  PortState GetPortState() const override;
  bool WriteData(const void* data, size_t size) override;
  void WriteGattCharacteristic(const uuid_t& service, const uuid_t& characteristic, const void* data, size_t size) const override;
  void ReadGattCharacteristic(const uuid_t& service, const uuid_t& characteristic) override;

 private:
  mutable Mutex mutex;
  PortBridge* bridge = nullptr;
};

#endif  // ANDROID_BLUETOOTHSENSOR_H
