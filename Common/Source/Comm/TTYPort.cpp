/*
 * File:   TTYPort.cpp
 * Author: blc
 *
 * Created on 11 août 2014, 10:42
 */
#ifdef __linux__
#include "externs.h"
#include "TTYPort.h"
#include "utils/stl_utils.h"
#include <functional>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "utils/lookup_table.h"

#ifdef KOBO
#include "Kobo/System.hpp"
#endif


using namespace std::placeholders;

TTYPort::TTYPort(unsigned idx, const tstring& sName, unsigned dwSpeed, BitIndex_t BitSize, bool polling) :
        ComPort(idx, sName),
        _dwPortSpeed(dwSpeed),
        _dwPortBit(BitSize),
        _tty(-1),
        _oldtio(),
        _Timeout()
{
}

TTYPort::~TTYPort() {
    Close();
}

namespace {

constexpr auto DecodeBaudrateTable = lookup_table<int, speed_t>({
    {1200, B1200},
    {2400, B2400},
    {4800, B4800},
    {9600, B9600},
    {19200, B19200},
    {38400, B38400},
    {57600, B57600},
    {115200, B115200},
    {230400, B230400},
    {460800, B460800},
    {500000, B500000},
    {1000000, B1000000}
});

speed_t DecodeBaudrate(int speed) {
    return DecodeBaudrateTable.get(speed, B9600);
}

} // namespace

bool TTYPort::Initialize() {
    StartupStore(_T(". ComPort %u Initialize <%s> speed=%u bit=%u %s"), GetPortIndex() + 1,GetPortName(),_dwPortSpeed,8-_dwPortBit,NEWLINE);

#ifdef KOBO
    if (_tcscmp(GetPortName(), _T("/dev/ttyGS0")) == 0) {
        KoboExportSerial();
    }
#endif

    tstring szPath = GetPortName();
    if(szPath.compare(0, 3, _T("id:")) == 0) {
        szPath.replace(szPath.begin(), std::next(szPath.begin(), 3), _T("/dev/serial/by-id/"));
    }

    _tty = open(szPath.c_str(), O_RDWR | O_NOCTTY);
    if (_tty < 0 || !isatty(_tty)) {
        StartupStore(_T("... ComPort %u Init failed, error=%u%s"), GetPortIndex() + 1, errno, NEWLINE); // 091117
        StatusMessage(_T("%s %s"), MsgToken(762), GetPortName());
        goto failed;
    }

    struct termios newtio;

    tcgetattr(_tty, &_oldtio); // save current port settings

    bzero(&newtio, sizeof (newtio));

    newtio.c_cflag = ((_dwPortBit == bit8N1) ? CS8 : CS7|PARENB) | CLOCAL | CREAD;
    newtio.c_iflag = IGNBRK | IGNPAR; // no break & no parity

    cfsetospeed(&newtio, DecodeBaudrate(_dwPortSpeed));
    cfsetispeed(&newtio, DecodeBaudrate(_dwPortSpeed));

    newtio.c_cc[VTIME] = _Timeout / 100; // inter-character timer ( 1/10sec unit)
    newtio.c_cc[VMIN] = 1; // blocking read until 1 chars received

    tcflush(_tty, TCIFLUSH);
    tcsetattr(_tty, TCSANOW, &newtio);

    SetPortStatus(CPS_OPENOK);
    StartupStore(_T(". ComPort %u Init <%s> end OK"), GetPortIndex() + 1, GetPortName());

    return true;

failed:
    if (_tty >= 0) {
        tcsetattr(_tty, TCSANOW, &_oldtio);
        close(_tty);
        _tty = -1;
    }
    return false;
}

int TTYPort::SetRxTimeout(int Timeout) {
    unsigned int OldTimeOut = _Timeout;
    if (_Timeout != Timeout) {
        _Timeout = Timeout;

        struct termios newtio;
        bzero(&newtio, sizeof (newtio));
        tcgetattr(_tty, &newtio); // save current port settings
        newtio.c_cc[VTIME] = _Timeout / 100; // inter-character timer ( 1/10sec unit)
        tcflush(_tty, TCIFLUSH);
        tcsetattr(_tty, TCSANOW, &newtio);
    }
    return OldTimeOut;
}

