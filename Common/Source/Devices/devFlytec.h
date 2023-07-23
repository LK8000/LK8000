/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devFlytec.h
 */

#ifndef DEVFLYTEC_H
#define DEVFLYTEC_H

#include "Devices/DeviceRegister.h"

void FlytecInstall(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t FlytecRegister() {
  return devRegister(_T("Flytec/FLYSEN"), FlytecInstall);
}

#endif
