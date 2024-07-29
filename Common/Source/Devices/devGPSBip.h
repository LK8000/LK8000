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

namespace Stodeus {

void Install(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t GPSBipRegister() {
  return devRegister( _T("GPSBip"), Install);
}

inline constexpr
DeviceRegister_t UltraBipRegister() {
  return devRegister(_T("UltraBip"), Install);
}

inline constexpr
DeviceRegister_t BlueBipRegister() {
  return devRegister(_T("BlueBip"), Install);
}

} // Stodeus

#endif	/* DEVGPSBIP_H */