unsigned long TTYPort::SetBaudrate(unsigned long BaudRate) {

    unsigned long oldSpeed = _dwPortSpeed;
    if (_dwPortSpeed != BaudRate) {
        _dwPortSpeed = BaudRate;

        struct termios newtio;
        bzero(&newtio, sizeof (newtio));
        tcgetattr(_tty, &newtio); // save current port settings
        cfsetospeed(&newtio, DecodeBaudrate(_dwPortSpeed));
        cfsetispeed(&newtio, DecodeBaudrate(_dwPortSpeed));
        tcflush(_tty, TCIFLUSH);
        tcsetattr(_tty, TCSANOW, &newtio);
    }
    return oldSpeed;
}

unsigned long TTYPort::GetBaudrate() const {
    return _dwPortSpeed;
}

void TTYPort::Flush() {
    if(_tty != -1) {
        tcflush(_tty, TCIOFLUSH);
    }
}

void TTYPort::Purge() {
    if(_tty != -1) {
#if defined(ANDROID) && __ANDROID_API__ < 21
        ioctl(_tty, TCSBRK, 1);
#else
        tcdrain(_tty);
#endif
    }
}

void TTYPort::CancelWaitEvent() {

}

void TTYPort::UpdateStatus() {

}

bool TTYPort::IsReady() {
    return (_tty >= 0 && isatty(_tty));
}

size_t TTYPort::Read(void *szString, size_t size) {
    if(_tty < 0) {
        return 0;
    }

    struct timespec timeout;
    timeout.tv_sec = _Timeout / 1000;
    timeout.tv_nsec = (_Timeout % 1000)*1000;

    fd_set readfs;
    FD_ZERO(&readfs);
    FD_SET(_tty, &readfs);

    sigset_t empty_mask;
    sigemptyset(&empty_mask);
    // wait for received data
    int iResult = pselect(_tty + 1, &readfs, NULL, NULL, &timeout, &empty_mask);
    if (iResult == -1 && errno == EINTR) {
        return 0U;
    }
    if (iResult == 0) {
        return 0U; // timeout
    }

    if ((iResult != -1) && FD_ISSET(_tty, &readfs)) {
        // Data ready to read
        iResult = read(_tty, (char*) szString, size);
        if (iResult > 0) {
            AddStatRx(iResult);
            return iResult;
        }
    }

    if (iResult <= 0) {
        AddStatErrRx(1);
        close(_tty);
        _tty = -1;
    }
    return 0U;
}

bool TTYPort::Close() {
    ComPort::Close();
    if (_tty >= 0) {
        tcsetattr(_tty, TCSANOW, &_oldtio);
        close(_tty);
        _tty = -1;
    }
#ifdef KOBO
    if (_tcscmp(GetPortName(), _T("/dev/ttyGS0")) == 0) {
        KoboUnexportSerial();
    }
#endif

    return true;
}

bool TTYPort::Write_Impl(const void *data, size_t size) {
    if(_tty < 0) {
        return 0;
    }

    struct timeval timeout;
    fd_set writefs;
    timeout.tv_sec = _Timeout / 1000;
    timeout.tv_usec = _Timeout % 1000;

    int iResult = 0;
    FD_ZERO(&writefs);
    FD_SET(_tty, &writefs);

    // wait for socket ready to write
    iResult = select(_tty + 1, NULL, &writefs, NULL, &timeout);
    if (iResult == 0) {
        return false; // timeout
    }

    if ((iResult != -1) && FD_ISSET(_tty, &writefs)) {
        // socket ready, Write data.
        iResult = write(_tty, (const char*) data, size);
        if (iResult > 0) {
            AddStatTx(iResult);
            return true;
        }
    }

    if (iResult == -1) {
        AddStatErrTx(1);
        close(_tty);
        _tty = -1;

        return false;
    }

    return true;
}

unsigned TTYPort::RxThread() {
    char szString[1024];
    Purge();

    while ((_tty != -1) && !StopEvt.tryWait(5)) {
        ScopeLock Lock(CritSec_Comm);
        UpdateStatus();
        int nRecv = ComPort::Read(szString);
        if (nRecv > 0) {
            std::for_each(std::begin(szString), std::next(szString, nRecv), GetProcessCharHandler());
        }
    }

    return 0UL;
}
#endif
