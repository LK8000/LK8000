/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   AndroidPort.cpp
 * Author: Bruno de Lacheisserie
 */

#include "AndroidPort.h"
#include "Android/PortBridge.hpp"
#include "Android/BluetoothHelper.hpp"
#include "OS/Sleep.h"
#include "functional"

using namespace std::placeholders;

AndroidPort::AndroidPort(int idx, const tstring& sName) : ComPort(idx, sName) {
    buffer.reserve(512);
    rxthread_buffer.reserve(512);
}

bool AndroidPort::Initialize() {

    try {
        JNIEnv *env = Java::GetEnv();
        if (CreateBridge()) {
            assert(bridge);
            bridge->setInputListener(env, this);
            bridge->setListener(env, this);

            return true;
        }
    } catch (const std::exception& e) {
        delete std::exchange(bridge, nullptr); // required if `setInputListener` or `setListener` throw exception
        const tstring what = to_tstring(e.what());
        StartupStore(_T("FAILED! <%s>" NEWLINE), what.c_str());
    }
    StatusMessage(mbOk, NULL, TEXT("%s %s"), MsgToken(762), GetPortName());
    return false;
}

bool AndroidPort::Close() {

    PortBridge * delete_bridge = WithLock(mutex, [&]() {
        running = false;
        return std::exchange(bridge, nullptr);
    });

    while(!ComPort::Close()) {
        Sleep(10);
    }

    delete delete_bridge;

    return true;
}

bool AndroidPort::StopRxThread() {
    WithLock(mutex, [&]() {
        running = false;
    });

    if(ComPort::StopRxThread()) {
        return true;
    }
    return false;
}

bool AndroidPort::StartRxThread() {
    ScopeLock lock(mutex);
    running = true;

    return ComPort::StartRxThread();
}


void AndroidPort::Purge() {
    if(bridge) {
        bridge->drain(Java::GetEnv());
    }
}

void AndroidPort::Flush() {
    ScopeLock protect(mutex);
    buffer.clear();
}

void AndroidPort::CancelWaitEvent() {
    newdata.Broadcast();
}

int AndroidPort::SetRxTimeout(int TimeOut) {
    int old_timeout = timeout;
    timeout = TimeOut;
    return old_timeout;
}


unsigned long AndroidPort::SetBaudrate(unsigned long baud_rate) {
    if(bridge) {
        unsigned long old = GetBaudrate();
        bridge->setBaudRate(Java::GetEnv(), baud_rate);
        return old;
    }
    return 0;
}

unsigned long AndroidPort::GetBaudrate() const {
    if(bridge) {
        return bridge->getBaudRate(Java::GetEnv());
    }
    return 0;
}

bool AndroidPort::Write_Impl(const void *data, size_t size) {
    if(bridge) {
        const char *p = (const char *)data;
        const char *end = p + size;

        while (p < end) {
            int nbytes = bridge->write(Java::GetEnv(), p, end - p);
            if (nbytes <= 0) {
                return false;
            }
            AddStatTx(nbytes);

            p += nbytes;
        }
        return true;
    }
    return false;
}

size_t AndroidPort::Read(void *szString, size_t size) {

    ScopeLock lock(mutex);
    assert(!running);

    if(buffer.empty()) {
        newdata.Wait(mutex, timeout);
    }

    if(running || buffer.empty()) {
        return 0;
    }

    const size_t consume_size = std::min(size, buffer.size());

    const auto src_begin = buffer.begin();
    const auto src_end = std::next(src_begin, consume_size);

    uint8_t *dst_data = static_cast<uint8_t*>(szString);

    std::copy(src_begin, src_end, dst_data);

    buffer.erase(src_begin, src_end);

    return consume_size;
}


void AndroidPort::DataReceived(const void *data, size_t length) {

    ScopeLock lock(mutex);
    const auto *src_data = static_cast<const uint8_t *>(data);

    // limit vector size to 16 KByte
    const size_t available_size =  (16U * 1024U) - buffer.size();
    const size_t insert_size = std::min(available_size, length);

    buffer.insert(buffer.cend(), src_data,  std::next(src_data, insert_size));

    AddStatRx(insert_size);
    if(insert_size < length) {
        AddStatErrRx(length - insert_size);
    }

    newdata.Broadcast();
}

enum PortState {
    STATE_READY = 0,
    STATE_FAILED = 1,
    STATE_LIMBO = 2,
};

bool AndroidPort::IsReady() {
    ScopeLock lock(mutex);
    if (bridge) {
        return bridge->getState(Java::GetEnv()) == STATE_READY;
    }
    return false;
}

void AndroidPort::PortStateChanged() {
    newdata.Signal();
}

void AndroidPort::PortError(const char *msg) {
    StartupStore(_T("ComPort Error : %s"), msg);
}

unsigned AndroidPort::RxThread() {

    ScopeLock lock(mutex);

    while( running ) {

        newdata.Wait(mutex);

        if (buffer.empty()) {

            if (!bridge) {
                SetPortStatus(CPS_OPENKO);
                // port is Closed.
                running = false;
                return 0; // Stop RxThread...
            }

            switch (bridge->getState(Java::GetEnv())) {
                case STATE_READY:
                    SetPortStatus(CPS_OPENOK);
                    devOpen(devGetDeviceOnPort(GetPortIndex()));
                    break;
                case STATE_FAILED:
                    SetPortStatus(CPS_OPENKO);
                    break;
                case STATE_LIMBO:
                    SetPortStatus(CPS_OPENWAIT);
                    break;
                default:
                    SetPortStatus(CPS_OPENKO);
                    break;
            }

        } else {
            std::swap(rxthread_buffer, buffer);
            buffer.clear();
            ScopeUnlock unlock(mutex); // workaround to prevent deadlock on shutdown

            WithLock(CritSec_Comm, [&]() {
                std::for_each(std::begin(rxthread_buffer), std::end(rxthread_buffer), GetProcessCharHandler());
            });
        }
    }

    return 0;
}
