/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   ComPort.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 28 juillet 2013, 16:15
 */
#include "externs.h"
#include "ComPort.h"
#include "utils/stl_utils.h"
#include <algorithm>
#include "dlgTools.h"
#include <functional>

ComPort::ComPort(int idx, const std::wstring& sName) : devIdx(idx), sPortName(sName) {
    pLastNmea = begin(_NmeaString);

    hReadThread = INVALID_HANDLE_VALUE;
    hStop = INVALID_HANDLE_VALUE;
}

ComPort::~ComPort() {
    Close();
}

bool ComPort::Initialize() {
    if(!StartRxThread()) {
        StartupStore(_T(". ComPort %u <%s> Failed to start Rx Thread%s"), GetPortIndex() + 1, GetPortName(), NEWLINE);
        return false;
    }
    return true;
}

bool ComPort::Close() {
    StopRxThread();
    return true;
}

void ComPort::WriteString(const TCHAR * Text) {
    int len = _tcslen(Text);
#ifdef  _UNICODE
    int size_needed = WideCharToMultiByte(CP_ACP, 0, Text, len, NULL, 0, NULL, NULL);
    char* szTmp = new char[size_needed];
    len = WideCharToMultiByte(CP_ACP, 0, Text, len, szTmp, size_needed, NULL, NULL);
#else
    const char* szTmp = Text;
#endif
    // don't write trailing '\0' to device
    if (--len > 0) {
        Write(szTmp, len);
    }
    delete szTmp;
}

int ComPort::GetChar() {
    char c = EOF;
    if (Read(&c, 1) == 1) {
        return c;
    }
    return EOF;
}

void ComPort::PutChar(BYTE b) {
    Write(&b, sizeof (b));
}

BOOL ComPort::StopRxThread() {
    if ((hStop != INVALID_HANDLE_VALUE) && (hReadThread != INVALID_HANDLE_VALUE)) {
        SetEvent(hStop);
        CancelWaitEvent();
        
        if (::WaitForSingleObject(hReadThread, 20000) == WAIT_TIMEOUT) {
            TerminateThread(hReadThread, 0);
            StartupStore(_T("... ComPort %d StopRxThread: RX Thread forced to terminate!%s"), GetPortIndex() + 1, NEWLINE);
        }
        CloseHandle(hReadThread);
        hReadThread = INVALID_HANDLE_VALUE;
        CloseHandle(hStop);
        hStop = INVALID_HANDLE_VALUE;
    }
    return TRUE;
}

BOOL ComPort::StartRxThread() {
    if ((hStop = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL) {
        goto failed;
    }

    DWORD dwThreadID;
    // Create a read thread for reading data from the communication port.
    if ((hReadThread = CreateThread(NULL, 0, RxThreadProc, this, 0, &dwThreadID)) == NULL) {
        goto failed;
    }
    SetThreadPriority(hReadThread, THREAD_PRIORITY_NORMAL); //THREAD_PRIORITY_ABOVE_NORMAL

    return TRUE;

failed:
    // Could not create the read thread.
    StartupStore(_T(". ComPort %u <%s> Failed to start Rx Thread%s"), GetPortIndex() + 1, GetPortName(), NEWLINE);

    // LKTOKEN  _@M761_ = "Unable to Start RX Thread on Port" 
    StatusMessage(MB_OK, TEXT("Error"), TEXT("%s %s"), gettext(TEXT("_@M761_")), GetPortName());
    //DWORD dwError = GetLastError();
    return FALSE;
}

DWORD ComPort::RxThreadProc(LPVOID prt) {
    DWORD ret = 0U; 
    ComPort* pCom = static_cast<ComPort*>(prt);
    if(pCom) {
        StartupStore(_T(". ComPort %d ReadThread : started%s"), pCom->GetPortIndex() + 1, NEWLINE);
        ret = pCom->RxThread();
        StartupStore(_T(". ComPort %d ReadThread : terminated%s"), pCom->GetPortIndex() + 1, NEWLINE);
    }
    return ret; 
}

void ComPort::ProcessChar(char c) {
    if (pLastNmea >= begin(_NmeaString) && pLastNmea < end(_NmeaString)) {

        if (c == '\n' || c == '\r') {
            // abcd\n , now closing the line also with \r
            *(pLastNmea++) = _T('\n');
            *(pLastNmea) = _T('\0'); // terminate string.
            // process only meaningful sentences, avoid processing a single \n \r etc.
            if (pLastNmea - begin(_NmeaString) > 5) {
                LockFlightData();
                devParseNMEA(devIdx, _NmeaString, &GPS_INFO);
                UnlockFlightData();
            }
        } else {
            *(pLastNmea++) = c;
            return;
        }
    }
    // overflow, so reset buffer
    pLastNmea = begin(_NmeaString);
}

void ComPort::AddStatRx(DWORD dwBytes) {
    if (GetPortIndex() < NUMDEV)
        ComPortRx[GetPortIndex()] += dwBytes;
}

void ComPort::AddStatErrRx(DWORD dwBytes) {
    if (GetPortIndex() < NUMDEV)
        ComPortErrRx[GetPortIndex()] += dwBytes;
}

void ComPort::AddStatTx(DWORD dwBytes) {
    if (GetPortIndex() < NUMDEV) {
        ComPortTx[GetPortIndex()] += dwBytes;
    }
}

void ComPort::AddStatErrTx(DWORD dwBytes) {
    if (GetPortIndex() < NUMDEV) {
        ComPortErrTx[GetPortIndex()] += dwBytes;
    }
}

void ComPort::AddStatErrors(DWORD dwBytes) {
    if (GetPortIndex() < NUMDEV) {
        ComPortErrors[GetPortIndex()] += dwBytes;
    }
}

void ComPort::SetPortStatus(int nStatus) {
    if (GetPortIndex() < NUMDEV) {
        ComPortStatus[GetPortIndex()] = nStatus;
    }
}

void ComPort::StatusMessage(UINT type, const TCHAR *caption, const TCHAR *fmt, ...) {
    TCHAR tmp[127];
    va_list ap;

    va_start(ap, fmt);
    _vsntprintf(tmp, 127, fmt, ap);
    va_end(ap);

    tmp[126] = _T('\0');

    if (caption) {
        MessageBoxX(hWndMainWindow, tmp, gettext(caption), type);
    } else {
        DoStatusMessage(tmp);
    }
}
