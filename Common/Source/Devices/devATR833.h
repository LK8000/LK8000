/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devATR833.h
 */

#ifndef DEV_ATR833_H
#define DEV_ATR833_H

#include "Devices/DeviceRegister.h"

void ATR833Install(PDeviceDescriptor_t d);

inline constexpr
DeviceRegister_t ATR833Register(void){
  return devRegister(_T("f.u.n.k.e. ATR833"), ATR833Install);
}

#endif
