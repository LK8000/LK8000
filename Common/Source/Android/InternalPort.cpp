/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   InternalPort.cpp
 * Author: Bruno de Lacheisserie
 *
 * Adapted from original code provided by Naviter
 */

#include "InternalPort.h"
#include "Android/InternalSensors.hpp"
#include "Android/Main.hpp"



InternalPort::InternalPort(int idx, const tstring& sName) : NullComPort(idx, sName), internal_sensors() {

}

InternalPort::~InternalPort() {

};

bool InternalPort::Initialize() {

    internal_sensors = InternalSensors::create(Java::GetEnv(), context, GetPortIndex());
    if (internal_sensors) {
// TODO: Allow user to specify whether they want certain sensors.
        internal_sensors->subscribeToSensor(InternalSensors::TYPE_PRESSURE);
        return NullComPort::Initialize();
    }
    return false;
}

bool InternalPort::Close() {
    delete internal_sensors;
    internal_sensors = nullptr;

    return NullComPort::Close();
}

