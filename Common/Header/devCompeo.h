/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devCompeo.h
 */

#ifndef DEVCOMPEO_H
#define DEVCOMPEO_H

#include "Devices/DeviceRegister.h"

void CompeoInstall(PDeviceDescriptor_t d);

inline constexpr
DeviceRegister_t CompeoRegister() {
  constexpr unsigned flags = (1l << dfGPS) | (1l << dfBaroAlt) | (1l << dfSpeed) | (1l << dfVario);
  return devRegister( _T("Brauniger/Compeo 5030"), flags, CompeoInstall );
}

#endif
