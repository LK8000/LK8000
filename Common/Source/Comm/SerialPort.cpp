/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   SerialPort.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 2 août 2013, 19:11
 */

#include "externs.h"
#ifdef WIN32
#include "SerialPort.h"
#include <iterator>
#include <algorithm>
#include <functional>

using namespace std::placeholders;

SerialPort::SerialPort(int idx, const tstring& sName, DWORD dwSpeed, BitIndex_t BitSize, bool polling) :
        ComPort(idx, sName),
        hPort(INVALID_HANDLE_VALUE),
        _dwPortSpeed(dwSpeed),
        _dwPortBit(BitSize),
        valid_frames(0U),
        _PollingMode(polling),
        _dwMask(0)
{
}

SerialPort::~SerialPort() {
    if(hPort != INVALID_HANDLE_VALUE) {
        Close();
    }
}

bool SerialPort::Initialize() {
    TCHAR sysPortName[20];
    DCB PortDCB;

#ifndef UNDER_CE
    // Do not use COMn , use \\.\COMnn  on PC version
    _stprintf(sysPortName, _T("\\\\.\\%s"), GetPortName());
#else
    // Do not use COMn , use COMn:  on WinCE version
    _stprintf(sysPortName, _T("%s:"), GetPortName());
#endif
    StartupStore(_T(". ComPort %u Initialize <%s> speed=%lu bit=%u %s"),GetPortIndex()+1,GetPortName(),_dwPortSpeed,8-_dwPortBit,NEWLINE);

    hPort = CreateFile(sysPortName, // Pointer to the name of the port
            GENERIC_READ | GENERIC_WRITE, // Access (read-write) mode
            0, // Share mode
            NULL, // Pointer to the security attribute
            OPEN_EXISTING, // How to open the serial port
            FILE_ATTRIBUTE_NORMAL, // Port attributes
            NULL); // Handle to port with attribute
    // to copy

    if (hPort == INVALID_HANDLE_VALUE) {
        DWORD dwError = GetLastError();
        StartupStore(_T("... ComPort %u Init failed, error=%lu%s"), GetPortIndex() + 1, dwError, NEWLINE); // 091117
        StatusMessage(_T("%s %s"), MsgToken(762), GetPortName());

        goto failed;
    }
    StartupStore(_T(". ComPort %u  <%s> is now open%s"), GetPortIndex() + 1, GetPortName(), NEWLINE);


    PortDCB.DCBlength = sizeof (DCB);
    // Get the default port setting information.
    if (GetCommState(hPort, &PortDCB)==0) {
        StartupStore(_T("... ComPort %u GetCommState failed, error=%lu%s"),GetPortIndex()+1,GetLastError(),NEWLINE);
        // @M759 = Unable to Change Settings on Port
        StatusMessage(_T("%s %s"), MsgToken(759), GetPortName());
        goto failed;
    }
    // Change the DCB structure settings.
    PortDCB.BaudRate = _dwPortSpeed; // Current baud
    PortDCB.fBinary = TRUE; // Binary mode; no EOF check
    PortDCB.fParity = TRUE; // Enable parity checking
    PortDCB.fOutxCtsFlow = FALSE; // CTS output flow control: when TRUE, and CTS off, output suspended
    PortDCB.fOutxDsrFlow = FALSE; // DSR output flow control
    PortDCB.fDtrControl = DTR_CONTROL_ENABLE;

    PortDCB.fDsrSensitivity = FALSE; // DSR sensitivity
    PortDCB.fTXContinueOnXoff = TRUE; // XOFF continues Tx
    PortDCB.fOutX = FALSE; // No XON/XOFF out flow control
    PortDCB.fInX = FALSE; // No XON/XOFF in flow control
    PortDCB.fErrorChar = FALSE; // Disable error replacement
    PortDCB.fNull = FALSE; // Disable null removal
    PortDCB.fRtsControl = RTS_CONTROL_ENABLE; // RTS flow control

    PortDCB.fAbortOnError = FALSE;
    switch (_dwPortBit) {
        case bit7E1:
            PortDCB.ByteSize = 7;
            PortDCB.Parity = EVENPARITY;
            break;
        case bit8N1:
        default:
            PortDCB.ByteSize = 8;
            PortDCB.Parity = NOPARITY;
            break;
    }

    PortDCB.StopBits = ONESTOPBIT;
    PortDCB.EvtChar = '\n'; // wait for end of line, RXFLAG should detect it

    if (!SetCommState(hPort, &PortDCB)) {
        DWORD dwError = GetLastError();
        StartupStore(_T("... ComPort %u Init <%s> change setting FAILED, error=%lu%s"), GetPortIndex() + 1, GetPortName(), dwError, NEWLINE); // 091117
        // @M759 = Unable to Change Settings on Port
        StatusMessage(_T("%s %s"), MsgToken(759), GetPortName());

        goto failed;
    }

    if (SetRxTimeout(RXTIMEOUT) == -1) {
        DWORD dwError = GetLastError();
        StartupStore(_T("... ComPort %u Init <%s> change TimeOut FAILED, error=%lu%s"), GetPortIndex() + 1, GetPortName(), dwError, NEWLINE); // 091117
        // LKTOKEN  _@M760_ = "Unable to Set Serial Port Timers"
        StatusMessage(_T("%s %s"), MsgToken(760), GetPortName());

        goto failed;
    }

    SetupComm(hPort, 1024, 1024);

    // Direct the port to perform extended functions SETDTR and SETRTS
    // SETDTR: Sends the DTR (data-terminal-ready) signal.
    // SETRTS: Sends the RTS (request-to-send) signal.
    EscapeCommFunction(hPort, SETDTR);
    EscapeCommFunction(hPort, SETRTS);

    StartupStore(_T(". ComPort %u Init <%s> end OK%s"), GetPortIndex() + 1, GetPortName(), NEWLINE);
    return true;

failed:
    if(hPort != INVALID_HANDLE_VALUE) {
        if (!CloseHandle(hPort)) {
            DWORD dwError = GetLastError();
            StartupStore(_T("... ComPort %u Init <%s> close failed, error=%u%s"),GetPortIndex() + 1, GetPortName(), dwError, NEWLINE);
        } else {
            StartupStore(_T("... ComPort %u Init <%s> closed%s"),GetPortIndex() + 1, GetPortName(),NEWLINE);
        }
        hPort = INVALID_HANDLE_VALUE;

#ifdef UNDER_CE
        if (_PollingMode) {
            Poco::Thread::sleep(2000);
        }
#else
        Poco::Thread::sleep(2000); // needed for windows bug
#endif
    }
    return false;
}

