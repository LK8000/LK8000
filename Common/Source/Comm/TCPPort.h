/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   TCPPort.h
 * Author: Bruno de Lacheisserie
 *
 * Created on February 22, 2016, 10:27 PM
 */

#ifndef TCPPORT_H
#define TCPPORT_H
#include "SocketPort.h"
#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>  // typedef int socklen_t;
// WSAGetLastError is alias of GetLastError, WSAGetLastError is not exported by winsok.dll
// and ws2.dll not exist on PPC2002 device.
//#define WSAGetLastError GetLastError
#else
#include <netinet/in.h>
#endif

class TCPClientPort : public SocketPort {
public:
    TCPClientPort(int idx, const tstring& sName) : SocketPort(idx, sName) { }
    
protected:
    virtual bool Connect();
};

class TCPServerPort : public SocketPort {
public:
    TCPServerPort(int idx, const tstring& sName) : SocketPort(idx, sName),  mServerSocket(INVALID_SOCKET) { }
    
    virtual bool Close();
    virtual int SetRxTimeout(int TimeOut) { return 0; }
protected:
    virtual bool Connect();
    
    virtual unsigned RxThread();
    
    SOCKET mServerSocket;
};

class UDPServerPort : public SocketPort {
public:

	UDPServerPort(int idx, const tstring& sName) : SocketPort(idx, sName) {
		mSAddressClient =  { 0 };
	}

    virtual int SetRxTimeout(int TimeOut) { return 0; }
protected:
    virtual bool Connect();

    virtual unsigned RxThread();
    virtual bool Write(const void *data, size_t length);

	SOCKADDR_IN mSAddressClient;

};

#endif /* TCPPORT_H */
