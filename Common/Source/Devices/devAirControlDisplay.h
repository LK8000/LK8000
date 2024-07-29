/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   devAirControlDisplay.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 27 october 2023
 */

#ifndef DEVICES_DEVAIRCONTROLDISPLAY_H
#define DEVICES_DEVAIRCONTROLDISPLAY_H

#include "Devices/DeviceRegister.h"

struct DeviceDescriptor_t;

namespace AirControlDisplay {

void Install(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t Register() {
  return devRegister(_T("Air Control Display"), Install);
}

}  // namespace AirControlDisplay

#endif /* DEVICES_DEVAIRCONTROLDISPLAY_H */
