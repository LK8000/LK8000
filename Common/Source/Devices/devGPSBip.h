/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   devGPSBip.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 7 juin 2016
 */

#ifndef DEVGPSBIP_H
#define	DEVGPSBIP_H

#include "Devices/DeviceRegister.h"

void GPSBipInstall(PDeviceDescriptor_t d);

inline constexpr
DeviceRegister_t GPSBipRegister() {
  return devRegister(_T("GPSBip"), GPSBipInstall);
}

#endif	/* DEVGPSBIP_H */
