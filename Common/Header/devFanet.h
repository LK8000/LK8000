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

void FanetInstall(PDeviceDescriptor_t d);

inline constexpr
DeviceRegister_t FanetRegister() {
  return(devRegister(TEXT("FANET"), FanetInstall));
}

#endif	/* DEVFANETH */
