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
#include <stdarg.h>
#include "ComPort.h"
#include <iterator>
#include <algorithm>
#include "Dialogs.h"
#include <functional>
#include "Poco/RunnableAdapter.h"

ComPort::ComPort(int idx, const std::tstring& sName) : devIdx(idx), sPortName(sName) {
    pLastNmea = std::begin(_NmeaString);
}

ComPort::~ComPort() {

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

// this is used by all functions to send data out
// it is called internally from thread for each device
void ComPort::WriteString(const TCHAR * Text) {
#if TESTBENCH && (WINDOWSPC>0)
StartupStore(_T("... ComPort write discarded: <%s>\n"),Text);
return;
#else
    int len = _tcslen(Text);
#ifdef  _UNICODE
    int size_needed = WideCharToMultiByte(CP_ACP, 0, Text, len+1, NULL, 0, NULL, NULL);
    char* szTmp = new char[size_needed];
    len = WideCharToMultiByte(CP_ACP, 0, Text, len+1, szTmp, size_needed, NULL, NULL);
#else
    const char* szTmp = Text;
#endif
    // don't write trailing '\0' to device
    if (--len > 0) {
        Write(szTmp, len);
    }
#ifdef _UNICODE
    delete[] szTmp;
#endif
#endif
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
    StopEvt.set();

#ifdef _DEBUG_STOP_RXTHREAD
    StartupStore(_T("... ComPort %d StopRxThread: Cancel Wait Event !%s"), GetPortIndex() + 1, NEWLINE);
#endif
    CancelWaitEvent();

#ifdef _DEBUG_STOP_RXTHREAD
    StartupStore(_T("... ComPort %d StopRxThread: Wait End of thread !%s"), GetPortIndex() + 1, NEWLINE);
#endif
    if (!ReadThread.tryJoin(20000)) {
        StartupStore(_T("... ComPort %d StopRxThread: RX Thread forced to terminate!%s"), GetPortIndex() + 1, NEWLINE);
        // TODO : Kill Thread ??
    }
    StopEvt.reset();

    return TRUE;
}

BOOL ComPort::StartRxThread() {
    StopEvt.reset();

    
    // Create a read thread for reading data from the communication port.
    ReadThread.start(*this);
    ReadThread.setPriority(Poco::Thread::PRIO_NORMAL); //THREAD_PRIORITY_ABOVE_NORMAL

    if(ReadThread.isRunning()) {
        // Could not create the read thread.
        StartupStore(_T(". ComPort %u <%s> Failed to start Rx Thread%s"), GetPortIndex() + 1, GetPortName(), NEWLINE);

        // LKTOKEN  _@M761_ = "Unable to Start RX Thread on Port"
        StatusMessage(mbOk, TEXT("Error"), TEXT("%s %s"), gettext(TEXT("_@M761_")), GetPortName());
        //DWORD dwError = GetLastError();
        return FALSE;
    }
    return FALSE;
}

void ComPort::run() {
    StartupStore(_T(". ComPort %d ReadThread : started%s"), GetPortIndex() + 1, NEWLINE);
    RxThread();
    StartupStore(_T(". ComPort %d ReadThread : terminated%s"), GetPortIndex() + 1, NEWLINE);
}

void ComPort::ProcessChar(char c) {
    // last char need to be reserved for '\0' for avoid buffer overflow
    // in theory this should never happen because NMEA sentence can't have more than 82 char and _NmeaString size is 160.
    if (pLastNmea >= std::begin(_NmeaString) && (pLastNmea+1) < std::end(_NmeaString)) {

        if (c == '\n' || c == '\r') {
            // abcd\n , now closing the line also with \r
            *(pLastNmea++) = _T('\n');
            *(pLastNmea) = _T('\0'); // terminate string.
            // process only meaningful sentences, avoid processing a single \n \r etc.
            if (pLastNmea - std::begin(_NmeaString) > 5) {
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
    pLastNmea = std::begin(_NmeaString);
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

void ComPort::StatusMessage(MsgType_t type, const TCHAR *caption, const TCHAR *fmt, ...) {
    TCHAR tmp[127];
    va_list ap;
    LKASSERT(fmt!=NULL);

    va_start(ap, fmt);
    int n = _vsntprintf(tmp, 127, fmt, ap);
    va_end(ap);

#ifdef TESTBENCH
    LKASSERT(n>=0); // Message to long for "tmp" buffer
#endif


    tmp[126] = _T('\0');

    if (caption) {
        MessageBoxX(tmp, gettext(caption), type);
    } else {
        DoStatusMessage(tmp);
    }
}
