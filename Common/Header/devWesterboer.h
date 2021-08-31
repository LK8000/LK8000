/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devWesterboer.h
 */

#ifndef DEVWESTER_H
#define DEVWESTER_H

#include "Devices/DeviceRegister.h"

void WesterboerInstall(PDeviceDescriptor_t d);

inline constexpr 
DeviceRegister_t WesterboerRegister() {
  return devRegister(
    TEXT("Westerboer VW1150"),
    (1l << dfBaroAlt) | (1l << dfSpeed) | (1l << dfVario),
    WesterboerInstall);
}

#endif
