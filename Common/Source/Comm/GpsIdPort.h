/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   GpsIdPort.h
 * Author: Bruno de Lacheisserie
 * 
 * Created on 2 octobre 2013, 01:52
 */

#ifndef GPSIDPORT_H
#define	GPSIDPORT_H
#include "ComPort.h"
#include "lkgpsapi.h"

class GpsIdPort  : public ComPort {
public:
    GpsIdPort(int idx, const std::wstring& sName);
    virtual ~GpsIdPort();

    virtual bool Initialize();
    virtual bool Close();

    virtual void Flush();
    virtual void Purge();
    virtual void CancelWaitEvent();

    virtual int SetRxTimeout(int TimeOut);
    virtual unsigned long SetBaudrate(unsigned long);
    virtual unsigned long GetBaudrate() const;

    virtual void UpdateStatus();

    virtual bool Write(const void *data, size_t length);
    virtual size_t Read(void *szString, size_t size);

protected:
    virtual DWORD RxThread();

    HANDLE _hGPS;  // GPS device
    HANDLE _hLoc;  // signals GPS locaton arrival
    HANDLE _hState;// signals GPS state arrival

};

#endif	/* GPSIDPORT_H */

