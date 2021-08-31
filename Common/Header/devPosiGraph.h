/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devPosiGraph.h
 */

#ifndef DEVPG_H
#define DEVPG_H

#include "Devices/DeviceRegister.h"

void pgInstall(PDeviceDescriptor_t d);

inline constexpr
DeviceRegister_t pgRegister() {
  return devRegister(
    _T("PosiGraph Logger"),
    1l << dfGPS | (1l << dfBaroAlt) | 1l << dfLogger,
    pgInstall);
}

#endif
