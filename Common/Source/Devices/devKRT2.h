/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devKRT2.h
 */

#ifndef DEV_KRT2_H
#define DEV_KRT2_H

#include "Devices/DeviceRegister.h"

void KRT2Install(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t KRT2Register() {
  return devRegister(_T("Dittel KRT2"), KRT2Install);
}

#endif
