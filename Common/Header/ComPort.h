/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
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
#include "boost/noncopyable.hpp"
#include "string"

class ComPort : public boost::noncopyable {
public:
    ComPort(int idx, const std::wstring& sName);
    virtual ~ComPort();

    BOOL StopRxThread();
    BOOL StartRxThread();

    inline LPCTSTR GetPortName() const {
        return sPortName.c_str();
    }

    inline size_t GetPortIndex() const {
        return devIdx;
    }

    virtual bool Initialize();
    virtual bool Close();

    virtual void Flush() = 0;
    virtual void Purge() = 0;
    virtual void CancelWaitEvent() = 0;

    virtual int SetRxTimeout(int TimeOut) = 0;
    virtual unsigned long SetBaudrate(unsigned long) = 0;
    virtual unsigned long GetBaudrate() const = 0;

    virtual void UpdateStatus() = 0;

    virtual bool Write(const void *data, size_t length) = 0;
    virtual size_t Read(void *szString, size_t size) = 0;

    void WriteString(const TCHAR *);

    void PutChar(BYTE);
    int GetChar();

protected:
    typedef char _Buff_t[1024];

    inline size_t ReadData(_Buff_t& szString) {
        return Read(szString, sizeof (szString));
    }

    void StatusMessage(UINT type, const TCHAR *caption, const TCHAR *fmt, ...);

    void AddStatRx(DWORD dwBytes);
    void AddStatErrRx(DWORD dwBytes);
    void AddStatTx(DWORD dwBytes);
    void AddStatErrTx(DWORD dwBytes);
    void AddStatErrors(DWORD dwBytes);
    void SetPortStatus(int Status);

    virtual DWORD RxThread() = 0;

    void ProcessChar(char c);

    HANDLE hStop;
    HANDLE hReadThread;
    
private:
    typedef TCHAR _NmeaString_t[MAX_NMEA_LEN];

    static DWORD WINAPI RxThreadProc(LPVOID);

    size_t devIdx;
    std::wstring sPortName;

    _NmeaString_t _NmeaString;
    TCHAR * pLastNmea;
};

#endif	/* COMPORT_H */
