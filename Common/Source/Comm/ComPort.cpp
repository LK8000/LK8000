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
#include "ComCheck.h"
#include <sstream>
#include <regex>

ComPort::ComPort(unsigned idx, const tstring& sName) 
        : Thread("ComPort"), StopEvt(false)
        , devIdx(idx), sPortName(sName)
        , status_thread("ComPort::status_thread", *this)
{
    pLastNmea = std::begin(_NmeaString);
}

bool ComPort::Close() {
    if (status_thread.IsDefined()) {
        WithLock(status_mutex, [&]() {
            status_thread_stop = true;
            status_cv.Signal();
        });
        status_thread.Join();
    }
    StopRxThread();
    return true;
}

namespace {

    tstring thread_name() {
        Poco::Thread* pThread = Poco::Thread::current();
        if (pThread) {
            return to_tstring(pThread->getName());
        }
        std::ostringstream name;
    	name << '#' << Poco::Thread::currentTid();
	    return to_tstring(name.str());
    }

    tstring data_string(const void *data, size_t size) {
        std::string s(static_cast<const char*>(data), size);
        s = std::regex_replace(s, std::regex(R"(\n)"), R"(\n)");
        s = std::regex_replace(s, std::regex(R"(\r)"), R"(\r)");
        return to_tstring(s);
    }

}

// this is used by all functions to send data out
bool ComPort::Write(const void *data, size_t size) {
    if (size > 0) {
        bool success = Write_Impl(data, size);

        DebugLog(_T(R"(<%s><%s> ComPort::Write("%s"))"),
                    success ? _T("success"): _T("failed"),
                    thread_name().c_str(),
                    data_string(data, size).c_str());

        return success;
    }
    return false;
}

#ifdef  _UNICODE
// this should be deprecated
bool ComPort::WriteString(const wchar_t* Text) {
    return WriteString(to_utf8(Text));
}
#endif

int ComPort::GetChar() {
    char c = EOF;
    if (Read(&c, 1) == 1) {
        return c;
    }
    return EOF;
}

bool ComPort::StopRxThread() {

    StopEvt.set();

    DebugLog(_T("... ComPort %u StopRxThread: Cancel Wait Event !"), GetPortIndex() + 1);
    CancelWaitEvent();

    if (IsDefined()) {
        DebugLog(_T("... ComPort %u StopRxThread: Wait End of thread !"), GetPortIndex() + 1);
        Join();
    }
    StopEvt.reset();

    return true;
}

bool ComPort::StartRxThread() {
  try {
      StopEvt.reset();

      // Create a read thread for reading data from the communication port.
      Start();

      if (!IsDefined()) {
          // Could not create the read thread.
          StartupStore(_T(". ComPort %u <%s> Failed to start Rx Thread"),
                       GetPortIndex() + 1, GetPortName());

          // LKTOKEN  _@M761_ = "Unable to Start RX Thread on Port"
          StatusMessage(_T("%s %s"), MsgToken<761>(), GetPortName());
          //DWORD dwError = GetLastError();
          return false;
      }
      return true;
  } catch(Poco::Exception& e) {
      const tstring error = to_tstring(e.displayText());
      StartupStore(_T(". ComPort %u <%s> StartRxThread : %s"), GetPortIndex() + 1, GetPortName(), error.c_str());
      return false;
  } catch(std::exception& e) {
      const tstring error = to_tstring(e.what());
      StartupStore(_T(". ComPort %u <%s> StartRxThread : %s"), GetPortIndex() + 1, GetPortName(), error.c_str());
      return false;
  }
}

void ComPort::Run() {
    StartupStore(_T(". ComPort %u ReadThread : started"), GetPortIndex() + 1);
    RxThread();
    StartupStore(_T(". ComPort %u ReadThread : terminated"), GetPortIndex() + 1);
}

