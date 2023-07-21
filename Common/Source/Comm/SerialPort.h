/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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


#ifdef WIN32
class SerialPort : public ComPort {
public:
    SerialPort(unsigned idx, const tstring& sName, DWORD dwSpeed, BitIndex_t BitSize, bool polling);
    virtual ~SerialPort();

    bool Initialize() override;
    bool Close() override;

    void Flush() override;
    void Purge() override;
    void CancelWaitEvent() override;

    bool IsReady() override;

    int SetRxTimeout(int) override;
    unsigned long SetBaudrate(unsigned long) override;
    unsigned long GetBaudrate() const override;

    void UpdateStatus() override;

    size_t Read(void *data, size_t size) override;

protected:
    unsigned RxThread() override;

    void DirtyPurge();

private:
    HANDLE hPort;

    DWORD _dwPortSpeed;
    BitIndex_t _dwPortBit;
    unsigned short valid_frames;

    bool _PollingMode;
    DWORD _dwMask;

    bool Write_Impl(const void *data, size_t size) override;
};
#elif __linux__
    #include "TTYPort.h"
    typedef TTYPort SerialPort;
#else
#warning "No Com Port in this platform"
    typedef NullComPort SerialPort;
#endif

#endif	/* SERIALPORT_H */
