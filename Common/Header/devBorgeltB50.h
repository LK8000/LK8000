/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devBorgeltB50.h
 */

#ifndef DEVB50_H
#define DEVB50_H

#include "Devices/DeviceRegister.h"

void b50Install(PDeviceDescriptor_t d);

inline constexpr
DeviceRegister_t b50Register() {
  return devRegister(_T("Borgelt B50"), (1l << dfGPS), b50Install );
}

#endif
