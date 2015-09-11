/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *  
 * File:   BthPort.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 2 août 2013, 14:37
 */

#ifndef BTHPORT_H
#define	BTHPORT_H

#include "ComPort.h"
#ifndef NO_BLUETOOTH
#ifdef PPC2002
#include <winsock.h>
// WSAGetLastError is alias of GetLastError, WSAGetLastError is not exported by winsok.dll
// and ws2.dll not exist on PPC2002 device.
//#define WSAGetLastError GetLastError
#else
#include <winsock2.h>
#endif


#include "bthapi.h"

class BthPort : public ComPort {
public:
    BthPort(int idx, const std::tstring& sName);
    virtual ~BthPort();

    virtual bool Initialize();
    virtual bool Close();

    virtual int SetRxTimeout(int);

    virtual unsigned long SetBaudrate(unsigned long) {
        return 0UL;
    }

    virtual unsigned long GetBaudrate() const {
        return 0UL;
    }

    virtual void Flush() {
    }

    virtual void Purge() {
    }
    
    virtual void CancelWaitEvent() {
    }

    virtual void UpdateStatus() {
    }

    virtual size_t Read(void *szString, size_t size);
    virtual bool Write(const void *data, size_t length);

protected:
    virtual unsigned RxThread();

private:
    SOCKET mSocket;
    DWORD mTimeout;
};
#else
#include "../NullComPort.h"
typedef NullComPort BthPort;
#endif
#endif	/* BTHPORT_H */

