/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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
    NullComPort(unsigned idx, const tstring& sName) : ComPort(idx, sName) {}

    bool StartRxThread() override { return true; }

    bool Initialize() override { return true; }
    bool Close() override { return true; }

    bool IsReady() override {
        return true;
    }

    void Flush() override { }
    void Purge() override { }
    void CancelWaitEvent() override { }

    int SetRxTimeout(int TimeOut) override { return 0; }
    unsigned long SetBaudrate(unsigned long) override { return 0U; }
    unsigned long GetBaudrate() const  override {  return 0U; }

    void UpdateStatus() override { };

    size_t Read(void *data, size_t size) override { return 0U; };
protected:
    unsigned RxThread() override { assert(false); return 0U; }

private:
    bool Write_Impl(const void *data, size_t size) override { return true; };
};




#endif	/* NULLCOMPORT_H */

