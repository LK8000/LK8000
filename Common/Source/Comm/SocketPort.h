/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   SocketPort.h
 * Author: Bruno de Lacheisserie
 *
 * Created on February 22, 2016, 8:29 PM
 */

#ifndef SOCKETPORT_H
#define SOCKETPORT_H
#include "ComPort.h"

#ifdef WIN32
#ifdef PPC2002
#include <winsock.h>
// WSAGetLastError is alias of GetLastError, WSAGetLastError is not exported by winsok.dll
// and ws2.dll not exist on PPC2002 device.
//#define WSAGetLastError GetLastError
#else
#include <winsock2.h>
#endif
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
#define ioctlsocket ioctl

inline unsigned WSAGetLastError() { return errno; }
inline unsigned GetLastError() { return errno; }

typedef int SOCKET;

#endif


class SocketPort : public ComPort {
public:
    SocketPort(int idx, const std::tstring& sName);
    virtual ~SocketPort();
    
    virtual bool Initialize();
    virtual bool Close();    

    virtual void Flush() {};
    virtual void Purge() {};
    virtual void CancelWaitEvent() {};

    virtual int SetRxTimeout(int TimeOut);
    virtual unsigned long SetBaudrate(unsigned long) { return 0U; }
    virtual unsigned long GetBaudrate() const  {  return 0U; }

    virtual void UpdateStatus() {};

    virtual bool Write(const void *data, size_t length);
    virtual size_t Read(void *szString, size_t size);

protected:
    virtual bool Connect() = 0;
    virtual unsigned RxThread();

    SOCKET mSocket;
    unsigned mTimeout;    
};

#endif /* SOCKETPORT_H */