int SerialPort::SetRxTimeout(int Timeout) {
    COMMTIMEOUTS CommTimeouts;
    int result;

    if (hPort == INVALID_HANDLE_VALUE)
        return -1;

    GetCommTimeouts(hPort, &CommTimeouts);

    result = CommTimeouts.ReadTotalTimeoutConstant;

    // Change the COMMTIMEOUTS structure settings.
    CommTimeouts.ReadIntervalTimeout = MAXDWORD;

    // JMW 20070515
    if (Timeout == 0) {
        // no total timeouts used
        CommTimeouts.ReadTotalTimeoutMultiplier = 0;
        CommTimeouts.ReadTotalTimeoutConstant = 0;
    } else {
        // only total timeout used
        CommTimeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
        CommTimeouts.ReadTotalTimeoutConstant = Timeout;
    }

    CommTimeouts.WriteTotalTimeoutMultiplier = 10;
    CommTimeouts.WriteTotalTimeoutConstant = 1000;

    // Set the time-out parameters
    // for all read and write
    // operations on the port.
    if (!SetCommTimeouts(hPort, &CommTimeouts)) {
        return -1;
    }

    return result;
}

unsigned long SerialPort::SetBaudrate(unsigned long BaudRate) {
    COMSTAT ComStat;
    DCB PortDCB;
    DWORD dwErrors;
    unsigned long result = 0;

    if (hPort == INVALID_HANDLE_VALUE) {
        return result;
    }

    do {
        ClearCommError(hPort, &dwErrors, &ComStat);
    } while (ComStat.cbOutQue > 0);

    Poco::Thread::sleep(10);

    GetCommState(hPort, &PortDCB);

    result = PortDCB.BaudRate;
    PortDCB.BaudRate = BaudRate;

    if (!SetCommState(hPort, &PortDCB)) {
        return 0;
    }

    return result;
}

unsigned long SerialPort::GetBaudrate() const {
    COMSTAT ComStat;
    DCB PortDCB;
    DWORD dwErrors;

    if (hPort == INVALID_HANDLE_VALUE) {
        return 0;
    }

    do {
        ClearCommError(hPort, &dwErrors, &ComStat);
    } while (ComStat.cbOutQue > 0);

    Poco::Thread::sleep(10);

    GetCommState(hPort, &PortDCB);

    return PortDCB.BaudRate;
}

size_t SerialPort::Read(void *data, size_t size) {
    if (hPort != INVALID_HANDLE_VALUE) {
        DWORD dwBytesTransferred = 0U;
        if (ReadFile(hPort, data, size, &dwBytesTransferred, (OVERLAPPED *) NULL)) {
            AddStatRx(dwBytesTransferred);
            return dwBytesTransferred;
        }
    }
    return 0U;
}

