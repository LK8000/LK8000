/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#endif


class SocketPort : public ComPort {
public:
    SocketPort(int idx, const tstring& sName);
    ~SocketPort();
    
    bool Initialize() override;
    bool Close() override;

    void Flush() override {};
    void Purge() override {};
    void CancelWaitEvent() override {};

    bool IsReady() override { 
        return mSocket != INVALID_SOCKET;
    }

    int SetRxTimeout(int TimeOut) override;
    unsigned long SetBaudrate(unsigned long) override { return 0U; }
    unsigned long GetBaudrate() const override {  return 0U; }

    void UpdateStatus() override {};

    bool Write(const void *data, size_t size) override;
    size_t Read(void *szString, size_t size) override;

protected:
    virtual bool Connect() = 0;
    unsigned RxThread() override;

    SOCKET mSocket;
    unsigned mTimeout;    
};

#endif /* SOCKETPORT_H */

