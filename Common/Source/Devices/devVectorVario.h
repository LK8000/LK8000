/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   devVectorVario.h
 * Author: Bruno de Lacheisserie
 */
#ifndef DEVICES_DEVVECTORVARIO_H
#define DEVICES_DEVVECTORVARIO_H

#include "Devices/DeviceRegister.h"

struct DeviceDescriptor_t;

namespace VectorVario {

void Install(DeviceDescriptor_t* d);

inline constexpr DeviceRegister_t Register() {
 return devRegister(_T("Vector Vario"), Install);
}

}  // namespace VectorVario

#endif /* DEVICES_DEVVECTORVARIO_H */
