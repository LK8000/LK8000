/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devGeneric.h
 */

#ifndef	DEVGENERIC_H
#define	DEVGENERIC_H

#include "Devices/DeviceRegister.h"

void genInstall(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t genRegister() {
  return devRegister( _T("Generic"), genInstall);
}

void internalInstall(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t InternalRegister() {
  return devRegister(DEV_INTERNAL_NAME, internalInstall);
}

#endif
