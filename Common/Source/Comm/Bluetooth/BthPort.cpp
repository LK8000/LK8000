/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   BthPort.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 2 août 2013, 14:37
 */

#include "externs.h"
#include "BthPort.h"

#ifndef NO_BLUETOOTH

#include "BtHandler.h"
#include "utils/stl_utils.h"
#include <algorithm>
#include <tr1/functional>

using namespace std::tr1::placeholders;

BthPort::BthPort(int idx, const std::tstring& sName) : ComPort(idx, sName), mSocket(INVALID_SOCKET), mTimeout(40) {
    WSADATA wsd;
    WSAStartup(MAKEWORD(1, 1), &wsd);
}

BthPort::~BthPort() {
    if (mSocket != INVALID_SOCKET) {
        Close();
    }
    WSACleanup();
}

bool BthPort::Initialize() {
    int iResult;

    mSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    if (mSocket == INVALID_SOCKET) {
        DWORD dwError = WSAGetLastError();
        StartupStore(_T("... Bluetooth Port %u Unable to create socket, error=%lu%s"), GetPortIndex() + 1, dwError, NEWLINE); // 091117

        goto failed;
    }

    SOCKADDR_BTH sa;
    memset(&sa, 0, sizeof (sa));
    sa.addressFamily = AF_BTH;
    sa.serviceClassId = RFCOMM_PROTOCOL_UUID;
    sa.btAddr = StrToBTAddr(GetPortName());

    iResult = connect(mSocket, (SOCKADDR*) & sa, sizeof (sa));
    if (iResult == SOCKET_ERROR) {
        DWORD dwError = WSAGetLastError();
        StartupStore(_T("... Bluetooth Port %u <%s> Unable connect, error=%lu%s"), GetPortIndex() + 1, GetPortName(), dwError, NEWLINE); // 091117

        goto failed;
    }

    if (SetRxTimeout(RXTIMEOUT) == -1) {
        DWORD dwError = GetLastError();
        StartupStore(_T("... ComPort %u Init <%s> change TimeOut FAILED, error=%lu%s"), GetPortIndex() + 1, GetPortName(), dwError, NEWLINE); // 091117
        // LKTOKEN  _@M760_ = "Unable to Set Serial Port Timers" 
        StatusMessage(MB_OK, TEXT("Error"), TEXT("%s %s"), gettext(TEXT("_@M760_")), GetPortName());        

        goto failed;
    }
    
    if(!ComPort::Initialize()) {
        // no need to log failed of StartRxThread it's already made in ComPort::Initialize();
        goto failed;
    }

    StartupStore(_T(". Bluetooth Port %u Init <%s> end OK%s"), GetPortIndex() + 1, GetPortName(), NEWLINE);
    return true;
failed:
    StatusMessage(MB_OK, NULL, TEXT("%s %s"), gettext(TEXT("_@M762_")), GetPortName());

    // Failed to initialize
    if (mSocket != INVALID_SOCKET) {
        closesocket(mSocket);
    }
    mSocket = INVALID_SOCKET;
    return false;
}

int BthPort::SetRxTimeout(int TimeOut) {

    DWORD dwTimeout = mTimeout;
    mTimeout = (DWORD)TimeOut;

    //-------------------------
    // Set the socket I/O mode: In this case FIONBIO
    // enables or disables the blocking mode for the 
    // socket based on the numerical value of iMode.
    // If iMode = 0, blocking is enabled; 
    // If iMode != 0, non-blocking mode is enabled.

    u_long iMode = 1;
    int iResult = ioctlsocket(mSocket, FIONBIO, &iMode);
    if (iResult != NO_ERROR) {
        StartupStore(_T(".... ioctlsocket failed with error: %d%s"), iResult, NEWLINE);
        // if failed, socket still in blocking mode, it's big problem
        dwTimeout = -1;
    }

    return dwTimeout;
}

size_t BthPort::Read(void *szString, size_t size) {

    struct timeval timeout;
    struct fd_set readfs;
    timeout.tv_sec = mTimeout / 1000;
    timeout.tv_usec = mTimeout % 1000;

    int iResult = 0;
    FD_ZERO(&readfs);
    FD_SET(mSocket, &readfs);
    
    // wait for received data
    iResult = select(mSocket + 1, &readfs, NULL, NULL, &timeout); 
    if (iResult == 0) {
        return 0U; // timeout
    }
    
    if ((iResult != SOCKET_ERROR) && FD_ISSET(mSocket, &readfs)) {
        // Data ready to read 
        iResult = recv(mSocket, (char*) szString, size, 0);
        if (iResult > 0) {
            AddStatRx(iResult);
            return iResult;
        }
    }

    if(iResult == SOCKET_ERROR) {
        AddStatErrRx(1);
        StartupStore(_T("ComPort %d : socket was forcefully disconnected <0x%X>.%s"), GetPortIndex() + 1, WSAGetLastError(), NEWLINE);
        closesocket(mSocket);
        mSocket = INVALID_SOCKET;
    }
    return 0U;
}

bool BthPort::Close() {
    ComPort::Close();
    if (mSocket != INVALID_SOCKET) {
        closesocket(mSocket);
        mSocket = INVALID_SOCKET;
    }
    StartupStore(_T(". ComPort %u closed Ok.%s"), GetPortIndex() + 1, NEWLINE); // 100210 BUGFIX missing
    return true;
}

bool BthPort::Write(const void *data, size_t length) {
    struct timeval timeout;
    struct fd_set writefs;
    timeout.tv_sec = mTimeout / 1000;
    timeout.tv_usec = mTimeout % 1000;

    int iResult = 0;
    FD_ZERO(&writefs);
    FD_SET(mSocket, &writefs);
    
    // wait for socket ready to write
    iResult = select(mSocket + 1, NULL, &writefs, NULL, &timeout); 
    if (iResult == 0) {
        StartupStore(_T("ComPort %d : write to socket timeout.%s"), GetPortIndex() + 1, NEWLINE);
        return false; // timeout
    }
    
    if ((iResult != SOCKET_ERROR) && FD_ISSET(mSocket, &writefs)) {
        // socket ready, Write data.
        iResult = send(mSocket, (const char*) data, length, 0);
        if (iResult > 0) {
            AddStatTx(iResult);
            return true;
        }
    }
    
    if (iResult == SOCKET_ERROR) {
        AddStatErrTx(1);
        AddStatErrors(1);
        StartupStore(_T("ComPort %d : socket was forcefully disconnected.%s"), GetPortIndex() + 1, NEWLINE);
        closesocket(mSocket);
        mSocket = INVALID_SOCKET;

        return false;
    }

    return true;
}

extern void Cpustats(int *acc, FILETIME *a, FILETIME *b, FILETIME *c, FILETIME *d);

DWORD BthPort::RxThread() {
    DWORD dwWaitTime = 0;
    _Buff_t szString;
    Purge();

    while (mSocket != INVALID_SOCKET && !StopEvt.tryWait(dwWaitTime)) {

        UpdateStatus();

        int nRecv = ReadData(szString);
        if (nRecv > 0) {
            std::for_each(begin(szString), begin(szString) + nRecv, std::tr1::bind(&BthPort::ProcessChar, this, _1));
            dwWaitTime = 5; // avoid cpu overhead;
        } else {
            dwWaitTime = 50; // if no more data wait 50ms ( max data rate 20Hz )
        }
    }

    return 0UL;
}
#endif
