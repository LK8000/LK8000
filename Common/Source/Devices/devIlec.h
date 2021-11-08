/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devIlec.h
 */

#ifndef DEVILEC_H
#define DEVILEC_H

#include "Devices/DeviceRegister.h"

void IlecInstall(PDeviceDescriptor_t d);

inline constexpr
DeviceRegister_t IlecRegister() {
  return devRegister(_T("Ilec SN10"), IlecInstall);
}

#endif
