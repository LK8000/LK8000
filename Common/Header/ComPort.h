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
#include "types.h"
#include "Enums.h"
#include "Util/tstring.hpp"
#include "Poco/Event.h"
#include "Thread/Thread.hpp"
#include "utils/uuid.h"

class ComPort : public Thread {
public:
    ComPort(unsigned idx, const tstring& sName);

    ComPort() = delete;

    ComPort( const ComPort& ) = delete;
    ComPort& operator=( const ComPort& ) = delete;

    ComPort( ComPort&& ) = delete;
    ComPort& operator=( ComPort&& ) = delete;

    virtual bool StopRxThread();
    virtual bool StartRxThread();

    inline LPCTSTR GetPortName() const {
        return sPortName.c_str();
    }

    inline unsigned GetPortIndex() const {
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
    virtual bool IsReady() = 0;

    bool Write(const void *data, size_t size);

    inline bool Write(uint8_t b) {
        return Write(&b, sizeof(b));
    }

#ifdef UNICODE
    bool WriteString(const wchar_t* Text) gcc_nonnull_all;
#endif

    bool WriteString(const std::string_view& str) {
        return Write(str.data(), str.size());
    }

    virtual size_t Read(void* data, size_t size) = 0;

    template<typename T, size_t size>
    size_t Read(T (&data)[size]) {
        return Read(data, size * sizeof(T));
    }

    int GetChar();

protected:

    static
    void StatusMessage(const TCHAR *fmt, ...) gcc_printf(1,2) gcc_nonnull(1);

    void AddStatRx(unsigned dwBytes) const;
    void AddStatErrRx(unsigned dwBytes) const;
    void AddStatTx(unsigned dwBytes) const;
    void AddStatErrTx(unsigned dwBytes) const;

    virtual unsigned RxThread() = 0;

    void ProcessChar(char c);

    void ProcessData(const char* data, size_t size);

    Poco::Event StopEvt;

private:

    using _NmeaString_t = char[MAX_NMEA_LEN];

    void Run() override;

    const unsigned devIdx;
    const tstring sPortName;

    _NmeaString_t _NmeaString;
    char* pLastNmea;

    virtual bool Write_Impl(const void *data, size_t size) = 0;

public:
    virtual void WriteGattCharacteristic(const uuid_t& service, const uuid_t& characteristic, const void *data, size_t size) const  { }
    virtual void ReadGattCharacteristic(const uuid_t& service, const uuid_t& characteristic)  { }

    template<typename Type>
    void WriteGattCharacteristic(const uuid_t& service, const uuid_t& characteristic, const Type& data) const {
        WriteGattCharacteristic(service, characteristic, &data, sizeof(data));
    }

};

#endif	/* COMPORT_H */
