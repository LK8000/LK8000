//
// Created by bruno on 12/20/16.
//

#ifndef ANDROID_STUDIO_INTERNALPORT_H
#define ANDROID_STUDIO_INTERNALPORT_H

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


#endif //ANDROID_STUDIO_INTERNALPORT_H
