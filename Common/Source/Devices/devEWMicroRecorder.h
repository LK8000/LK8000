/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devEWMicroRecorder.h
 */

#ifndef	DEVEWMICRORECORDER_H
#define	DEVEWMICRORECORDER_H

#include "Devices/DeviceRegister.h"

void ewMicroRecorderInstall(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t ewMicroRecorderRegister() {
  return devRegister(_T("EW MicroRecorder"), ewMicroRecorderInstall);
}

#endif