bool SerialPort::Close() {
    bool Ret = ComPort::Close(); // NOTICE: Ret is unused here, because Close is always returning true
    if (hPort != INVALID_HANDLE_VALUE) {
        // Close the communication port.
        if (!CloseHandle(hPort)) {
            DWORD dwError = GetLastError();
            StartupStore(_T("... ComPort %u close failed, error=%lu%s"), GetPortIndex() + 1, dwError, NEWLINE);
            Ret = false;
        } else {

#ifdef UNDER_CE
            if (_PollingMode) {
                Poco::Thread::sleep(2000);
            }
#else
            Poco::Thread::sleep(2000); // needed for windows bug
#endif
            hPort = INVALID_HANDLE_VALUE;
            StartupStore(_T(". ComPort %u closed Ok.%s"), GetPortIndex() + 1, NEWLINE); // 100210 BUGFIX missing
            Ret = true;
        }
    }
    return Ret;
}

void SerialPort::Flush() {
    if(hPort != INVALID_HANDLE_VALUE) {
        FlushFileBuffers(hPort);
    }
}

// This is called on open, to discard anything existing in the buffer
void SerialPort::Purge() {
    if(hPort != INVALID_HANDLE_VALUE) {
        PurgeComm(hPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
    }
}

// This is not accessing I/O buffers at all. We do this on close.
void SerialPort::DirtyPurge() {

}

void SerialPort::CancelWaitEvent() {
#ifdef UNDER_CE
    if(hPort != INVALID_HANDLE_VALUE) {
        DirtyPurge();
        if (!_PollingMode) {
            // setting the comm event mask with the same value
            SetCommMask(hPort, _dwMask); // will cancel any
                                        // WaitCommEvent!  this is a
                                        // documented CE trick to
                                        // cancel the WaitCommEvent
        }
    }
#endif
}

bool SerialPort::IsReady() {
    return (hPort != INVALID_HANDLE_VALUE);
}

void SerialPort::UpdateStatus() {
    DWORD dwErrors = 0;
    COMSTAT comStat = {};
    if(hPort != INVALID_HANDLE_VALUE) {
        ClearCommError(hPort, &dwErrors, &comStat);
        if (dwErrors & CE_FRAME) {
            SetPortStatus(CPS_EFRAME);
            AddStatErrRx(1);
            valid_frames = 0;
        } else {
            if (++valid_frames > 10) {
                valid_frames = 20;
                SetPortStatus(CPS_OPENOK);
            }
        }
    }
}

bool SerialPort::Write_Impl(const void *data, size_t size) {
    DWORD written;

    if (hPort == INVALID_HANDLE_VALUE) {
        return (false);
    }

    if (!WriteFile(hPort, data, size, &written, NULL) || written != size) {
        // WriteFile failed, report error
        AddStatErrTx(1);
        return (false);
    }

    AddStatTx(written);

    return (true);
}

unsigned SerialPort::RxThread() {
    DWORD dwBytesTransferred = 0; // 091117 initialized variables
    char szString[1024];

    Purge();


#ifdef UNDER_CE
    DWORD dwCommModemStatus = 0;

    // Specify a set of events to be monitored for the port.
    _dwMask = EV_RXFLAG | EV_CTS | EV_DSR | EV_RING | EV_RXCHAR;
    if (!_PollingMode) {
        SetCommMask(hPort, _dwMask);
    }
#endif

    while ((hPort != INVALID_HANDLE_VALUE) && !StopEvt.tryWait(0)) {

        UpdateStatus();

#ifdef UNDER_CE
        if (_PollingMode) {
            Poco::Thread::sleep(5);
        } else {
            // Wait for an event to occur for the port.
            if (!WaitCommEvent(hPort, &dwCommModemStatus, 0)) {
                // error reading from port
                Poco::Thread::sleep(5);
            }
        }
#else
        // PC version does BUSY WAIT
        Poco::Thread::sleep(5); // ToDo rewrite the whole driver to use overlaped IO on W2K or higher
#endif

#ifdef UNDER_CE
        if (_PollingMode || (dwCommModemStatus & EV_RXFLAG) || (dwCommModemStatus & EV_RXCHAR)) // Do this only for non-PC
#endif
        {
            // Loop to wait for the data.
            do {
                WithLock(CritSec_Comm, [&](){
                    // Read the data from the serial port.
                    dwBytesTransferred = ComPort::Read(szString);
                    if (dwBytesTransferred > 0) {
                        std::for_each(std::begin(szString), std::next(szString, dwBytesTransferred), GetProcessCharHandler());
                    } else {
                        dwBytesTransferred = 0;
                    }
                });
                Poco::Thread::sleep(1); // JMW20070515: give port some time to
                // fill... prevents ReadFile from causing the
                // thread to take up too much CPU

                if (StopEvt.tryWait(0)) {
                    dwBytesTransferred = 0;
                }
            } while (dwBytesTransferred != 0);
        }

        // Retrieve modem control-register values.
#ifdef UNDER_CE
        if (!_PollingMode) {
            // this is causing problems on PC BT, apparently. Setting Polling will not call this, but it is a bug
            GetCommModemStatus(hPort, &dwCommModemStatus);
        }
#endif
    }

    DirtyPurge();

    return 0;
}
#endif
