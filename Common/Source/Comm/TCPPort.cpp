/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   TCPPort.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on February 22, 2016, 10:27 PM
 */
#include "externs.h"
#include "TCPPort.h"
#include "utils/stringext.h"

#ifdef linux
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#endif

static LPCTSTR GetAddress(unsigned idx) {
    switch (idx) {
        case 0:
            return szIpAddress1;
        case 1:
            return szIpAddress2;
    }
    return _T("");
}

static uint16_t GetPort(unsigned idx) {
    switch (idx) {
        case 0:
            return dwIpPort1;
        case 1:
            return dwIpPort2;
    }
    return 0U;
}


bool TCPClientPort::Connect() {
    
    mSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mSocket == INVALID_SOCKET) {
        unsigned dwError = WSAGetLastError();
        StartupStore(_T("... TCPClientPort Port %u Unable to create socket, error=%u") NEWLINE, (unsigned)GetPortIndex() + 1, dwError); // 091117

        return false;
    }

#ifdef UNICODE    
    char szaddr[16];
    TCHAR2ascii(GetAddress(GetPortIndex()), szaddr, 16);
#else
    const char* szaddr = GetAddress(GetPortIndex());
#endif
    
    SOCKADDR_IN sin = { 0 };
    sin.sin_addr.s_addr = inet_addr(szaddr);
    sin.sin_port = htons(GetPort(GetPortIndex()));
    sin.sin_family = AF_INET;

    if(connect(mSocket, (SOCKADDR*)&sin, sizeof(sin)) == SOCKET_ERROR) {
        unsigned dwError = WSAGetLastError();
        StartupStore(_T("... TCPClientPort Port %u <%s> connect Failed, error=%u") NEWLINE, (unsigned)GetPortIndex() + 1, GetPortName(), dwError); // 091117

        return false;
    }

    StartupStore(_T(". TCPClientPort %u Connect <%s> OK") NEWLINE, (unsigned)GetPortIndex() + 1, GetPortName());

    return true;
}

bool TCPServerPort::Close() {
    if(!SocketPort::Close()) {
        return false;
    }
    closesocket(mServerSocket);
    mServerSocket = INVALID_SOCKET;
        
    return true;
}

bool TCPServerPort::Connect() {

    mServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mServerSocket == INVALID_SOCKET) {
        unsigned dwError = WSAGetLastError();
        StartupStore(_T("... TCPServerPort Port %u Unable to create socket, error=%u") NEWLINE, (unsigned)GetPortIndex() + 1, dwError); // 091117

        return false;
    }

    SOCKADDR_IN sin = { 0 };
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(GetPort(GetPortIndex()));
    sin.sin_family = AF_INET;
    
    int reuse = 1;
    if (setsockopt(mServerSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(int)) == SOCKET_ERROR) {
        
    }
    
    if(bind (mServerSocket, (SOCKADDR *)&sin, sizeof(sin)) == SOCKET_ERROR) {
        unsigned dwError = WSAGetLastError();
        StartupStore(_T("... TCPServerPort %u <%s> Bind Failed, error=%u") NEWLINE, (unsigned)GetPortIndex() + 1, GetPortName(), dwError); // 091117

        return false;
    }
    
    if(listen(mServerSocket, 1) == SOCKET_ERROR) {
        unsigned dwError = WSAGetLastError();
        StartupStore(_T("... TCPServerPort %u <%s> Listen Failed, error=%u") NEWLINE, (unsigned)GetPortIndex() + 1, GetPortName(), dwError); // 091117

        return false;
    }

    StartupStore(_T(". TCPServerPort %u Connect <%s> OK") NEWLINE, (unsigned)GetPortIndex() + 1, GetPortName());

    return true;
}

unsigned TCPServerPort::RxThread() {
    struct timeval timeout;
    timeout.tv_sec = (mTimeout) / 1000;
    timeout.tv_usec = (mTimeout)  % 1000;
    
    //-------------------------
    // Set the socket I/O mode: In this case FIONBIO
    // enables or disables the blocking mode for the 
    // socket based on the numerical value of iMode.
    // If iMode = 0, blocking is enabled; 
    // If iMode != 0, non-blocking mode is enabled.

    u_long iMode = 1;
    int iResult = ioctlsocket(mServerSocket, FIONBIO, &iMode);
    if (iResult == SOCKET_ERROR) {
        StartupStore(_T(".... ioctlsocket failed with error: %d%s"), iResult, NEWLINE);
        // if failed, socket still in blocking mode, it's big problem
    }  
    
    while (mServerSocket != INVALID_SOCKET && !StopEvt.tryWait(5)) {

        fd_set readfs;
        FD_ZERO(&readfs);
        FD_SET(mServerSocket, &readfs);
        // wait for received data
        int iResult = select(mServerSocket + 1, &readfs, NULL, NULL, &timeout);
        if(iResult > 0 && FD_ISSET(mServerSocket, &readfs)) {
            
            /* new client */
            mSocket = accept(mServerSocket, nullptr, nullptr);
            if(mSocket != INVALID_SOCKET) {
                SocketPort::RxThread();
            }
            if(mSocket != INVALID_SOCKET) {
                closesocket(mSocket);
                mSocket = INVALID_SOCKET;
            }
        }
    }
    return 0U;
}
