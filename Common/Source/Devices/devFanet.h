/*
 *  LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 *  Released under GNU/GPL License v.2 or later
 *  See CREDITS.TXT file for authors and copyrights
 *
 *  File:   devFanet.h
 *  Author: Gerald Eichler
 *
 *  Created on 13 march 2020, 14:45
 */

#ifndef DEVFANET_H
#define	DEVFANET_H

#include "Devices/DeviceRegister.h"

namespace GXAirCom {

constexpr const TCHAR DeviceName[] = _T("GXAirCom");

void Install(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t Register() {
  return (devRegister(DeviceName, Install));
}

} // GXAirCom

#endif	/* DEVFANET_H */
