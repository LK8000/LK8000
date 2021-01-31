/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   InternalPort.h
 * Author: Bruno de Lacheisserie
 */

#ifndef ANDROID_INTERNALPORT_H
#define ANDROID_INTERNALPORT_H

#include <externs.h>
#include <Comm/NullComPort.h>

#include <thread>

class InternalSensors;

class InternalPort : public NullComPort {

public:
    InternalPort(int idx, const tstring& sName);
    ~InternalPort();

    InternalPort( const InternalPort& ) = delete;
    InternalPort& operator=( const InternalPort& ) = delete;

    InternalPort( InternalPort&& ) = delete;
    InternalPort& operator=( InternalPort&& ) = delete;

    bool Initialize() override;
    bool Close() override;

    void CancelWaitEvent() override;

private:
    void thread_run();

    InternalSensors* internal_sensors = nullptr;

    std::thread thread_status;
    std::mutex mutex_status;
    std::condition_variable cv_status;
};


#endif //ANDROID_INTERNALPORT_H
