/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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

        thread_status = std::thread(&InternalPort::thread_run, this);

        return NullComPort::Initialize();
    }
    return false;
}

bool InternalPort::Close() {
    InternalSensors* p = WithLock(mutex_status, [&]() {
        /* set "internal_sensors" to nullptr and notify "cv_status" to ask
         * to stop "thread_status" before delete internal_sensors
         */
        auto p = std::exchange(internal_sensors, nullptr);
        cv_status.notify_all();
        return p;
    });

    delete p;

    if(thread_status.joinable()) {
        thread_status.join();
    }

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
        if(EnableAudioVario) {
            PDeviceDescriptor_t d = devGetDeviceOnPort(GetPortIndex());
            if (d && d->IsBaroSource && d->IsBaroSource(d)) {
                internal_sensors->InitialiseVarioSound();
            }
        }
    }
    internal_sensors->DeinitialiseVarioSound();
}
