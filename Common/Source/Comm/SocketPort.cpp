/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   SocketPort.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on February 22, 2016, 8:29 PM
 */
#include "externs.h"
#include "SocketPort.h"
#include <functional>
using namespace std::placeholders;

SocketPort::SocketPort(unsigned idx, const tstring& sName) : ComPort(idx, sName), mSocket(INVALID_SOCKET), mTimeout(40) {
#ifdef WIN32
    WSADATA wsd;
    WSAStartup(MAKEWORD(1, 1), &wsd);
#endif
}

SocketPort::~SocketPort() {
    if (mSocket != INVALID_SOCKET) {
        Close();
    }
#ifdef WIN32
    WSACleanup();
#endif
}

bool SocketPort::Initialize() {

    if (!Connect()) {
        goto failed;
    }

    if (SetRxTimeout(RXTIMEOUT) == -1) {
        unsigned dwError = GetLastError();
        StartupStore(_T("... ComPort %u Init <%s> change TimeOut FAILED, error=%u"), GetPortIndex() + 1, GetPortName(), dwError); // 091117
        // LKTOKEN  _@M760_ = "Unable to Set Serial Port Timers" 
        StatusMessage(_T("%s %s"), MsgToken<760>(), GetPortName());        

        goto failed;
    }

    SetPortStatus(CPS_OPENOK);
    StartupStore(_T(". ComPort %u Init <%s> end OK"), GetPortIndex() + 1, GetPortName());
    return true;
failed:
    StatusMessage(_T("%s %s"), MsgToken<762>(), GetPortName());

    // Failed to initialize
    if (mSocket != INVALID_SOCKET) {
        closesocket(mSocket);
    }
    mSocket = INVALID_SOCKET;
    return false;
}

int SocketPort::SetRxTimeout(int TimeOut) {

    unsigned dwTimeout = mTimeout;
    mTimeout = TimeOut;
    return dwTimeout;
}

size_t SocketPort::Read(void *data, size_t size) {

    if(mSocket == INVALID_SOCKET) {
        return false; // socket not connect,that can happen with TCPServer Port.
    }

    struct timeval timeout;
    fd_set readfs;
    timeout.tv_sec = mTimeout / 1000;
    timeout.tv_usec = mTimeout % 1000;

    FD_ZERO(&readfs);
    FD_SET(mSocket, &readfs);
    
    // wait for received data
    int iResult = select(mSocket + 1, &readfs, NULL, NULL, &timeout); 
    if (iResult == 0) {
        return 0U; // timeout
    }
    
    if ((iResult != SOCKET_ERROR) && FD_ISSET(mSocket, &readfs)) {
        // Data ready to read 
        iResult = recv(mSocket, static_cast<char*>(data), size, 0);
        if (iResult > 0) {
            AddStatRx(iResult);
            return iResult;
        } else if (iResult == 0) {
            // no data : socket disconnected ?
            iResult = SOCKET_ERROR;
        }
    }

    if(iResult == SOCKET_ERROR) {
        AddStatErrRx(1);
        StartupStore(_T("ComPort %u : socket was forcefully disconnected <%d>."), GetPortIndex() + 1, WSAGetLastError());
        closesocket(mSocket);
        mSocket = INVALID_SOCKET;
    }
    return 0U;
}

bool SocketPort::Close() {
    ComPort::Close();
    if (mSocket != INVALID_SOCKET) {
        closesocket(mSocket);
        mSocket = INVALID_SOCKET;
    }
    StartupStore(_T(". ComPort %u closed Ok."), GetPortIndex() + 1); // 100210 BUGFIX missing
    return true;
}

bool SocketPort::Write_Impl(const void *data, size_t size) {
    if(mSocket == INVALID_SOCKET) {
        return false; // socket not connect,that can happen with TCPServer Port.
    }
    
    struct timeval timeout;
    fd_set writefs;
    timeout.tv_sec = mTimeout / 1000;
    timeout.tv_usec = mTimeout % 1000;

    int iResult = 0;
    FD_ZERO(&writefs);
    FD_SET(mSocket, &writefs);
    
    // wait for socket ready to write
    iResult = select(mSocket + 1, NULL, &writefs, NULL, &timeout); 
    if (iResult == 0) {
        AddStatErrTx(1);
        return false; // timeout
    }
    
    if ((iResult != SOCKET_ERROR) && FD_ISSET(mSocket, &writefs)) {
        // socket ready, Write data.
        
#ifdef __linux        
        const int flags = MSG_NOSIGNAL; // avoid SIGPIPE if socket is closed by peer.
#else
        const int flags = 0;
#endif
        
        iResult = send(mSocket, (const char*) data, size, flags);
        if (iResult > 0) {
            AddStatTx(iResult);
            return true;
        }
    }
    
    if (iResult == SOCKET_ERROR) {
        AddStatErrTx(1);
        StartupStore(_T("ComPort %u : socket was forcefully disconnected."), GetPortIndex() + 1);
        closesocket(mSocket);
        mSocket = INVALID_SOCKET;

        return false;
    }

    return true;
}

unsigned SocketPort::RxThread() {
    char szString[1024];
    Purge();
    
        
    //-------------------------
    // Set the socket I/O mode: In this case FIONBIO
    // enables or disables the blocking mode for the 
    // socket based on the numerical value of iMode.
    // If iMode = 0, blocking is enabled; 
    // If iMode != 0, non-blocking mode is enabled.

    u_long iMode = 1;
    int iResult = ioctlsocket(mSocket, FIONBIO, &iMode);
    if (iResult == SOCKET_ERROR) {
        StartupStore(_T(".... ioctlsocket failed with error: %d%s"), iResult, NEWLINE);
        // if failed, socket still in blocking mode, it's big problem
    }

    while (mSocket != INVALID_SOCKET && !StopEvt.tryWait(5)) {

        ScopeLock Lock(CritSec_Comm);
        UpdateStatus();
        int nRecv = ComPort::Read(szString);
        if (nRecv > 0) {
            ProcessData(szString, nRecv);
        }
    }

    return 0UL;
}
