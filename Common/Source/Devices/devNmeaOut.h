/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devNmeaOut.h
 */

#ifndef	DEVNMEAOUT_H
#define	DEVNMEAOUT_H

#include "Devices/DeviceRegister.h"

void nmoInstall(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t nmoRegister(void){
  return devRegister(_T("NmeaOut"), nmoInstall);
}

#endif
