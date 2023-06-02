/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   devXCVario.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 30 may 2023
 */

#ifndef _DEVICES_DEVXCVARIO_H
#define	_DEVICES_DEVXCVARIO_H

#include "Devices/DeviceRegister.h"
struct DeviceDescriptor_t;

namespace XCVario {

void Install(DeviceDescriptor_t* d);

constexpr const TCHAR* Name = _T("XCVario");

inline constexpr
DeviceRegister_t Register() {
  return devRegister(Name, Install);
}

} // XCVario

#endif	/* _DEVICES_DEVXCVARIO_H */
