/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devLK8EX1.h
 */

#ifndef DEVLK8EX1_H
#define DEVLK8EX1_H

#include "Devices/DeviceRegister.h"

void LK8EX1Install(PDeviceDescriptor_t d);

inline constexpr
DeviceRegister_t LK8EX1Register(void){
  return devRegister( _T("LK8EX1"), LK8EX1Install);
}

BOOL LK8EX1ParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS);
BOOL LK8EX1IsBaroSource(PDeviceDescriptor_t d);
BOOL LK8EX1LinkTimeout(PDeviceDescriptor_t d);
#endif
