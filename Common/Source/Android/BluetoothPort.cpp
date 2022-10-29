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
#include "OS/Sleep.h"
#include "functional"

bool BluetoothPort::CreateBridge() {
    JNIEnv *env = Java::GetEnv();
    if (env && BluetoothHelper::isEnabled(env)) {
        bridge = BluetoothHelper::connect(env, GetPortName());
    }
    return (bridge);
}

bool BleHM10Port::CreateBridge() {
    JNIEnv *env = Java::GetEnv();
    if (env && BluetoothHelper::isEnabled(env)) {
        bridge = BluetoothHelper::connectHM10(env, GetPortName());
    }
    return (bridge);
}

bool BluetoothServerPort::CreateBridge() {
    JNIEnv *env = Java::GetEnv();
    if (env && BluetoothHelper::isEnabled(env)) {
        bridge = BluetoothHelper::createServer(env);
    }
    return (bridge);
}
