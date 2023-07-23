/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devCAI302.h
 */

#ifndef	DEVCAI302_H
#define	DEVCAI302_H

#include "Devices/DeviceRegister.h"

void cai302Install(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t cai302Register() {
  return devRegister(_T("CAI 302"), cai302Install);
}

#endif
