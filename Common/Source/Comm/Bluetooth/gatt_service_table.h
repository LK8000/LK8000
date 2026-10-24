/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   gatt_service_table.h
 * Author: Bruno de Lacheisserie
 */
#pragma once

#include <unordered_map>
#include <functional>
#include "gatt_utils.h"

struct DeviceDescriptor_t;
struct NMEA_INFO;

namespace bluetooth {

class gatt_service_table {
  using notify_t = std::function<void(DeviceDescriptor_t&, NMEA_INFO&,
                                      const std::vector<uint8_t>&)>;
  using enable_t = std::function<bool(DeviceDescriptor_t&)>;

  class handler_t {
   public:
    handler_t() = default;

    handler_t(notify_t notify) noexcept : notify_(std::move(notify)) {}

    handler_t(notify_t notify, enable_t enable) noexcept
        : notify_(std::move(notify)), enable_(std::move(enable)) {}

    explicit operator bool() const noexcept {
      return static_cast<bool>(notify_);
    }

    bool enable(DeviceDescriptor_t& dev) const {
      if (notify_) {  // Only enable if notify is set, should be always true if
                      // this handler is valid, see operator bool()
        if (enable_) {  // If enable is provided, use it
          return enable_(dev);
        }
        // If notify is set but enable is not provided, default to true
        return true;
      }
      // If notify is not set, cannot enable
      return false;
    }

    void notify(DeviceDescriptor_t& dev, NMEA_INFO& info,
                const std::vector<uint8_t>& data) const {
      if (notify_) { // should always be true if this handler is valid, see operator bool()
        notify_(dev, info, data);
      }
    }

   private:
    notify_t notify_;
    enable_t enable_;
  };

  using characteristic_t = std::unordered_map<uuid_t, handler_t, uuid_hash>;
  using service_t = std::unordered_map<uuid_t, characteristic_t, uuid_hash>;

 public:
  using value_type = typename service_t::value_type;

  gatt_service_table() = default;

  gatt_service_table(service_t table) noexcept : table(std::move(table)) {}

  const handler_t* get(const uuid_t& service,
                       const uuid_t& characteristic) const {
    auto it_svc = table.find(service);
    if (it_svc != table.end()) {
      auto it_chr = it_svc->second.find(characteristic);
      if (it_chr != it_svc->second.end()) {
        return &it_chr->second;
      }
    }
    return nullptr;
  }

  bool empty() const {
    return table.empty();
  }

  void insert(const uuid_t& service, const uuid_t& characteristic,
              handler_t&& handler) {
    if (handler) {
      table[service][characteristic] = std::move(handler);
    }
    else {
      // if handler is empty, remove it
      erase(service, characteristic);
    }
  }

  void erase(const uuid_t& service, const uuid_t& characteristic) {
    auto it_svc = table.find(service);
    if (it_svc == table.end()) {
      return;
    }
    it_svc->second.erase(characteristic);
    if (it_svc->second.empty()) {
      // if the service has no characteristics left, remove it
      table.erase(it_svc);
    }
  }

  void append(service_t list) {
    for (auto& svc : list) {
      auto& service = table[svc.first];
      for (auto& chr_item : svc.second) {
        if (!chr_item.second) {
          // remove characteristics without valid handler
          service.erase(chr_item.first);
          continue;
        }
        service[chr_item.first] = std::move(chr_item.second);  
      }
      if (service.empty()) {
        // Remove the service if it has no characteristics left
        table.erase(svc.first);
      }
    }
  }

  void append(const gatt_service_table& other) {
    append(other.table);
  }

  void clear() {
    table.clear();
  }

  bool enable(const uuid_t& service, const uuid_t& characteristic,
              DeviceDescriptor_t& dev) const {
    if (const handler_t* h = get(service, characteristic)) {
      return h->enable(dev);
    }
    return false;
  }

  void notify(const gatt_sensor_data& data, DeviceDescriptor_t& dev,
              NMEA_INFO& info) const {
    if (const handler_t* h = get(data.service, data.characteristic)) {
      h->notify(dev, info, data.data);
    }
  }

 private:
  service_t table;
};

}  // namespace bluetooth
