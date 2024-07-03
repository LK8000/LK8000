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

constexpr const TCHAR GPSBip[] = _T("GPSBip");

void GPSBipInstall(DeviceDescriptor_t* d);

inline constexpr DeviceRegister_t GPSBipRegister() {
  return devRegister( GPSBip, GPSBipInstall);
}

constexpr const TCHAR UltraBip[] = _T("UltraBip");

void UltraBipInstall(DeviceDescriptor_t* d);

inline constexpr DeviceRegister_t UltraBipRegister() {
  return devRegister(UltraBip, UltraBipInstall);
}

constexpr const TCHAR BlueBip[] = _T("BlueBip");

void BlueBipInstall(DeviceDescriptor_t* d);

inline constexpr DeviceRegister_t BlueBipRegister() {
  return devRegister(BlueBip, BlueBipInstall);
}

} // Stodeus

#endif	/* DEVGPSBIP_H */
