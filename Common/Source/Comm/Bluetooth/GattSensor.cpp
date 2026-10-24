/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GattSensor.cpp
 */

#include "externs.h"
#include "GattSensor.h"
#include "OS/Sleep.h"
#include "gatt_utils.h"
#include "characteristic_value.h"
#include "utils/charset_helper.h"

namespace {

using ProcessSensorDataT = std::function<void(GattSensor*, const std::vector<uint8_t>&)>;
using DoEnableNotificationT = std::function<bool(const GattSensor*)>;

struct DataHandlerT {
  ProcessSensorDataT ProcessSensorData;
  DoEnableNotificationT DoEnableNotification;
};

using service_table_t = bluetooth::service_table_t<DataHandlerT>;

const service_table_t& service_table() {
  using bluetooth::gatt_uuid;
  static const service_table_t table = {{
    { gatt_uuid(0x180D), {{ // Heart Rate
        { gatt_uuid(0x2A37), {
            &GattSensor::HeartRateMeasurement,
            &GattSensor::EnableCharacteristic<&DeviceDescriptor_t::OnHeartRate>,
        }}
    }}},
    { gatt_uuid(0x181A), {{ // Environmental Sensing Service
        { gatt_uuid(0x2A6D), {
            &GattSensor::BarometricPressure,
            &GattSensor::EnableCharacteristic<&DeviceDescriptor_t::OnBarometricPressure>,
        }},
        { gatt_uuid(0x2A6E), {
            &GattSensor::OutsideTemperature,
            &GattSensor::EnableCharacteristic<&DeviceDescriptor_t::OnOutsideTemperature>,
        }},
        { gatt_uuid(0x2A6F), {
            &GattSensor::RelativeHumidity,
            &GattSensor::EnableCharacteristic<&DeviceDescriptor_t::OnRelativeHumidity>,
        }},
        { gatt_uuid(0x2A70), {
            &GattSensor::WindSpeed,
            &GattSensor::EnableCharacteristic<&DeviceDescriptor_t::OnWindSpeed>,
        }},
        { gatt_uuid(0x2A71), {
            &GattSensor::WindOriginDirection,
            &GattSensor::EnableCharacteristic<&DeviceDescriptor_t::OnWindOriginDirection>,
        }},
    }}},
    { gatt_uuid(0xFFE0), {{ // HM-10 and compatible bluetooth modules
        { gatt_uuid(0xFFE1), {
            &GattSensor::Hm10Data,
            &GattSensor::Hm10DataEnable
        }},
        { gatt_uuid(0xFFE4), { // SkyDrop2
            &GattSensor::Hm10Data,
            &GattSensor::Hm10DataEnable
        }},
    }}},
    { gatt_uuid(0x1800), {{ // Generic Access
        { gatt_uuid(0x2A00), {
            &GattSensor::DeviceName,
            &GattSensor::EnableCharacteristic<true>
        }}
    }}},
    { gatt_uuid(0x180A), {{ // Device Information Service
        { gatt_uuid(0x2A25), { // Serial Number String
            &GattSensor::SerialNumber,
            &GattSensor::EnableCharacteristic<true>
        }}
    }}},
    { gatt_uuid(0x180F), {{ // Battery Service
        { gatt_uuid(0x2A19), { // Battery Level
            &GattSensor::BatteryLevel,
            &GattSensor::EnableCharacteristic<&DeviceDescriptor_t::OnBatteryLevel>
        }}
    }}},
  }};
  return table;
}

}  // namespace

bool GattSensor::Initialize() {
  try {
    if (Connect()) {
      return true;
    }
  } catch (const std::exception& e) {
    Disconnect();
    const tstring what = to_tstring(e.what());
    StartupStore(_T("FAILED! <%s>"), what.c_str());
  }
  StatusMessage(_T("{} {}"), MsgToken<762>(), GetPortName());
  return false;
}

bool GattSensor::Close() {
  WithLock(mutex, [&]() {
    running = false;
  });

  while (!ComPort::Close()) {
    Sleep(10);
  }

  Disconnect();

  return true;
}

bool GattSensor::StopRxThread() {
  WithLock(mutex, [&]() {
    running = false;
  });

  if (ComPort::StopRxThread()) {
    return true;
  }
  return false;
}

bool GattSensor::StartRxThread() {
  const std::lock_guard lock(mutex);
  running = true;

  return ComPort::StartRxThread();
}

void GattSensor::CancelWaitEvent() {
  newdata.notify_all();
}

bool GattSensor::IsReady() {
  return GetPortState() == PortState::READY;
}

unsigned GattSensor::RxThread() {
  unsigned observed_state_generation = WithLock(mutex, [&]() {
    // Intentional unsigned wraparound when state_generation==0:
    // this guarantees one initial state poll before the regular wait loop.
    return state_generation - 1;
  });

  PortState state = PortState::LIMBO;
  bool connected = false;

  std::vector<sensor_data> rxthread_queue;

  while (true) {
    const bool stop = WithLock(mutex, [&]() {
      while (running && data_queue.empty() &&
             observed_state_generation == state_generation) {
        // Wait until data is queued, state changes, or thread is stopped.
        newdata.wait(mutex);
      }
      if (!running) {
        return true;
      }

      observed_state_generation = state_generation;
      std::swap(rxthread_queue, data_queue);
      return false;
    });
    if (stop) {
      return 0;
    }

    state = GetPortState();

    if (!connected && state == PortState::READY) {
      connected = true;
      devOpen(devGetDeviceOnPort(GetPortIndex()));
    }
    else if (connected && state != PortState::READY) {
      connected = false;
      NotifyDisconnected();
    }

    for (const auto& data : rxthread_queue) {
      ProcessSensorData(data);
    }
    rxthread_queue.clear();
  }
}

