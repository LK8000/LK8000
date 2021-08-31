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

using std::placeholders::_1;

static const
TCHAR szDeviceName[] = TEXT("GPSBip");

static const TCHAR* DeviceNameList[] = {
  TEXT("LK8EX1"),
  TEXT("C-Probe"),
};

static
std::array<DeviceDescriptor_t, std::size(DeviceNameList)> DeviceDesciptorList;


static
BOOL ParseNMEA(DeviceDescriptor_t *d, TCHAR *String, NMEA_INFO *GPS_INFO) {
  for(auto& Dev : DeviceDesciptorList) {
    if(Dev.ParseNMEA && Dev.ParseNMEA(d, String, GPS_INFO)) {
      return TRUE;
    }
  }
  return FALSE;
}

static
BOOL GetTrue(DeviceDescriptor_t *d) {
  return TRUE;
}

void GPSBipInstall(PDeviceDescriptor_t d) {

  LKASSERT(DeviceDesciptorList.size() == std::size(DeviceNameList));
  auto ItOut = std::begin(DeviceDesciptorList);
  for(auto DevName : DeviceNameList) {
    const DeviceRegister_t* pDev = GetRegisteredDevice(DevName);
    if (pDev) {
      pDev->Installer(ItOut++);
    }
  }

  _tcscpy(d->Name, szDeviceName);
  d->ParseNMEA = ParseNMEA;
  d->IsGPSSource = GetTrue;
  d->IsBaroSource = GetTrue;
}
