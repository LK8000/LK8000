/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   SerialPort.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 2 août 2013, 19:11
 */

#include "externs.h"
#include "SerialPort.h"
#include "utils/stl_utils.h"
#include <algorithm>
#include <functional>

SerialPort::SerialPort(int idx, const std::wstring& sName, DWORD dwSpeed, BitIndex_t BitSize, bool polling) : 
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

}

bool SerialPort::Initialize() {
    TCHAR sysPortName[20];
    DCB PortDCB;

#if (WINDOWSPC>0)
    // Do not use COMn , use \\.\COMnn  on PC version
    _stprintf(sysPortName, _T("\\\\.\\%s"), GetPortName());
#else
    // Do not use COMn , use COMn:  on WinCE version
    _stprintf(sysPortName, _T("%s:"), GetPortName());
#endif
    StartupStore(_T(". ComPort %u Initialize <%s> speed=%u bit=%u %s"),GetPortIndex()+1,GetPortName(),_dwPortSpeed,8-_dwPortBit,NEWLINE);

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
        StartupStore(_T("... ComPort %u Init failed, error=%u%s"), GetPortIndex() + 1, dwError, NEWLINE); // 091117
        StatusMessage(MB_OK, NULL, TEXT("%s %s"), gettext(TEXT("_@M762_")), GetPortName());

        return false;
    }
    StartupStore(_T(". ComPort %u  <%s> is now open%s"), GetPortIndex() + 1, GetPortName(), NEWLINE);


    PortDCB.DCBlength = sizeof (DCB);
    // Get the default port setting information.
    if (GetCommState(hPort, &PortDCB)==0) {
        CloseHandle(hPort);
        hPort = INVALID_HANDLE_VALUE;

    	StartupStore(_T("... ComPort %u GetCommState failed, error=%u%s"),GetPortIndex()+1,GetLastError(),NEWLINE);
        // cannot set serial port timers. good anyway
        StatusMessage(MB_OK|MB_ICONINFORMATION, NULL, TEXT("%s %s"), gettext(TEXT("_@M760_")), GetPortName());
        return false;
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

    PortDCB.fAbortOnError = TRUE; // FALSE need something else to work
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
        // Could not create the read thread.
        CloseHandle(hPort);
        hPort = INVALID_HANDLE_VALUE;
#if (WINDOWSPC>0) || NEWCOMM // 091206
        Sleep(2000); // needed for windows bug 101116 not verified
#endif
#if !(WINDOWSPC>0)
        if (PollingMode) Sleep(2000);
#endif
        DWORD dwError = GetLastError();
        StartupStore(_T("... ComPort %u Init <%s> change setting FAILED, error=%u%s"), GetPortIndex() + 1, GetPortName(), dwError, NEWLINE); // 091117
    	StatusMessage(MB_OK, TEXT("Error"), TEXT("%s %s"), gettext(TEXT("_@M759_")), GetPortName());

        return false;
    }

    if (SetRxTimeout(RXTIMEOUT) == -1) {
        DWORD dwError = GetLastError();
        StartupStore(_T("... ComPort %u Init <%s> change TimeOut FAILED, error=%u%s"), GetPortIndex() + 1, GetPortName(), dwError, NEWLINE); // 091117
        StatusMessage(MB_OK, TEXT("Error"), TEXT("%s %s"), gettext(TEXT("_@M759_")), GetPortName());

        return false;
    }

    SetupComm(hPort, 1024, 1024);

    // Direct the port to perform extended functions SETDTR and SETRTS
    // SETDTR: Sends the DTR (data-terminal-ready) signal.
    // SETRTS: Sends the RTS (request-to-send) signal. 
    EscapeCommFunction(hPort, SETDTR);
    EscapeCommFunction(hPort, SETRTS);

    StartupStore(_T(". ComPort %u Init <%s> end OK%s"), GetPortIndex() + 1, GetPortName(), NEWLINE);

    return ComPort::Initialize();
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
        CommTimeouts.ReadTotalTimeoutMultiplier = 0;
        CommTimeouts.ReadTotalTimeoutConstant = Timeout;
    }

    CommTimeouts.WriteTotalTimeoutMultiplier = 10;
    CommTimeouts.WriteTotalTimeoutConstant = 1000;

    // Set the time-out parameters
    // for all read and write
    // operations on the port.
    if (!SetCommTimeouts(hPort, &CommTimeouts)) {
        // Could not create the read thread.
        CloseHandle(hPort);
        hPort = INVALID_HANDLE_VALUE;
        Sleep(2000); // needed for windows bug

        // LKTOKEN  _@M760_ = "Unable to Set Serial Port Timers" 
        StatusMessage(MB_OK, TEXT("Error"), TEXT("%s %s"), gettext(TEXT("_@M760_")), GetPortName());
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

    Sleep(10);

    GetCommState(hPort, &PortDCB);

    result = PortDCB.BaudRate;
    PortDCB.BaudRate = BaudRate;

    if (!SetCommState(hPort, &PortDCB)) {
        return 0;
    }

    return result;
}

