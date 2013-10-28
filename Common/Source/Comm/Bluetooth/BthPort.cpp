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
#include "BtHandler.h"
#include "utils/stl_utils.h"
#include <algorithm>
#include <functional>

BthPort::BthPort(int idx, const std::wstring& sName) : ComPort(idx, sName), mSocket(INVALID_SOCKET) {

}

BthPort::~BthPort() {
    Close();
}

bool BthPort::Initialize() {
    int iResult;

    WSADATA wsd;
    WSAStartup(MAKEWORD(2, 0), &wsd);

    mSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    if (mSocket == INVALID_SOCKET) {
        DWORD dwError = WSAGetLastError();
        StartupStore(_T("... Bluetooth Port %u Unable to create socket, error=%u%s"), GetPortIndex() + 1, dwError, NEWLINE); // 091117

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
        StartupStore(_T("... Bluetooth Port %u <%s> Unable connect, error=%u%s"), GetPortIndex() + 1, GetPortName(), dwError, NEWLINE); // 091117

        goto failed;
    }

    // max string receive rate is less to 20Hz
    if( !SetRxTimeout(10) ) {
        // if SetRxTimeout failed, use non-blocking socket
        StartupStore(_T("... use non-blocking socket%s"), NEWLINE);                

        //-------------------------
        // Set the socket I/O mode: In this case FIONBIO
        // enables or disables the blocking mode for the 
        // socket based on the numerical value of iMode.
        // If iMode = 0, blocking is enabled; 
        // If iMode != 0, non-blocking mode is enabled.

        u_long iMode = 1;
        iResult = ioctlsocket(mSocket, FIONBIO, &iMode);
        if (iResult != NO_ERROR) {
            StartupStore(_T(".... ioctlsocket failed with error: %ld%s"), iResult, NEWLINE);
            // if failed, socket still in blocking mode, it's big problem
	        goto failed;
        }
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

    DWORD dwTimeOut = (DWORD) TimeOut;
    int iResult = setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, (char*) &dwTimeOut, sizeof (DWORD));
    if (iResult == SOCKET_ERROR) {
        StartupStore(_T(". ComPort %d : setsockopt failed <0x%X>.%s"), GetPortIndex() + 1, WSAGetLastError(), NEWLINE);                
        return 0;
    }
    return 1;
}

size_t BthPort::Read(void *szString, size_t size) {
    int iResult = recv(mSocket, (char*) szString, size, 0);
    if (iResult > 0) {
        AddStatRx(iResult);
        return iResult;
    } if(iResult == SOCKET_ERROR) {
        DWORD dwError = WSAGetLastError();
        if(WSAEWOULDBLOCK != dwError) {
            StartupStore(_T("ComPort %d : recv failed <0x%X>.%s"), GetPortIndex() + 1, dwError, NEWLINE);
        }
    } else {
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
    WSACleanup();
    StartupStore(_T(". ComPort %u closed Ok.%s"), GetPortIndex() + 1, NEWLINE); // 100210 BUGFIX missing
    return true;
}

bool BthPort::Write(const void *data, size_t length) {
    int iResult = send(mSocket, (char*) data, length, 0);
    if (iResult == SOCKET_ERROR) {
        AddStatErrTx(1);
        AddStatErrors(1);
        StartupStore(_T("ComPort %d : socket was forcefully disconnected.%s"), GetPortIndex() + 1, NEWLINE);
        closesocket(mSocket);
        mSocket = INVALID_SOCKET;

        return false;
    }
    AddStatTx(iResult);

    return true;
}

extern void Cpustats(int *acc, FILETIME *a, FILETIME *b, FILETIME *c, FILETIME *d);

DWORD BthPort::RxThread() {
    DWORD dwWaitTime = 0;
    _Buff_t szString;
    FILETIME CreationTime, ExitTime, StartKernelTime, EndKernelTime, StartUserTime, EndUserTime;
    Purge();

    while (mSocket != INVALID_SOCKET && ::WaitForSingleObject(hStop, dwWaitTime) == WAIT_TIMEOUT) {
        GetThreadTimes(hReadThread, &CreationTime, &ExitTime, &StartKernelTime, &StartUserTime);
        UpdateStatus();

        int nRecv = ReadData(szString);
        if (nRecv > 0) {
            std::for_each(begin(szString), begin(szString) + nRecv, std::bind1st(std::mem_fun(&BthPort::ProcessChar), this));
            dwWaitTime = 5; // avoid cpu overhead;
        } else {
            dwWaitTime = 50; // if no more data wait 50ms ( max data rate 20Hz )
        }
    
        if ((GetThreadTimes(hReadThread, &CreationTime, &ExitTime, &EndKernelTime, &EndUserTime)) == 0) {
            if (GetPortIndex() == 0)
                Cpu_PortA = 9999;
            else
                Cpu_PortB = 9999;
        } else {
            Cpustats((GetPortIndex() == 0) ? &Cpu_PortA : &Cpu_PortB, &StartKernelTime, &EndKernelTime, &StartUserTime, &EndUserTime);
        }
    }

    return 0UL;
}
