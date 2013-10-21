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

#include <winsock2.h>
#include "ComPort.h"
#include "bthapi.h"

class BthPort : public ComPort {
public:
    BthPort(int idx, const std::wstring& sName);
    virtual ~BthPort();

    virtual bool Initialize();
    virtual bool Close();

    virtual int SetRxTimeout(int);

    virtual unsigned long SetBaudrate(unsigned long) {
        return 0UL;
    }

    virtual unsigned long GetBaudrate() {
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
    virtual DWORD RxThread();

private:
    SOCKET mSocket;
};

#endif	/* BTHPORT_H */

