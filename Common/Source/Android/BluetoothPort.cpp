/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   BluetoothPort.cpp
 * Author: Bruno de Lacheisserie
 */

#include "BluetoothPort.h"
#include "Android/PortBridge.hpp"
#include "Android/BluetoothHelper.hpp"
#include "OS/Sleep.h"
#include "functional"

bool BluetoothPort::Initialize() {

    bridge = BluetoothHelper::connect(Java::GetEnv(), GetPortName());
    if (bridge == nullptr)
        return false;

    return AndroidPort::Initialize();
}

bool BluetoothServerPort::Initialize() {

    bridge = BluetoothHelper::createServer(Java::GetEnv());
    if (bridge == nullptr)
        return false;

    return AndroidPort::Initialize();
}
