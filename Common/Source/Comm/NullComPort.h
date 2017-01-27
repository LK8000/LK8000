/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   NullComPort.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 10 ao�t 2014, 13:56
 */

#ifndef NULLCOMPORT_H
#define	NULLCOMPORT_H
#include "ComPort.h"

class NullComPort : public ComPort {
public:
    NullComPort(int idx, const tstring& sName) : ComPort(idx, sName) {}

    virtual bool Initialize() { return true; }
    virtual bool Close() { return true; }

    virtual void Flush() { }
    virtual void Purge() { }
    virtual void CancelWaitEvent() { }

    virtual int SetRxTimeout(int TimeOut) { return 0; }
    virtual unsigned long SetBaudrate(unsigned long) { return 0U; }
    virtual unsigned long GetBaudrate() const  {  return 0U; }

    virtual void UpdateStatus()  { };

    virtual bool Write(const void *data, size_t length)  { return true; };
    virtual size_t Read(void *szString, size_t size)  { return 0U; };
protected:
    virtual unsigned RxThread() { assert(false); return 0U; }
};




#endif	/* NULLCOMPORT_H */

