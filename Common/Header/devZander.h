/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devZander.h
 */

#ifndef DEVBZANDER_H
#define DEVBZANDER_H

#include "Devices/DeviceRegister.h"

void zanderInstall(PDeviceDescriptor_t d);

inline constexpr
DeviceRegister_t zanderRegister() {
  return devRegister(
    _T("Zander"),
    (1l << dfGPS) | (1l << dfBaroAlt) | (1l << dfSpeed) | (1l << dfVario),
    zanderInstall
  );
}

#endif