unsigned long SerialPort::GetBaudrate() {
    COMSTAT ComStat;
    DCB PortDCB;
    DWORD dwErrors;

    if (hPort == INVALID_HANDLE_VALUE) {
        return 0;
    }

    do {
        ClearCommError(hPort, &dwErrors, &ComStat);
    } while (ComStat.cbOutQue > 0);

    Sleep(10);

    GetCommState(hPort, &PortDCB);

    return PortDCB.BaudRate;
}

size_t SerialPort::Read(void *szString, size_t size) {
    if (hPort != INVALID_HANDLE_VALUE) {
        DWORD dwBytesTransferred = 0U;
        if (ReadFile(hPort, szString, size, &dwBytesTransferred, (OVERLAPPED *) NULL)) {
            AddStatRx(dwBytesTransferred);
            return dwBytesTransferred;
        }
    }
    return 0U;
}

bool SerialPort::Close() {
    bool Ret = ComPort::Close();
    if (hPort != INVALID_HANDLE_VALUE) {
        // Close the communication port.
        if (!CloseHandle(hPort)) {
            DWORD dwError = GetLastError();
            StartupStore(_T("... ComPort %u close failed, error=%u%s"), GetPortIndex() + 1, dwError, NEWLINE);
            Ret = true;
        } else {
            Sleep(2000); // needed for windows bug
            hPort = INVALID_HANDLE_VALUE;
            StartupStore(_T(". ComPort %u closed Ok.%s"), GetPortIndex() + 1, NEWLINE); // 100210 BUGFIX missing
            Ret = false;
        }
    }
    return Ret;
}

void SerialPort::Flush() {
    FlushFileBuffers(hPort);
}

