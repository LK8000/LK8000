/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   devGDL90.h
 * Author: Bruno de Lacheisserie
 */
#pragma once

#include "DeviceRegister.h"

namespace GDL90 {

void Install(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t Register() {
  return devRegister(_T("GDL-90"), Install);
}

}  // namespace GDL90
