/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   devXCTracer.h
 * Author: Bruno de Lacheisserie
 *
 * Created on June 1, 2016, 9:44 PM
 */

#ifndef DEVXCTRACER_H
#define DEVXCTRACER_H

#include "Devices/DeviceRegister.h"

void XCTracerInstall(DeviceDescriptor_t* d);

inline constexpr
DeviceRegister_t XCTracerRegister() {
    return devRegister(_T("XCTracer"), XCTracerInstall);
}

#endif /* DEVXCTRACER_H */
