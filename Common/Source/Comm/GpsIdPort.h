/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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

#if defined(PNA) && defined(UNDER_CE)
#include "lkgpsapi.h"

class GpsIdPort : public ComPort {
public:
    GpsIdPort(int idx, const tstring& sName);
    ~GpsIdPort();

    bool Initialize() override;
    bool Close() override;

    void Flush() override;
    void Purge() override;
    void CancelWaitEvent() override;

    int SetRxTimeout(int TimeOut) override;
    unsigned long SetBaudrate(unsigned long) override;
    unsigned long GetBaudrate() const override;

    void UpdateStatus() override;

    bool Write(const void *data, size_t size) override;
    size_t Read(void *szString, size_t size) override;

protected:
    unsigned RxThread() override;

    HANDLE _hGPS;  // GPS device
    HANDLE _hLoc;  // signals GPS locaton arrival
    HANDLE _hState;// signals GPS state arrival

};
#else
#include "NullComPort.h"
    typedef NullComPort GpsIdPort;
#endif  /* UNDER_CE */
#endif	/* GPSIDPORT_H */
