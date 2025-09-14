/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   devGPSBip.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 7 juin 2016
 */

#include "externs.h"
#include "devGPSBip.h"
#include <functional>
#include <array>
#include "DeviceRegister.h"
#include "devGeneric.h"

namespace {

  const TCHAR* DeviceNameList[] = {
    _T("LK8EX1"),
    _T("C-Probe"),
  };

  auto DeviceDesciptorList = make_device_list<std::size(DeviceNameList)>();

  BOOL ParseNMEA(DeviceDescriptor_t *d, const char *String, NMEA_INFO *GPS_INFO) {
    for(auto& Dev : DeviceDesciptorList) {
      if(Dev.ParseNMEA && Dev.ParseNMEA(d, String, GPS_INFO)) {
        // this device send GPS data only when fix is valid.
        d->nmeaParser.connected = true;
        return TRUE;
      }
    }
    return FALSE;
  }

} // namespace

namespace Stodeus {

void Install(DeviceDescriptor_t* d) {
  genInstall(d); // install Generic driver callback first

  static_assert(DeviceDesciptorList.size() == std::size(DeviceNameList));

  auto ItOut = std::begin(DeviceDesciptorList);
  for(auto DevName : DeviceNameList) {
    auto& descriptor = *(ItOut++);
    descriptor.Reset();

    const DeviceRegister_t* pDev = GetRegisteredDevice(DevName);
    if (pDev) {
      pDev->Install(&descriptor);
    }
  }

  d->ParseNMEA = ParseNMEA;
}

} // Stodeus