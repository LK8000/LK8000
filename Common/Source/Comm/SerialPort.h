/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   SerialPort.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 2 août 2013, 19:11
 */

#ifndef SERIALPORT_H
#define	SERIALPORT_H
#include "ComPort.h"

class SerialPort : public ComPort {
public:
    SerialPort(int idx, const std::tstring& sName, DWORD dwSpeed, BitIndex_t BitSize, bool polling);
    virtual ~SerialPort();

    virtual bool Initialize();
    virtual bool Close();

    virtual void Flush();
    virtual void Purge();
    virtual void DirtyPurge();
    virtual void CancelWaitEvent();

    virtual int SetRxTimeout(int);
    virtual unsigned long SetBaudrate(unsigned long);
    virtual unsigned long GetBaudrate() const;

    virtual void UpdateStatus();

    virtual size_t Read(void *szString, size_t size);
    virtual bool Write(const void *data, size_t length);

protected:
    virtual DWORD RxThread();
    
private:
    HANDLE hPort;
            
    DWORD _dwPortSpeed;
    BitIndex_t _dwPortBit;
    unsigned short valid_frames;

    bool _PollingMode;
    DWORD _dwMask;
};

#endif	/* SERIALPORT_H */

