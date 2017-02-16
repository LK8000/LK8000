/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   InternalPort.h
 * Author: Bruno de Lacheisserie
 *
 * Adapted from original code provided by Naviter
 */

#ifndef ANDROID_INTERNALPORT_H
#define ANDROID_INTERNALPORT_H

#include <externs.h>
#include <Comm/NullComPort.h>

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

private:

    InternalSensors* internal_sensors;

};


#endif //ANDROID_INTERNALPORT_H
