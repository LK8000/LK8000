/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devFlyNet.h
 */

#ifndef _DEVFLYNET_H_
#define _DEVFLYNET_H_

#include "Devices/DeviceRegister.h"

BOOL FlyNetParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *_INFO);

void FlyNetInstall(PDeviceDescriptor_t d);

inline constexpr
DeviceRegister_t FlyNetRegister() {
	return devRegister(_T("FlyNet"), FlyNetInstall);
}

#endif // _DEVFLYNET_H_
