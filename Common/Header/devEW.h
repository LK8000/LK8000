/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devEW.h
 */

#ifndef	DEVEW_H
#define	DEVEW_H

#include "Devices/DeviceRegister.h"

void ewInstall(PDeviceDescriptor_t d);

inline constexpr
DeviceRegister_t ewRegister() {
  return devRegister(_T("EW Logger"), 1l << dfGPS | 1l << dfLogger, ewInstall);
}

#endif
