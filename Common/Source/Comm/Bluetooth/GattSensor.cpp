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
#include "utils/charset_helper.h"

namespace {

  using namespace bluetooth;

  /** Handles incoming BLE UART data for HM-10 and Nordic UART Service devices.
   * this is the default handler set to Port when device driver don't provide a specific handler.
   * required for backward compatibility to allow to use BLE/ serail adapter with any device drivers.
   */

  void BleUart(DeviceDescriptor_t& d, NMEA_INFO&, const std::vector<uint8_t>& data) {
    d.Com->ProcessData(reinterpret_cast<const char*>(data.data()), data.size());
  }

  const gatt_service_table default_gatt_service_table = {{
    { gatt_uuid(0xFFE0), {{ // HM-10 and compatible bluetooth modules
        { gatt_uuid(0xFFE1), { &BleUart }}, // HM-10 RX
        { gatt_uuid(0xFFE4), { &BleUart }}, // SkyDrop2 RX
    }}},
    { NUS_SERVICE, {{ // Nordic UART Service
        { NUS_TX_CHARACTERISTIC, { &BleUart }} // RX
    }}}
  }};

}

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

  std::vector<gatt_sensor_data> rxthread_queue;

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
  using bluetooth::gatt_uuid;

  if (service == gatt_uuid(0x1800) && characteristic == gatt_uuid(0x2A00)) {
    // Generic Access / Device Name
    // must return true to read the device name characteristic and notify user
    // about connecting
    return true;
  }

  if (service == gatt_uuid(0x180A) && characteristic == gatt_uuid(0x2A25)) {
    // Device Information Service / Serial Number String
    return true;
  }

  const std::lock_guard lock(CritSec_Comm);
  auto dev = devGetDeviceOnPort(GetPortIndex());
  if (dev) {
    if (dev->GattTableEmpty()) {
      // Append the default GATT service table to the device if it is empty.
      // this is required to allow to use BLE/serial adapter with any device drivers.
      dev->GattAppend(default_gatt_service_table);
    }
    return dev->GattEnable(service, characteristic, *dev);
  }

  return false;
}

void GattSensor::ProcessSensorData(const gatt_sensor_data& data) {
  using bluetooth::gatt_uuid;
  
  try {

    if (data.service == gatt_uuid(0x1800) && data.characteristic == gatt_uuid(0x2A00)) {
      // Generic Access / Device Name
      DeviceName(data.data);
    }

    if (data.service == gatt_uuid(0x180A) && data.characteristic == gatt_uuid(0x2A25)) {
      // Device Information Service / Serial Number String
      SerialNumber(data.data);
    }

    const std::lock_guard lock(CritSec_Comm);
    auto dev = devGetDeviceOnPort(GetPortIndex());
    if (dev) {
      dev->HB = LKHearthBeats;
      dev->GattNotify(data, GPS_INFO);
    }
  }
  catch (std::exception& e) {
    // ignore invalid data ...
    DebugLog(_T("GattSensor::ProcessSensorData: exception caught: %s"), to_tstring(e.what()).c_str());
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

void GattSensor::DataReceived(const void *data, size_t length) {
  ComPort::ProcessData(static_cast<const char*>(data), length);
}

bool GattSensor::Write_Impl(const void *data, size_t size) {
  if (WriteData(data, size)) {
    AddStatTx(size);
    return true;
  }
  return false;
}
