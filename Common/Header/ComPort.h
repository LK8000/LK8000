/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   ComPort.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 28 juillet 2013, 16:15
 */

#ifndef COMPORT_H
#define	COMPORT_H

#include "Sizes.h"
#include "Enums.h"
#include "Util/tstring.hpp"
#include "Poco/Event.h"
#include "Poco/Thread.h"

class ComPort : public Poco::Runnable {
public:
    ComPort(int idx, const tstring& sName);
    virtual ~ComPort();

    ComPort( const ComPort& ) = delete;
    ComPort& operator=( const ComPort& ) = delete;

    ComPort( ComPort&& ) = delete;
    ComPort& operator=( ComPort&& ) = delete;

    virtual bool StopRxThread();
    virtual bool StartRxThread();

    inline LPCTSTR GetPortName() const {
        return sPortName.c_str();
    }

    inline size_t GetPortIndex() const {
        return devIdx;
    }

    virtual bool Initialize() = 0;
    virtual bool Close();

    virtual void Flush() = 0;
    virtual void Purge() = 0;
    virtual void CancelWaitEvent() = 0;

    virtual int SetRxTimeout(int TimeOut) = 0;
    virtual unsigned long SetBaudrate(unsigned long) = 0;
    virtual unsigned long GetBaudrate() const = 0;

    virtual void UpdateStatus() = 0;

    virtual bool Write(const void *data, size_t size) = 0;

    inline void Write(uint8_t b) {
        Write(&b, sizeof(b));
    }

#ifdef UNICODE
    void WriteString(const TCHAR* Text) gcc_nonnull_all;
#endif
    void WriteString(const char* Text) gcc_nonnull_all;


    virtual size_t Read(void *szString, size_t size) = 0;

    int GetChar();

protected:
    typedef char _Buff_t[1024];

    inline size_t ReadData(_Buff_t& szString) {
        return Read(szString, sizeof (szString));
    }

    static
    void StatusMessage(MsgType_t type, const TCHAR *caption, const TCHAR *fmt, ...)
            gcc_printf(3,4) gcc_nonnull(3);

    void AddStatRx(unsigned dwBytes);
    void AddStatErrRx(unsigned dwBytes);
    void AddStatTx(unsigned dwBytes);
    void AddStatErrTx(unsigned dwBytes);

    void SetPortStatus(int Status);

    virtual unsigned RxThread() = 0;

    void ProcessChar(char c);

    Poco::Event StopEvt;
    Poco::Thread ReadThread;

private:
    typedef TCHAR _NmeaString_t[MAX_NMEA_LEN];

    void run();

    const size_t devIdx;
    const tstring sPortName;

    _NmeaString_t _NmeaString;
    TCHAR * pLastNmea;
};

#endif	/* COMPORT_H */
