/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devXCOM760.h
 */

#ifndef DEVXCOM760_H
#define DEVXCOM760_H

#include "Devices/DeviceRegister.h"

void XCOM760Install(PDeviceDescriptor_t d);

inline constexpr
DeviceRegister_t xcom760Register() {
  return devRegister(_T("XCOM760"), XCOM760Install);
}

#endif
