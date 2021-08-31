/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devDisabled.h
 */

#ifndef	DEVDISABLED_H
#define	DEVDISABLED_H

#include "Devices/DeviceRegister.h"

void disInstall(PDeviceDescriptor_t d);

inline constexpr
DeviceRegister_t disRegister() {
  return devRegister(_T(DEV_DISABLED_NAME), (1l << dfGPS), disInstall);
}

#endif
