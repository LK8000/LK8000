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

class TCPClientPort : public SocketPort {
public:
    TCPClientPort(int idx, const std::tstring& sName) : SocketPort(idx, sName) { }
    
protected:
    virtual bool Connect();
};

class TCPServerPort : public SocketPort {
public:
    TCPServerPort(int idx, const std::tstring& sName) : SocketPort(idx, sName),  mServerSocket(INVALID_SOCKET) { }
    
    virtual bool Close();
    virtual int SetRxTimeout(int TimeOut) { return 0; }
protected:
    virtual bool Connect();
    
    virtual unsigned RxThread();
    
    SOCKET mServerSocket;
};

#endif /* TCPPORT_H */
