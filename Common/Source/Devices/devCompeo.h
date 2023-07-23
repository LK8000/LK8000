/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devCompeo.h
 */

#ifndef DEVCOMPEO_H
#define DEVCOMPEO_H

#include "Devices/DeviceRegister.h"

void CompeoInstall(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t CompeoRegister() {
  return devRegister( _T("Brauniger/Compeo 5030"), CompeoInstall);
}

#endif
