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
#include "OS/Sleep.h"
#include "Android/BluetoothHelper.hpp"
#include "Android/PortBridge.hpp"

namespace {

enum PortState : int {
  STATE_READY = 0,
  STATE_FAILED = 1,
  STATE_LIMBO = 2,
};

}  // namespace

bool BluetoothSensor::Initialize() {
  try {
    JNIEnv* env = Java::GetEnv();
    if (env && BluetoothHelper::isEnabled(env)) {
      bridge = BluetoothHelper::connectSensor(env, GetPortName());
      if (bridge) {
        bridge->setListener(env, this);
        bridge->setInputListener(env, this);
        return true;
      }
    }
  } catch (const std::exception& e) {
    delete std::exchange(bridge, nullptr);  // required if `setInputListener` or `setListener` throw exception
    const tstring what = to_tstring(e.what());
    StartupStore(_T("FAILED! <%s>"), what.c_str());
  }
  StatusMessage(_T("%s %s"), MsgToken<762>(), GetPortName());
  return false;
}

bool BluetoothSensor::Close() {
  PortBridge* delete_bridge = WithLock(mutex, [&]() {
    running = false;
    return std::exchange(bridge, nullptr);
  });

  while (!ComPort::Close()) {
    Sleep(10);
  }

  delete delete_bridge;

  return true;
}

bool BluetoothSensor::StopRxThread() {
  WithLock(mutex, [&]() { running = false; });

  if (ComPort::StopRxThread()) {
    return true;
  }
  return false;
}

bool BluetoothSensor::StartRxThread() {
  ScopeLock lock(mutex);
  running = true;

  return ComPort::StartRxThread();
}

void BluetoothSensor::CancelWaitEvent() {
  newdata.Broadcast();
}

bool BluetoothSensor::IsReady() {
  ScopeLock lock(mutex);
  if (bridge) {
    return bridge->getState(Java::GetEnv()) == STATE_READY;
  }
  return false;
}

unsigned BluetoothSensor::RxThread() {
  int state = STATE_LIMBO;

  bool connected = false;

  std::vector<sensor_data> rxthread_queue;

  do {
    bool stop = WithLock(mutex, [&]() {
      if (running && bridge) {
        state = bridge->getState(Java::GetEnv());
        assert(rxthread_queue.empty());
        std::swap(rxthread_queue, data_queue);
        assert(data_queue.empty());
        return false;
      }
      return true;  // Stop RxThread...
    });

    if (stop) {
      return 0;  // Stop RxThread...
    }

    if (!connected && state == STATE_READY) {
      connected = true;
      devOpen(devGetDeviceOnPort(GetPortIndex()));
    }

    if (connected && state != STATE_READY) {
      connected = false;
      auto name = WithLock(mutex, [&]() {
        return device_name;
      });
      StatusMessage("%s disconnected", name.c_str());
    }

    for (auto& data : rxthread_queue) {
      ProcessSensorData(data);
    }
    rxthread_queue.clear();

    ScopeLock lock(mutex);
    newdata.Wait(mutex);  // wait for data or state change
  } while (true);
}

void BluetoothSensor::PortStateChanged() {
  newdata.Signal();
}

void BluetoothSensor::PortError(const char* msg) {
  StartupStore("BluetoothSensor Error : %s", msg);
}

void BluetoothSensor::OnCharacteristicChanged(uuid_t service, uuid_t characteristic, const void* data, size_t size) {
  auto first = static_cast<const uint8_t*>(data);
  auto last = std::next(first, size);

  ScopeLock lock(mutex);
  data_queue.emplace_back(service, characteristic, std::vector<uint8_t>(first, last));
  newdata.Signal();
}

const BluetoothSensor::service_table_t& BluetoothSensor::service_table() {
  static const service_table_t table = {{
    { "0000180D-0000-1000-8000-00805F9B34FB", {{ // Heart Rate
        { "00002A37-0000-1000-8000-00805F9B34FB", {
            &BluetoothSensor::HeartRateMeasurement,
            &BluetoothSensor::Enable<&DeviceDescriptor_t::OnHeartRate>,
        }}
    }}},
    { "0000181A-0000-1000-8000-00805F9B34FB", { { // Environmental Sensing Service
        { "00002A6D-0000-1000-8000-00805F9B34FB", {
            &BluetoothSensor::BarometricPressure,
            &BluetoothSensor::Enable<&DeviceDescriptor_t::OnBarometricPressure>,
        }},
        { "00002A6E-0000-1000-8000-00805F9B34FB", {
            &BluetoothSensor::OutsideTemperature,
            &BluetoothSensor::Enable<&DeviceDescriptor_t::OnOutsideTemperature>,
        }},
    }}},
    { "0000FFE0-0000-1000-8000-00805F9B34FB", {{ // HM-10 and compatible bluetooth modules
        { "0000FFE1-0000-1000-8000-00805F9B34FB", {
            &BluetoothSensor::Hm10Data,
            &BluetoothSensor::Hm10DataEnable
        }}
    }}},
    { "00001800-0000-1000-8000-00805F9B34FB", {{ // Generic Access
        { "00002A00-0000-1000-8000-00805F9B34FB", {
            &BluetoothSensor::DeviceName,
            &BluetoothSensor::DeviceNameEnable
        }}
    }}},
    { "0000180F-0000-1000-8000-00805F9B34FB", {{ // Battery Service
        { "00002A19-0000-1000-8000-00805F9B34FB", { // Battery Level
            &BluetoothSensor::BatteryLevel,
            &BluetoothSensor::Enable<&DeviceDescriptor_t::OnBatteryLevel>
        }}
    }}}
  }};
  return table;
}

