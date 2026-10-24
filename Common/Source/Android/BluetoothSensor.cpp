/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   BluetoothSensor.cpp
 * Author: Bruno de Lacheisserie
 */

#include "externs.h"
#include "BluetoothSensor.h"
#include "Android/BluetoothHelper.hpp"
#include "Android/PortBridge.hpp"

#include <utility>

bool BluetoothSensor::Connect() {
  JNIEnv* env = Java::GetEnv();
  if (env && BluetoothHelper::isEnabled(env)) {
    PortBridge* new_bridge = BluetoothHelper::connectSensor(env, GetPortName());
    if (new_bridge) {
      // Assign before setListener()/setInputListener(), which may throw: on
      // exception, GattSensor::Initialize() calls Disconnect(), which must
      // find `bridge` already set in order to free it.
      WithLock(mutex, [&]() { bridge = new_bridge; });
      new_bridge->setListener(env, this);
      new_bridge->setInputListener(env, this);
      return true;
    }
  }
  return false;
}

void BluetoothSensor::Disconnect() {
  PortBridge* old_bridge = WithLock(mutex, [&]() {
    return std::exchange(bridge, nullptr);
  });
  delete old_bridge;
}

GattSensor::PortState BluetoothSensor::GetPortState() const {
  const std::lock_guard lock(mutex);
  if (bridge) {
    return static_cast<PortState>(bridge->getState(Java::GetEnv()));
  }
  return PortState::LIMBO;
}

bool BluetoothSensor::WriteData(const void* data, size_t size) {
  const std::lock_guard lock(mutex);
  if (!bridge) {
    return false;
  }
  const char *p = (const char *)data;
  const char *end = p + size;

  while (p < end) {
    int nbytes = bridge->write(Java::GetEnv(), p, end - p);
    if (nbytes <= 0) {
      return false;
    }
    AddStatTx(nbytes);

    p += nbytes;
  }
  return true;
}

void BluetoothSensor::WriteGattCharacteristic(const uuid_t& service, const uuid_t& characteristic, const void *data, size_t size) const {
  const std::lock_guard lock(mutex);
  if (bridge) {
    bridge->writeGattCharacteristic(Java::GetEnv(), service, characteristic, data, size);
  }
}

void BluetoothSensor::ReadGattCharacteristic(const uuid_t& service, const uuid_t& characteristic) {
  const std::lock_guard lock(mutex);
  if (bridge) {
    bridge->readGattCharacteristic(Java::GetEnv(), service, characteristic);
  }
}
