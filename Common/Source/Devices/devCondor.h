/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devCondor.h
 */

#ifndef DEVCONDOR_H
#define DEVCONDOR_H

#include "Devices/DeviceRegister.h"

void condorInstall(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t condorRegister() {
  return devRegister(_T("Condor"), condorInstall);
}

#endif
