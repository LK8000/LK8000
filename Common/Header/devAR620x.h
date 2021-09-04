/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devAR620x.h
 */

#ifndef DEV_AR620x_H
#define DEV_AR620x_H

#include "Devices/DeviceRegister.h"

void AR620xInstall(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t AR620xRegister() {
  return devRegister( _T("Becker AR620x"), AR620xInstall);
}

#endif