void GattSensor::PortStateChanged() {
  WithLock(mutex, [&]() {
    ++state_generation;
  });
  newdata.notify_one();
}

void GattSensor::PortError(const char* msg) {
  StartupStore(_T("GattSensor Error : %s"), to_tstring(msg).c_str());
}

void GattSensor::OnCharacteristicChanged(uuid_t service,
                                         uuid_t characteristic,
                                         std::vector<uint8_t>&& data) {
  WithLock(mutex, [&]() {
    data_queue.emplace_back(std::move(service), std::move(characteristic),
                            std::move(data));
  });
  newdata.notify_one();
}

bool GattSensor::DoEnableNotification(const uuid_t& service,
                                      const uuid_t& characteristic) const {
  auto handler = service_table().get(service, characteristic);
  if (handler) {
    return std::invoke(handler->DoEnableNotification, this);
  }

  const std::lock_guard lock(CritSec_Comm);
  auto port = devGetDeviceOnPort(GetPortIndex());
  if (port && port->DoEnableGattCharacteristic) {
    return port->DoEnableGattCharacteristic(*port, service, characteristic);
  }
  return false;
}

void GattSensor::ProcessSensorData(const sensor_data& data) {
  WithLock(CritSec_Comm, [&]() {
    auto port = devGetDeviceOnPort(GetPortIndex());
    if (port) {
      port->HB = LKHearthBeats;
      AddStatRx(data.data.size());
    }
  });

  try {
    auto handler = service_table().get(data.service, data.characteristic);
    if (handler) {
      return std::invoke(handler->ProcessSensorData, this, data.data);
    }
    OnSensorData<&DeviceDescriptor_t::OnGattCharacteristic>(data.service, data.characteristic, data.data);
  }
  catch(std::exception&) {
    // ignore invalid data ...
  }
}

void GattSensor::DeviceName(const std::vector<uint8_t>& data) {
  tstring name =
      from_unknown_charset(std::string(data.begin(), data.end()).c_str());
  WithLock(mutex, [&]() {
    device_name = std::move(name);
  });
  NotifyConnected();
}

tstring GattSensor::GetDeviceName() {
  const std::lock_guard lock(mutex);
  return device_name;
}

void GattSensor::SerialNumber(const std::vector<uint8_t>& data) {
  tstring serial = from_unknown_charset(std::string(data.begin(), data.end()).c_str());
  const std::lock_guard lock(CritSec_Comm);
  auto port = devGetDeviceOnPort(GetPortIndex());
  if (port) {
    port->SerialNumber = std::move(serial);
  }
}

void GattSensor::BatteryLevel(const std::vector<uint8_t>& data) {
  auto value = characteristic_value<uint8_t>(data).get(0);
  OnSensorData<&DeviceDescriptor_t::OnBatteryLevel, double>(value);
}

void GattSensor::HeartRateMeasurement(const std::vector<uint8_t>& data) {
  if (data.empty()) {
    return;
  }
  auto bpm = [&]() -> uint32_t {
    if (data[0] & 0x01) {
      return characteristic_value<uint16_t>(data).get(1);
    }
    else {
      return characteristic_value<uint8_t>(data).get(1);
    }
  };

  OnSensorData<&DeviceDescriptor_t::OnHeartRate>(bpm());
}

void GattSensor::BarometricPressure(const std::vector<uint8_t>& data) {
  auto value = characteristic_value<uint32_t>(data).get();
  OnSensorData<&DeviceDescriptor_t::OnBarometricPressure>(value / 10.);
}

void GattSensor::OutsideTemperature(const std::vector<uint8_t>& data) {
  auto value = characteristic_value<uint16_t>(data).get();
  if (value != 0x8000) {
    OnSensorData<&DeviceDescriptor_t::OnOutsideTemperature>(
        static_cast<int16_t>(value) / 100.);
  }
}

void GattSensor::RelativeHumidity(const std::vector<uint8_t>& data) {
  auto value = characteristic_value<int16_t>(data).get();
  if (value < 10000) {
    OnSensorData<&DeviceDescriptor_t::OnRelativeHumidity>(value / 100.);
  }
}

void GattSensor::WindOriginDirection(const std::vector<uint8_t>& data) {
  auto value = characteristic_value<uint16_t>(data).get();
  if (value < 35999) {
    OnSensorData<&DeviceDescriptor_t::OnWindOriginDirection>(value / 100.);
  }
}

void GattSensor::WindSpeed(const std::vector<uint8_t>& data) {
  auto value = characteristic_value<int16_t>(data).get();
  OnSensorData<&DeviceDescriptor_t::OnWindSpeed>(
      Units::From(Units_t::unCentimeterPersecond, value));
}

void GattSensor::DataReceived(const void *data, size_t length) {
  ComPort::ProcessData(static_cast<const char*>(data), length);
}

bool GattSensor::Hm10DataEnable() const {
  // TODO: allow to disable this ?
  return true;
}

bool GattSensor::Write_Impl(const void *data, size_t size) {
  if (WriteData(data, size)) {
    AddStatTx(size);
    return true;
  }
  return false;
}
