/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   InternalPort.cpp
 * Author: Bruno de Lacheisserie
 */

#include "InternalPort.h"
#include "Android/InternalSensors.hpp"
#include "Android/Main.hpp"



InternalPort::InternalPort(int idx, const tstring& sName) : NullComPort(idx, sName) { }

InternalPort::~InternalPort() { }

bool InternalPort::Initialize() {

    internal_sensors = InternalSensors::create(Java::GetEnv(), context, GetPortIndex());
    if (internal_sensors) {
        // TODO: Allow user to specify whether they want certain sensors.
        internal_sensors->subscribeToSensor(InternalSensors::TYPE_PRESSURE);

        thread_status = std::thread([&] {
            thread_run();
        });

        return NullComPort::Initialize();
    }
    return false;
}

bool InternalPort::Close() {
    InternalSensors* p = internal_sensors;
    {
        std::unique_lock<std::mutex> lock(mutex_status);
        internal_sensors = nullptr;
        cv_status.notify_all();
    }
    delete p;

    thread_status.join();

    return NullComPort::Close();
}

void InternalPort::CancelWaitEvent() {
    cv_status.notify_all();
}

void InternalPort::thread_run() {
    std::unique_lock<std::mutex> lock(mutex_status);
    while (internal_sensors) {
        cv_status.wait(lock);
        if (!internal_sensors) {
            continue;
        }
        PDeviceDescriptor_t d = devGetDeviceOnPort(GetPortIndex());
        if (d && d->IsBaroSource && d->IsBaroSource(d)) {
            internal_sensors->InitialiseVarioSound();
        }
    }
    internal_sensors->DeinitialiseVarioSound();
}
