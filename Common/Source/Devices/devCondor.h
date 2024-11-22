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

void CondorInstall(DeviceDescriptor_t* d);
void Condor3Install(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t condorRegister() {
  return devRegister(_T("Condor"), CondorInstall);
}

inline constexpr
DeviceRegister_t Condor3Register() {
  return devRegister(_T("Condor 3"), Condor3Install);
}

#endif
