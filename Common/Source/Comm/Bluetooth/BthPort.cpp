/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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
#include <functional>


bool BthPort::Connect() {
    int iResult;

    mSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    if (mSocket == INVALID_SOCKET) {
        DWORD dwError = WSAGetLastError();
        StartupStore(_T("... Bluetooth Port %u Unable to create socket, error=%lu%s"), GetPortIndex() + 1, dwError, NEWLINE); // 091117

        return false;
    }

    SOCKADDR_BTH sa = {
        AF_BTH,
        StrToBTAddr(GetPortName()),
        SerialPortServiceClass_UUID,
        0
    };

    iResult = connect(mSocket, (SOCKADDR*) & sa, sizeof (sa));
    if (iResult == SOCKET_ERROR) {
        DWORD dwError = WSAGetLastError();
        StartupStore(_T("... Bluetooth Port %u <%s> Unable connect, error=%lu%s"), GetPortIndex() + 1, GetPortName(), dwError, NEWLINE); // 091117

        return false;
    }

    StartupStore(_T(". Bluetooth Port %u Connect <%s> OK%s"), GetPortIndex() + 1, GetPortName(), NEWLINE);

    return true;
}

#endif
