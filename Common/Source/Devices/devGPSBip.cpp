/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
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

static
BOOL Install(PDeviceDescriptor_t d){

  LKASSERT(DeviceDesciptorList.size() == std::size(DeviceNameList));
  auto ItOut = std::begin(DeviceDesciptorList);
  for(auto DevName : DeviceNameList) {
    DeviceRegister_t* pDev = std::find_if(&DeviceRegister[0], &DeviceRegister[DeviceRegisterCount], std::bind(&devNameCompare, _1, DevName));
    if (pDev != &DeviceRegister[DeviceRegisterCount]) {
      pDev->Installer(ItOut++);
    } else {
      LKASSERT(false);
      DeviceRegister[0].Installer(ItOut++); // Disabled
    }
  }

  _tcscpy(d->Name, szDeviceName);
  d->ParseNMEA = ParseNMEA;
  d->IsGPSSource = GetTrue;
  d->IsBaroSource = GetTrue;
  return(TRUE);

}

BOOL GPSBipRegister(void) {
  return(devRegister(
    szDeviceName,
      (1l << dfGPS) |
      (1l << dfVario) |
      (1l << dfBaroAlt)
    ,
    Install
  ));
}
