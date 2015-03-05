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
#include <tr1/functional>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


using namespace std::tr1::placeholders;

TTYPort::TTYPort(int idx, const std::tstring& sName, DWORD dwSpeed, BitIndex_t BitSize, bool polling) :
        ComPort(idx, sName),
        _dwPortSpeed(dwSpeed),
        _dwPortBit(BitSize),
        valid_frames(0U),
        _tty(-1),
        _oldtio(),
        _Timeout() 
{
}

TTYPort::~TTYPort() {
    Close();
}

tcflag_t DecodeBaudrate(int speed) {

    struct SpeedToFlag {
        int nSpeed;
        tcflag_t FlagSpeed;
    };
    static const SpeedToFlag SpeedToFlagTable[] = {
        {1200, B1200},
        {2400, B2400},
        {4800, B4800},
        {9600, B9600},
        {19200, B19200},
        {38400, B38400},
        {57600, B57600},
        {115200, B115200}
    };

    tcflag_t BaudRate = B9600;
    const SpeedToFlag* ItSpeed = std::find_if(std::begin(SpeedToFlagTable), std::end(SpeedToFlagTable), [speed](SpeedToFlag const& t) {
        return t.nSpeed == speed;
    });
    if (ItSpeed != std::end(SpeedToFlagTable)) {
        BaudRate = ItSpeed->FlagSpeed;
    }
    return BaudRate;
}

bool TTYPort::Initialize() {

    _tty = open(GetPortName(), O_RDWR | O_NOCTTY);
    if (_tty < 0) {
        goto failed;
    }
    struct termios newtio;

    tcgetattr(_tty, &_oldtio); // save current port settings

    bzero(&newtio, sizeof (newtio));
    newtio.c_cflag = DecodeBaudrate(_dwPortSpeed) | CRTSCTS | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = _Timeout / 100; // inter-character timer ( 1/10sec unit)
    newtio.c_cc[VMIN] = 1; // blocking read until 5 chars received

    tcflush(_tty, TCIFLUSH);
    tcsetattr(_tty, TCSANOW, &newtio);

    return ComPort::Initialize();

failed:
    if (_tty >= 0) {
        tcsetattr(_tty, TCSANOW, &_oldtio);
        close(_tty);
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
        newtio.c_cflag = DecodeBaudrate(_dwPortSpeed) | (newtio.c_cflag & (~CBAUD));
        tcflush(_tty, TCIFLUSH);
        tcsetattr(_tty, TCSANOW, &newtio);
    }
    return oldSpeed;
}

unsigned long TTYPort::GetBaudrate() const {
    return _dwPortSpeed;
}

void TTYPort::Flush() {
    tcflush(_tty, TCIOFLUSH);
}

void TTYPort::Purge() {
    tcdrain(_tty);
}

void TTYPort::CancelWaitEvent() {

}

void TTYPort::UpdateStatus() {

}

size_t TTYPort::Read(void *szString, size_t size) {
    struct timeval timeout;
    fd_set readfs;
    timeout.tv_sec = _Timeout / 1000;
    timeout.tv_usec = _Timeout % 1000;

    int iResult = 0;
    FD_ZERO(&readfs);
    FD_SET(_tty, &readfs);

    // wait for received data
    iResult = select(_tty + 1, &readfs, NULL, NULL, &timeout);
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

    if (iResult == -1) {
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
    return true;
}

bool TTYPort::Write(const void *data, size_t length) {
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
        iResult = write(_tty, (const char*) data, length);
        if (iResult > 0) {
            AddStatTx(iResult);
            return true;
        }
    }

    if (iResult == -1) {
        AddStatErrTx(1);
        AddStatErrors(1);
        close(_tty);
        _tty = -1;

        return false;
    }

    return true;
}

DWORD TTYPort::RxThread() {
    DWORD dwWaitTime = 0;
    _Buff_t szString;
    Purge();

    while ((_tty != -1) && !StopEvt.tryWait(dwWaitTime)) {

        UpdateStatus();

        int nRecv = ReadData(szString);
        if (nRecv > 0) {
            std::for_each(std::begin(szString), std::begin(szString) + nRecv, std::tr1::bind(&TTYPort::ProcessChar, this, _1));
            dwWaitTime = 5; // avoid cpu overhead;
        } else {
            dwWaitTime = 50; // if no more data wait 50ms ( max data rate 20Hz )
        }
    }

    return 0UL;
}
#endif
