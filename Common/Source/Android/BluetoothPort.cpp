/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   BluetoothPort.cpp
 * Author: Bruno de Lacheisserie
 */

#include "BluetoothPort.h"
#include "Android/PortBridge.hpp"
#include "Android/BluetoothHelper.hpp"
#include "functional"

PortBridge* BluetoothPort::CreateBridge() {
    JNIEnv *env = Java::GetEnv();
    if (env && BluetoothHelper::isEnabled(env)) {
        return BluetoothHelper::connect(env, GetPortName());
    }
    return nullptr;
}

PortBridge* BluetoothServerPort::CreateBridge() {
    JNIEnv *env = Java::GetEnv();
    if (env && BluetoothHelper::isEnabled(env)) {
        return BluetoothHelper::createServer(env);
    }
    return nullptr;
}
