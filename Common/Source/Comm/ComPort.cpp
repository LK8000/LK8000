/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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
#include <stdarg.h>
#include <stdio.h>
#include "Poco/RunnableAdapter.h"
#include "ComCheck.h"

ComPort::ComPort(int idx, const tstring& sName) : StopEvt(false), devIdx(idx), sPortName(sName) {
    pLastNmea = std::begin(_NmeaString);
}

ComPort::~ComPort() {
    assert(!ReadThread.isRunning());
}

bool ComPort::Close() {
    StopRxThread();
    return true;
}

#ifdef  _UNICODE
// this is used by all functions to send data out
// it is called internally from thread for each device
void ComPort::WriteString(const TCHAR * Text) {
#if TESTBENCH && (WINDOWSPC>0) && COM_DISCARD
StartupStore(_T("... ComPort write discarded: <%s>\n"),Text);
return;
#else
    int len = _tcslen(Text);
    int size_needed = WideCharToMultiByte(CP_ACP, 0, Text, len+1, NULL, 0, NULL, NULL);
    char* szTmp = new char[size_needed];
    len = WideCharToMultiByte(CP_ACP, 0, Text, len+1, szTmp, size_needed, NULL, NULL);
    if(len>0) {
        // WideCharToMultiByte return size off Buffer, so trailling '\0' included...
        --len;
    }
    // don't write trailing '\0' to device
    if (len > 0) {
        Write(szTmp, len);
    }
    delete[] szTmp;
#endif
}
#endif

void ComPort::WriteString(const char* Text) {
    const size_t length = strlen(Text);
    if(length > 0) {
        Write(Text, length);
    }
}

int ComPort::GetChar() {
    char c = EOF;
    if (Read(&c, 1) == 1) {
        return c;
    }
    return EOF;
}

bool ComPort::StopRxThread() {

    StopEvt.set();

#ifdef _DEBUG_STOP_RXTHREAD
    StartupStore(_T("... ComPort %u StopRxThread: Cancel Wait Event !%s"), (unsigned)(GetPortIndex() + 1), NEWLINE);
#endif
    CancelWaitEvent();

    if (ReadThread.isRunning()) {
#ifdef _DEBUG_STOP_RXTHREAD
        StartupStore(_T("... ComPort %u StopRxThread: Wait End of thread !%s"), (unsigned)(GetPortIndex() + 1), NEWLINE);
#endif
        // CritSec_Comm is locked by caller and can be waited by RxThread.
        // to prevent deadlock, CritSec_Comm must be unlocked before waiting end of thread.
        ScopeUnlock Unlock(CritSec_Comm);
        ReadThread.join();
    }
    StopEvt.reset();

    return true;
}

bool ComPort::StartRxThread() {
  try {
      StopEvt.reset();


      // Create a read thread for reading data from the communication port.
      ReadThread.start(*this);
      ReadThread.setPriority(Poco::Thread::PRIO_NORMAL); //THREAD_PRIORITY_ABOVE_NORMAL

      if (!ReadThread.isRunning()) {
          // Could not create the read thread.
          StartupStore(_T(". ComPort %u <%s> Failed to start Rx Thread%s"),
                       (unsigned) (GetPortIndex() + 1), GetPortName(), NEWLINE);

          // LKTOKEN  _@M761_ = "Unable to Start RX Thread on Port"
          StatusMessage(mbOk, MsgToken(266), TEXT("%s %s"), MsgToken(761), GetPortName());
          //DWORD dwError = GetLastError();
          return false;
      }
      return true;
  } catch(Poco::Exception& e) {
      const tstring error = to_tstring(e.displayText());
      StartupStore(_T(". ComPort %u <%s> StartRxThread : %s"), (unsigned)(GetPortIndex() + 1), GetPortName(), error.c_str());
      return false;
  } catch(std::exception& e) {
      const tstring error = to_tstring(e.what());
      StartupStore(_T(". ComPort %u <%s> StartRxThread : %s"), (unsigned)(GetPortIndex() + 1), GetPortName(), error.c_str());
      return false;
  }
}

void ComPort::run() {

    StartupStore(_T(". ComPort %u ReadThread : started%s"), (unsigned)(GetPortIndex() + 1), NEWLINE);

    RxThread();
    PDeviceDescriptor_t d = devGetDeviceOnPort(GetPortIndex());
    if(d) {
        d->Status = CPS_OPENKO;
    }

    StartupStore(_T(". ComPort %u ReadThread : terminated%s"), (unsigned)(GetPortIndex() + 1), NEWLINE);
}

void ComPort::ProcessChar(char c) {
    if (ComCheck_ActivePort>=0 && GetPortIndex()==(unsigned)ComCheck_ActivePort) {
        ComCheck_AddChar(c);
    }

    if(/*RadioPara.Enabled && */devParseStream(devIdx, &c, 1, &GPS_INFO)) {
        // if this port is used for stream device, leave immediately.
  //       don't return mayby more devices on one Port (shared Port)
    }

    // last char need to be reserved for '\0' for avoid buffer overflow
    // in theory this should never happen because NMEA sentence can't have more than 82 char and _NmeaString size is 160.
    if (pLastNmea >= std::begin(_NmeaString) && (pLastNmea+1) < std::end(_NmeaString)) {

        if (c == '\n' || c == '\r') {
            // abcd\n , now closing the line also with \r
            *(pLastNmea++) = _T('\n');
            *(pLastNmea) = _T('\0'); // terminate string.
            // process only meaningful sentences, avoid processing a single \n \r etc.
            if (std::distance(std::begin(_NmeaString), pLastNmea) > 5) {
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

void ComPort::AddStatRx(unsigned dwBytes) {
    if (GetPortIndex() < NUMDEV) {
        DeviceList[GetPortIndex()].Rx += dwBytes;
    }
}

void ComPort::AddStatErrRx(unsigned dwBytes) {
    if (GetPortIndex() < NUMDEV) {
        DeviceList[GetPortIndex()].ErrRx += dwBytes;
    }
}

void ComPort::AddStatTx(unsigned dwBytes) {
    if (GetPortIndex() < NUMDEV) {
        DeviceList[GetPortIndex()].Tx += dwBytes;
    }
}

void ComPort::AddStatErrTx(unsigned dwBytes) {
    if (GetPortIndex() < NUMDEV) {
        DeviceList[GetPortIndex()].ErrTx += dwBytes;
    }
}

void ComPort::SetPortStatus(int nStatus) {
    if (GetPortIndex() < NUMDEV) {
        DeviceList[GetPortIndex()].Status = nStatus;
    }
}

void ComPort::StatusMessage(MsgType_t type, const TCHAR *caption, const TCHAR *fmt, ...) {
    TCHAR tmp[127];
    va_list ap;
    LKASSERT(fmt!=NULL);

    va_start(ap, fmt);
    gcc_unused int n = _vsntprintf(tmp, 127, fmt, ap);
    va_end(ap);
    LKASSERT(n>=0); // Message to long for "tmp" buffer


    tmp[126] = _T('\0');

    if (caption) {
        MessageBoxX(tmp, LKGetText(caption), type);
    } else {
        DoStatusMessage(tmp);
    }
}