void ComPort::ProcessChar(char c) {
    if (ComCheck_ActivePort>=0 && GetPortIndex()==(unsigned)ComCheck_ActivePort) {
        ComCheck_AddChar(c);
    }

    if (devParseStream(devIdx, &c, 1, &GPS_INFO)) {
        // if this port is used for stream device, leave immediately.
        // don't return mayby more devices on one Port (shared Port)
    }

    // last char need to be reserved for '\0' for avoid buffer overflow
    // in theory this should never happen because NMEA sentence can't have more than 82 char and _NmeaString size is 160.
    if (pLastNmea >= std::begin(_NmeaString) && (pLastNmea+1) < std::end(_NmeaString)) {

        if (c == '\n' || c == '\r') {
            // abcd\n , now closing the line also with \r
            *(pLastNmea++) = '\n';
            *(pLastNmea) = '\0'; // terminate string.
            // process only meaningful sentences, avoid processing a single \n \r etc.
            if (std::distance(std::begin(_NmeaString), pLastNmea) > 5) {
                devParseNMEA(devIdx, _NmeaString, &GPS_INFO);
            }
        } else {
            *(pLastNmea++) = c;
            return;
        }
    }
    // overflow, so reset buffer
    pLastNmea = std::begin(_NmeaString);
}

void ComPort::ProcessData(const char* string, size_t size) {
    for (auto c : std::string_view(string, size)) {
        ProcessChar(c);
    }
}

void ComPort::AddStatRx(unsigned dwBytes) const {
    if (GetPortIndex() < NUMDEV) {
        DeviceList[GetPortIndex()].Rx += dwBytes;
    }
}

void ComPort::AddStatErrRx(unsigned dwBytes) const {
    if (GetPortIndex() < NUMDEV) {
        DeviceList[GetPortIndex()].ErrRx += dwBytes;
    }
}

void ComPort::AddStatTx(unsigned dwBytes) const {
    if (GetPortIndex() < NUMDEV) {
        DeviceList[GetPortIndex()].Tx += dwBytes;
    }
}

void ComPort::AddStatErrTx(unsigned dwBytes) const {
    if (GetPortIndex() < NUMDEV) {
        DeviceList[GetPortIndex()].ErrTx += dwBytes;
    }
}

void ComPort::StatusMessage(const TCHAR *fmt, ...) {
    TCHAR tmp[127];
    va_list ap;
    LKASSERT(fmt!=NULL);

    va_start(ap, fmt);
    gcc_unused int n = _vsntprintf(tmp, 127, fmt, ap);
    va_end(ap);
    LKASSERT(n>=0); // Message to long for "tmp" buffer

    tmp[126] = _T('\0');
    DoStatusMessage(tmp);
}

tstring ComPort::GetDeviceName() {
    return GetPortName();
}

void ComPort::NotifyConnected() {
    ScopeLock lock(status_mutex);
    status_connected = true;
    if (status_disconnected_notify) {
        status_cv.Signal();
        return;
    }

    // notify user
    tstring name = GetDeviceName();
    StatusMessage(_T("%s connected"), name.c_str());
    TestLog(_T("ble_notify: %s connected"), name.c_str());
}

void ComPort::status_thread_loop() {
    try {
        ScopeLock lock(status_mutex);
        while (!status_thread_stop) { // until stop not request
            if (status_disconnected_notify) { // if disconnect notify requested
                tstring name = GetDeviceName();
                status_cv.Wait(status_mutex, 10000); // wait 10s for reconnecting
                if(status_thread_stop) {
                  return; // must exit otherwise thread never end ...
                }
                if (!status_connected) { // if not reconnected notify user
                    StatusMessage(_T("%s disconnected"), name.c_str());
                    TestLog(_T("ble_notify: %s disconnected"), name.c_str());
                }
                else { // if reconnected don't notify user
                    TestLog(_T("ble_notify: %s reconnected"), name.c_str());
                }
                status_disconnected_notify = false; // reset notify request
            }
            status_cv.Wait(status_mutex);  // wait for notification request or stop
        }
    }
    catch (std::exception& e) {
        StartupStore(_T("ComPort::status_thread_loop : %s"), to_tstring(e.what()).c_str());
    }
}

void ComPort::NotifyDisconnected() {
    tstring name = GetDeviceName();
    DebugLog(_T("ble_notify: %s request disconnected"), name.c_str());
    // notify user in next 10 sec, notification canceled if reconnect happen...
    ScopeLock lock(status_mutex);
    status_connected = false;
    if (!status_thread.IsDefined()) {
        status_thread.Start();
    }
    status_disconnected_notify = true;
    status_cv.Signal();
}
