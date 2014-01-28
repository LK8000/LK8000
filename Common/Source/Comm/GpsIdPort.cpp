/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   GpsIdPort.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 2 octobre 2013, 01:52
 */
#include "externs.h"
#include "GpsIdPort.h"

template <typename T>
void GPSResetData(T& data) {
    memset(&data, 0, data.dwSize);
    data.dwSize = sizeof (T);
    data.dwVersion = 1;
}

GpsIdPort::GpsIdPort(int idx, const std::wstring& sName) : ComPort(idx, sName),
_hGPS(NULL),
_hLoc(NULL),
_hState(NULL) {

}

GpsIdPort::~GpsIdPort() {

}

bool GpsIdPort::Initialize() {
    _hLoc = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    _hState = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    _hGPS = ::GPSOpenDevice(_hLoc, _hState, NULL, 0);
    if (0 == _hGPS) {
        StartupStore(_T("Unable to Open GPS Intermediate driver %s"), NEWLINE);    
        return false;
    }
    SetPortStatus(CPS_OPENOK);

    GPS_DEVICE dev = {0};
    GPSResetData(dev);
    GPSGetDeviceState(&dev);
    StartupStore(_T("GPSID : DeviceState: %X, ServiceState: %X%s"), dev.dwDeviceState, dev.dwServiceState, NEWLINE);
//    StartupStore(_T("GPSID : LastDataTime: %s%s"), dev.ftLastDataReceived, NEWLINE);
    StartupStore(_T("GPSID : DrvPrefix; %s%s"), dev.szGPSDriverPrefix, NEWLINE);
    StartupStore(_T("GPSID : MxPrefix %s%s"), dev.szGPSMultiplexPrefix, NEWLINE);
    StartupStore(_T("GPSID : Name:%s%s"), dev.szGPSFriendlyName, NEWLINE);

    if (!StartRxThread()) {
        StartupStore(_T(". ComPort %u <%s> Failed to start Rx Thread%s"), GetPortIndex() + 1, GetPortName(), NEWLINE);
        return false;
    }
    return true;
}

bool GpsIdPort::Close() {
    ComPort::Close();

    
    if(_hGPS) ::GPSCloseDevice(_hGPS);
    if(_hLoc) ::CloseHandle(_hLoc);
    if(_hState) ::CloseHandle(_hState);

    return true;
}

void GpsIdPort::UpdateStatus() {

}

DWORD GpsIdPort::RxThread() {
    DWORD rc = 0;
    const int nh = 3;
    HANDLE handles[nh] = {0};
    handles[0] = hStop;
    handles[1] = _hLoc;
    handles[2] = _hState;

    GPS_POSITION loc = {0};
    GPSResetData(loc);

    GPS_DEVICE dev = {0};
    GPSResetData(dev);

    bool listen = true;
    while (listen) {
        DWORD dw = ::WaitForMultipleObjects(nh, handles, FALSE, INFINITE);
        switch (dw) {
            case WAIT_OBJECT_0 + 0:
                listen = false;
                break;
            case WAIT_OBJECT_0 + 1:
                rc = GPSGetPosition(_hGPS, &loc, 10000, 0);
                if(ERROR_SUCCESS == rc) {
                    AddStatRx(1);
                    NMEAParser::ParseGPS_POSITION(GetPortIndex(), loc, GPS_INFO);
                }
                GPSResetData(loc);
                break;
            case WAIT_OBJECT_0 + 2:
                rc = GPSGetDeviceState(&dev);
                if(ERROR_SUCCESS == rc) {
                    AddStatRx(1);
                    StartupStore(_T("GPSID : DeviceState: %X, ServiceState: %X%s"), dev.dwDeviceState, dev.dwServiceState, NEWLINE);
                }
                GPSResetData(dev);
                break;
            case WAIT_FAILED:
                listen = false;
                rc = ::GetLastError();
                break;
        }
    }

    return rc;
}

void GpsIdPort::Flush() {
    LKASSERT(FALSE);
}

void GpsIdPort::Purge() {
    LKASSERT(FALSE);
}

void GpsIdPort::CancelWaitEvent() {
}

int GpsIdPort::SetRxTimeout(int TimeOut) {
    LKASSERT(FALSE);
    return 0;
}

unsigned long GpsIdPort::SetBaudrate(unsigned long) {
    LKASSERT(FALSE);
    return 0U;
}

unsigned long GpsIdPort::GetBaudrate() const {
    LKASSERT(FALSE);
    return 0U;
}

bool GpsIdPort::Write(const void *data, size_t length) {
    LKASSERT(FALSE);
    return false;
}

size_t GpsIdPort::Read(void *szString, size_t size) {
    LKASSERT(FALSE);
    return 0U;
}
