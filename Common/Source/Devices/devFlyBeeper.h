/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   devFlyBeeper.h
 * Author: Bruno de Lacheisserie
 */
#ifndef DEVICES_DEVFLYBEEPER_H
#define DEVICES_DEVFLYBEEPER_H

#include "Devices/DeviceRegister.h"

struct DeviceDescriptor_t;

namespace FlyBeeper {

void Install(DeviceDescriptor_t* d);

constexpr const TCHAR* Name = _T("FlyBeeper");

inline constexpr DeviceRegister_t Register() {
 return devRegister(Name, Install);
}

}  // namespace FlyBeeper

#endif /* DEVICES_DEVFLYBEEPER_H */
