/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   BluetoothPort.h
 * Author: Bruno de Lacheisserie
 */

#ifndef ANDROID_BLUETOOTHPORT_H
#define ANDROID_BLUETOOTHPORT_H


#include "externs.h"
#include "AndroidPort.h"
#include "IO/DataHandler.hpp"
#include "Device/Port/Listener.hpp"

class PortBridge;

class BluetoothPort : public AndroidPort {
public:
    using AndroidPort::AndroidPort;

protected:
    PortBridge* CreateBridge() override;
};

class BleHM10Port : public AndroidPort {
public:
    using AndroidPort::AndroidPort;

protected:
    PortBridge* CreateBridge() override;
};

class BluetoothServerPort : public AndroidPort {
public:
    using AndroidPort::AndroidPort;

protected:
    PortBridge* CreateBridge() override;
};

#endif //ANDROID_BLUETOOTHPORT_H