bool BluetoothSensor::DoEnableNotification(uuid_t service, uuid_t characteristic) const {
  auto it_service = service_table().find(service);
  if (it_service != service_table().end()) {
    auto it_characteristic = it_service->second.find(characteristic);
    if (it_characteristic != it_service->second.end()) {
      return std::invoke(it_characteristic->second.enable_handler, this);
    }
  }

  ScopeLock lock(CritSec_Comm);
  auto port = devGetDeviceOnPort(GetPortIndex());
  if (port && port->EnableGattCharacteristic) {
    return port->EnableGattCharacteristic(*port, service, characteristic);
  }
  return false;
}

void BluetoothSensor::ProcessSensorData(const sensor_data& data) {
  WithLock(CritSec_Comm, [&]() {
    auto port = devGetDeviceOnPort(GetPortIndex());
    if (port) {
      port->HB = LKHearthBeats;
      AddStatRx(data.data.size());
    }
  });

  auto it_service = service_table().find(data.service);
  if (it_service != service_table().end()) {
    auto it_characteristic = it_service->second.find(data.characteristic);
    if (it_characteristic != it_service->second.end()) {
      return std::invoke(it_characteristic->second.handler, this, data.data);
    }
  }
  OnSensorData<&DeviceDescriptor_t::OnGattCharacteristic>(data.service, data.characteristic, data.data);
}

void BluetoothSensor::DeviceName(const std::vector<uint8_t>& data) {
  std::string name(data.begin(), data.end());
  WithLock(mutex, [&]() {
    device_name = name;
  });
  StatusMessage("%s connected", name.c_str());
}

bool BluetoothSensor::DeviceNameEnable() const {
  return true;
}

void BluetoothSensor::BatteryLevel(const std::vector<uint8_t>& data) {
  if (data.size() == 1) {
    OnSensorData<&DeviceDescriptor_t::OnBatteryLevel, double>(data[0]);
  }
}

void BluetoothSensor::HeartRateMeasurement(const std::vector<uint8_t>& data) {
  if (data.size() >= 2) {
    uint32_t bpm = data[1];
    if (data[0] & 0x01 && data.size() >= 2) {
      bpm += (static_cast<uint32_t>(data[2]) << 8);
    }
    OnSensorData<&DeviceDescriptor_t::OnHeartRate>(bpm);
  }
}

void BluetoothSensor::BarometricPressure(const std::vector<uint8_t>& data) {
  if (data.size() != 4) {
    return;
  }
  uint32_t value = FromLE32(*reinterpret_cast<const uint32_t*>(data.data()));
  OnSensorData<&DeviceDescriptor_t::OnBarometricPressure>(value / 10.);
}

void BluetoothSensor::OutsideTemperature(const std::vector<uint8_t>& data) {
  if (data.size() != 2) {
    return;
  }
  uint16_t value = FromLE16(*reinterpret_cast<const uint16_t *>(data.data()));
  if (value == 0x8000) {
    return;
  }
  OnSensorData<&DeviceDescriptor_t::OnOutsideTemperature>(static_cast<int16_t>(value) / 100.);
}

void BluetoothSensor::DataReceived(const void *data, size_t length) {
  const auto *src_data = static_cast<const char *>(data);
  std::for_each_n(src_data, length, [this](auto c) {
    ProcessChar(static_cast<char>(c));
  });
}

bool BluetoothSensor::Hm10DataEnable() const {
  // TODO: allow to disable this ?
  return true;
}

bool BluetoothSensor::Write_Impl(const void *data, size_t size) {
  if(bridge) {
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
  return false;
}

void BluetoothSensor::WriteGattCharacteristic(uuid_t service, uuid_t characteristic, const void *data, size_t size) {
  if(bridge) {
    bridge->writeGattCharacteristic(Java::GetEnv(), service, characteristic, data, size);
  }
}

void BluetoothSensor::ReadGattCharacteristic(uuid_t service, uuid_t characteristic) {
  if(bridge) {
    bridge->readGattCharacteristic(Java::GetEnv(), service, characteristic);
  }
}
