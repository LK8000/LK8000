/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devVolkslogger.h
 */

#ifndef	DEVVL_H
#define	DEVVL_H

#include "Devices/DeviceRegister.h"

void vlInstall(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t vlRegister(void){
  return devRegister(_T("Volkslogger"), vlInstall);
}



#endif
