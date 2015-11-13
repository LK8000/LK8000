/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   TTYPort.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 11 août 2014, 10:42
 */

#ifndef TTYPORT_H
#define	TTYPORT_H
#ifdef __linux__
#include "ComPort.h"
#include <atomic>
#include <termios.h>

class TTYPort : public ComPort {
public:
    TTYPort(int idx, const std::tstring& sName, unsigned dwSpeed, BitIndex_t BitSize, bool polling);
    virtual ~TTYPort();

    virtual bool Initialize();
    virtual bool Close();

    virtual void Flush();
    virtual void Purge();
    virtual void DirtyPurge() { Purge(); }
    virtual void CancelWaitEvent();

    virtual int SetRxTimeout(int);
    virtual unsigned long SetBaudrate(unsigned long);
    virtual unsigned long GetBaudrate() const;

    virtual void UpdateStatus();

    virtual size_t Read(void *szString, size_t size);
    virtual bool Write(const void *data, size_t length);

protected:
    virtual unsigned RxThread();

private:
    void signal_handler_IO(int status);

    unsigned _dwPortSpeed;
    BitIndex_t _dwPortBit;
    unsigned short valid_frames;

    int _tty;
    struct termios _oldtio;
    int _Timeout;

};
#endif
#endif	/* TTYPORT_H */

