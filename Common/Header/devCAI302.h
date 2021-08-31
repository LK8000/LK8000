/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devCAI302.h
 */

#ifndef	DEVCAI302_H
#define	DEVCAI302_H

#include "Devices/DeviceRegister.h"

void cai302Install(PDeviceDescriptor_t d);

inline constexpr
DeviceRegister_t cai302Register() {
  constexpr unsigned flags = (1l << dfGPS) | (1l << dfLogger) | (1l << dfSpeed)
        | (1l << dfVario) | (1l << dfBaroAlt) | (1l << dfWind);

  return devRegister(_T("CAI 302"), flags, cai302Install);
}

#endif
