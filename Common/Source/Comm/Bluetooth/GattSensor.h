/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GattSensor.h
 *
 * Platform-independent Bluetooth LE GATT sensor logic, shared between the
 * Android backend (Android/BluetoothSensor) and the Linux/Kobo BlueZ backend
 * (Comm/Bluetooth/BlueZGattSensor). Subclasses only implement the small
 * "transport" interface below (Connect/Disconnect/GetPortState/WriteData/
 * DoWriteGattCharacteristic/DoReadGattCharacteristic); everything else --
 * service/characteristic dispatch, the Rx thread, and the known-characteristic
 * decoders -- lives here.
 */

#ifndef COMM_BLUETOOTH_GATTSENSOR_H
#define COMM_BLUETOOTH_GATTSENSOR_H

#include "Thread/Mutex.hpp"
#include "ComPort.h"
#include "Device/Port/Listener.hpp"
#include "IO/DataHandler.hpp"

#include <vector>
#include <string>

class GattSensor : public ComPort, protected PortListener, protected DataHandler {
 public:
  using ComPort::ComPort;

  /* override ComPort */
 public:
  bool Initialize() override;
  bool Close() override;

  bool StopRxThread() override;
  bool StartRxThread() override;

  void Purge() override {};
  void Flush() override {};
  void CancelWaitEvent() override;

  bool IsReady() override;

  int SetRxTimeout(int TimeOut) override { return 0; }

  unsigned long SetBaudrate(unsigned long) override { return 0; };

  unsigned long GetBaudrate() const override { return 0; }

  size_t Read(void* data, size_t size) override { return 0; };

 protected:
  unsigned RxThread() override;

  tstring GetDeviceName() override;

  /**
   * The transport's connection state, polled by RxThread() and IsReady().
   */
  enum class PortState : int {
    READY = 0,
    FAILED = 1,
    LIMBO = 2,
  };

  /**
   * Start connecting to the device named by GetPortName() (its Bluetooth
   * address). Called from Initialize(). May throw on immediate/local
   * failure; actual connection success or failure is reported later
   * through PortStateChanged()/PortError(), polled via GetPortState().
   */
  virtual bool Connect() = 0;

  /**
   * Tear down the connection. Called from Close(), always after RxThread()
   * has fully stopped -- implementations don't need to guard against
   * concurrent use from the Rx thread, only from whichever other thread
   * might call GetPortState()/WriteData()/DoWrite.../DoRead... concurrently.
   * Must also be safe to call after a failed or partial Connect().
   */
  virtual void Disconnect() = 0;

  virtual PortState GetPortState() const = 0;

  /**
   * Write raw data to the device's default write characteristic (e.g. the
   * HM-10 RX/TX characteristic). Returns false on error; a short/partial
   * write is not distinguished from a full one, unlike ComPort::Write_Impl
   * in general -- the underlying transports either write it all or fail.
   */
  virtual bool WriteData(const void* data, size_t size) = 0;

 private:
  bool Write_Impl(const void* data, size_t size) override;

  Mutex mutex;
  Cond newdata;

  bool running = false;

  struct sensor_data {
    sensor_data(uuid_t&& s, uuid_t&& c, std::vector<uint8_t>&& _data)
        : service(s), characteristic(c), data(std::move(_data)) {}

    uuid_t service;
    uuid_t characteristic;
    std::vector<uint8_t> data;
  };

  std::vector<sensor_data> data_queue;
  unsigned state_generation = 0;
  tstring device_name;

  void ProcessSensorData(const sensor_data& data);

 public:

  template <auto callback>
  bool EnableCharacteristic() const {
    const std::lock_guard lock(CritSec_Comm);
    auto port = devGetDeviceOnPort(GetPortIndex());
    return port && port->*callback;
  }

  template <auto callback, typename... Args>
  void OnSensorData(Args&& ...args) {
    const std::lock_guard lock(CritSec_Comm);
    auto port = devGetDeviceOnPort(GetPortIndex());
    if (port && port->*callback) {
      std::invoke(port->*callback, *port, GPS_INFO, std::forward<Args>(args)...);
    }
  }

  void DeviceName(const std::vector<uint8_t>& data);
  void SerialNumber(const std::vector<uint8_t>& data);

  void BatteryLevel(const std::vector<uint8_t>& data);

  void HeartRateMeasurement(const std::vector<uint8_t>& data);

  void BarometricPressure(const std::vector<uint8_t>& data);
  void OutsideTemperature(const std::vector<uint8_t>& data);
  void RelativeHumidity(const std::vector<uint8_t>& data);
  void WindOriginDirection(const std::vector<uint8_t>& data);
  void WindSpeed(const std::vector<uint8_t>& data);

  void Hm10Data(const std::vector<uint8_t>& data) {
    DataReceived(data.data(), data.size());
  }
  bool Hm10DataEnable() const;

  /* override PortListener */
 protected:
  void PortStateChanged() override;
  void PortError(const char* msg) override;

  /* override DataHandler */
 public:
  void DataReceived(const void* data, size_t size) override;
  void OnCharacteristicChanged(uuid_t service, uuid_t characteristic, std::vector<uint8_t>&& data) override;
  bool DoEnableNotification(const uuid_t& service, const uuid_t& characteristic) const override;
};

// Explicit specialization of a member template must be at namespace scope,
// not inside the class body (some compilers accept the latter as a
// non-standard extension, but not all -- keep this one portable).
template <>
inline bool GattSensor::EnableCharacteristic<true>() const {
  return true;
}

#endif  // COMM_BLUETOOTH_GATTSENSOR_H
