/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devFlymasterF1.h
 */

#ifndef DEVFLYMASTERF1_H
#define DEVFLYMASTERF1_H

#include "Devices/DeviceRegister.h"

void flymasterf1Install(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t flymasterf1Register() {
  return devRegister(_T("FlymasterF1"), flymasterf1Install);
}

void flymasterInstall(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t flymasterGPSRegister() {
  return devRegister(_T("Flymaster GPS"), flymasterInstall);
}

#endif
