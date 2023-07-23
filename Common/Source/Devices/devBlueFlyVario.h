/*
 *  LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 *  Released under GNU/GPL License v.2 or later
 *  See CREDITS.TXT file for authors and copyrights
 *
 *  File:   devBlueFlyVario.h
 *  Author: Bruno de Lacheisserie
 *
 *  Created on 2 décembre 2013, 21:00
 */

#ifndef DEVBLUEFLYVARIO_H
#define	DEVBLUEFLYVARIO_H

#include "Devices/DeviceRegister.h"

void BlueFlyInstall(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t BlueFlyRegister() {
  return devRegister(_T("BlueFlyVario"), BlueFlyInstall);
}

#endif	/* DEVBLUEFLYVARIO_H */
