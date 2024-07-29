/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devDigifly.h
 */

#ifndef DEVDIGIFLY_H
#define DEVDIGIFLY_H

#include "Devices/DeviceRegister.h"

void DigiflyInstall(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t DigiflyRegister() {
  return devRegister(_T("Digifly"), DigiflyInstall);
}

#endif
