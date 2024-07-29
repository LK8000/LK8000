/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   devGenericAutopilot.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 18 september 2023
 */

#ifndef DEVICES_DEVGENERICAUTOPILOT_H
#define DEVICES_DEVGENERICAUTOPILOT_H

#include "Devices/DeviceRegister.h"

struct DeviceDescriptor_t;

namespace GenericAutopilot {

void Install(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t Register() {
  return devRegister(_T("Generic Autopilot"), Install);
}

}  // namespace GenericAutopilot

#endif /* DEVICES_DEVGENERICAUTOPILOT_H */
