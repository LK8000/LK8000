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

namespace {
    enum PortState {
        STATE_READY = 0,
        STATE_FAILED = 1,
        STATE_LIMBO = 2,
    };
}

AndroidPort::AndroidPort(int idx, const tstring& sName) : ComPort(idx, sName) {
    buffer.reserve(512);
}

bool AndroidPort::Initialize() {

    try {
        bridge = CreateBridge();
        if (bridge) {
            JNIEnv *env = Java::GetEnv();
            bridge->setInputListener(env, this);
            bridge->setListener(env, this);
            return true;
        }
    } catch (const std::exception& e) {
        delete std::exchange(bridge, nullptr); // required if `setInputListener` or `setListener` throw exception
        const tstring what = to_tstring(e.what());
        StartupStore(_T("FAILED! <%s>"), what.c_str());
    }
    StatusMessage(_T("%s %s"), MsgToken<762>(), GetPortName());
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

    auto dst_data = static_cast<char*>(szString);

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

    std::vector<char> rxthread_buffer;
    int state = STATE_LIMBO;

    do {
        bool stop = WithLock(mutex, [&]() {
            if (running && bridge) {
                state = bridge->getState(Java::GetEnv());
                assert(rxthread_buffer.empty());
                std::swap(rxthread_buffer, buffer);
                assert(buffer.empty());
                return false;
            }
            return true; // Stop RxThread...
        });

        if (stop) {
            return 0; // Stop RxThread...
        }

        if (!rxthread_buffer.empty() ) {
            WithLock(CritSec_Comm, [&]() {
                ProcessData(rxthread_buffer.data(), rxthread_buffer.size());
            });
            rxthread_buffer.clear();
        } else if (state == STATE_READY) {
            devOpen(devGetDeviceOnPort(GetPortIndex()));
        }

        ScopeLock lock(mutex);
        newdata.Wait(mutex); // wait for data or state change
    }
    while(true);
}
