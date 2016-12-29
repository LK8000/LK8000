//
// Created by bruno on 12/20/16.
//

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