void SerialPort::Purge() {
    PurgeComm(hPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
}

void SerialPort::CancelWaitEvent() {
#if (WINDOWSPC>0) || NEWCOMM  // 091206

#else    
    Flush();

    // setting the comm event mask with the same value
    SetCommMask(hPort, _dwMask); // will cancel any
                                 // WaitCommEvent!  this is a
                                 // documented CE trick to
                                 // cancel the WaitCommEvent
#endif
}

void SerialPort::UpdateStatus() {
    DWORD dwErrors = 0;
    COMSTAT comStat = {0};

    ClearCommError(hPort, &dwErrors, &comStat);
    if (dwErrors & CE_FRAME) {
        //StartupStore(_T("... Com port %d, dwErrors=%ld FRAME (old status=%d)\n"),
        //	GetPortIndex(),dwErrors,ComPortStatus[GetPortIndex()]);
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

bool SerialPort::Write(const void *data, size_t length) {
    DWORD written;

    if (hPort == INVALID_HANDLE_VALUE) {
        return (false);
    }

    if (!WriteFile(hPort, data, length, &written, NULL) || written != length) {
        // WriteFile failed, report error
        AddStatErrTx(1);
        AddStatErrors(1);
        return (false);
    }

    AddStatTx(written);

    return (true);
}

extern void Cpustats(int *acc, FILETIME *a, FILETIME *b, FILETIME *c, FILETIME *d);

DWORD SerialPort::RxThread() {
#if (!defined(WINDOWSPC) || (WINDOWSPC == 0)) && !NEWCOMM	// 100222
    DWORD dwCommModemStatus = 0;
    // Specify a set of events to be monitored for the port.
#endif
    DWORD dwBytesTransferred = 0; // 091117 initialized variables
    _Buff_t szString;
    FILETIME CreationTime, ExitTime, StartKernelTime, EndKernelTime, StartUserTime, EndUserTime;

    Purge();
#if TRACETHREAD
    StartupStore(_T("##############  PORT=%d threadid=%d\n"), GetPortIndex(), GetCurrentThreadId());
    if (GetPortIndex() = 0) _THREADID_PORT1 = GetCurrentThreadId();
    if (GetPortIndex() = 1) _THREADID_PORT2 = GetCurrentThreadId();
    if (GetPortIndex() != 1 && GetPortIndex() != 2) _THREADID_UNKNOWNPORT = GetCurrentThreadId();
#endif
    // Specify a set of events to be monitored for the port.
    _dwMask = EV_RXFLAG | EV_CTS | EV_DSR | EV_RING | EV_RXCHAR;

#if (!defined(WINDOWSPC) || (WINDOWSPC == 0)) && !NEWCOMM
    SetCommMask(hPort, _dwMask);
#endif
#if (WINDOWSPC<1)
    if (!_PollingMode) SetCommMask(hPort, _dwMask);
#endif

    while ((hPort != INVALID_HANDLE_VALUE) && (::WaitForSingleObject(hStop, 0) == WAIT_TIMEOUT)) {
        GetThreadTimes(hReadThread, &CreationTime, &ExitTime, &StartKernelTime, &StartUserTime);

        UpdateStatus();

#if (WINDOWSPC>0) || NEWCOMM // 091206
        // PC version does BUSY WAIT
        Sleep(50); // ToDo rewrite the whole driver to use overlaped IO on W2K or higher
#else
        if (_PollingMode)
            Sleep(100);
        else
            // Wait for an event to occur for the port.
            if (!WaitCommEvent(hPort, &dwCommModemStatus, 0)) {
            // error reading from port
            Sleep(100);
        }
#endif

        // Re-specify the set of events to be monitored for the port.
        //    SetCommMask(hPort, dwMask1);

        // #if !defined(WINDOWSPC) || (WINDOWSPC == 0) 091206
#if (!defined(WINDOWSPC) || (WINDOWSPC == 0)) && !NEWCOMM
        if (_PollingMode || (dwCommModemStatus & EV_RXFLAG) || (dwCommModemStatus & EV_RXCHAR)) // Do this only for non-PC
#endif
        {
            // Loop for waiting for the data.
            do {
                // Read the data from the serial port.
                dwBytesTransferred = ReadData(szString);
                if (dwBytesTransferred > 0) {
                    if (ProgramStarted >= psNormalOp) { // ignore everything until started
                        std::for_each(begin(szString), begin(szString) + dwBytesTransferred, std::bind1st(std::mem_fun(&SerialPort::ProcessChar), this));
                    }
                } else {
                    dwBytesTransferred = 0;
                }

                Sleep(50); // JMW20070515: give port some time to
                // fill... prevents ReadFile from causing the
                // thread to take up too much CPU
                if ((GetThreadTimes(hReadThread, &CreationTime, &ExitTime, &EndKernelTime, &EndUserTime)) == 0) {
                    if (GetPortIndex() == 0)
                        Cpu_PortA = 9999;
                    else
                        Cpu_PortB = 9999;
                } else {
                    Cpustats((GetPortIndex() == 0) ? &Cpu_PortA : &Cpu_PortB, &StartKernelTime, &EndKernelTime, &StartUserTime, &EndUserTime);
                }

                if (::WaitForSingleObject(hStop, 0) != WAIT_TIMEOUT) {
                    dwBytesTransferred = 0;
                }
            } while (dwBytesTransferred != 0);
        }

        // give port some time to fill
        Sleep(5);

        // Retrieve modem control-register values.
#if (!defined(WINDOWSPC) || (WINDOWSPC == 0)) 
        if (!_PollingMode) {
            // this is causing problems on PC BT, apparently. Setting Polling will not call this, but it is a bug
            GetCommModemStatus(hPort, &dwCommModemStatus);
        }
#endif
    }

    Purge();

    return 0;
}
