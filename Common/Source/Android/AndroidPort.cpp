/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
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

AndroidPort::AndroidPort(int idx, const tstring& sName) : ComPort(idx, sName),
                                                          running(), closing(), bridge() {

}

bool AndroidPort::Initialize() {

    try {
        JNIEnv *env = Java::GetEnv();
        if (CreateBridge()) {
            assert(bridge);
            bridge->setInputListener(env, this);
            bridge->setListener(env, this);

            return ComPort::Initialize();
        }
    } catch (const std::exception& e) {
        const tstring what = to_tstring(e.what());
        StartupStore(_T("FAILED! <%s>" NEWLINE), what.c_str());
    }
    StatusMessage(mbOk, NULL, TEXT("%s %s"), MsgToken(762), GetPortName());
    return false;
}

bool AndroidPort::Close() {

    PortBridge * delete_bridge = bridge;
    {
        ScopeLock lock(mutex);
        running = false;
        bridge = nullptr;
    }

    while(!ComPort::Close()) {
        Sleep(10);
    }

    delete delete_bridge;

    return true;
}

bool AndroidPort::StopRxThread() {

    if(ComPort::StopRxThread()) {
        ScopeLock lock(mutex);
        running = false;
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

void AndroidPort::CancelWaitEvent() {
    newdata.Broadcast();
    newstate.Broadcast();
};

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

bool AndroidPort::Write(const void *data, size_t length) {
    if(bridge) {
        AddStatTx(length);
        return bridge->write(Java::GetEnv(), data, length);
    }
    return false;
}

size_t AndroidPort::Read(void *szString, size_t size) {

    ScopeLock lock(mutex);
    assert(!running);

    if(buffer.empty()) {
        newdata.Wait(mutex);
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

    AddStatRx(length);

    if(running) {
        const char *string_data = static_cast<const char *>(data);

        std::for_each(string_data,
                      string_data + length,
                      std::bind(&AndroidPort::ProcessChar, this, _1));

    } else {
        ScopeLock lock(mutex);
        const uint8_t *src_data = static_cast<const uint8_t *>(data);

        const size_t available_size =  buffer.capacity() - buffer.size();
        const size_t insert_size = std::min(available_size, length);

        buffer.insert(buffer.cend(), src_data,  src_data + insert_size);

        newdata.Broadcast();
    }
}

enum PortState {
    STATE_READY = 0,
    STATE_FAILED = 1,
    STATE_LIMBO = 2,
};

void AndroidPort::PortStateChanged() {
    newstate.Signal();
}

unsigned AndroidPort::RxThread() {

    ScopeLock lock(mutex);

    PortBridge * active_bridge = bridge;
    while( running ) {

        newstate.Wait(mutex);

        PDeviceDescriptor_t d = devGetDeviceOnPort(GetPortIndex());
        if (d) {
            switch(active_bridge->getState(Java::GetEnv())) {
                case STATE_READY :
                    d->Status = CPS_OPENOK;
                    break;
                case STATE_FAILED:
                    d->Status = CPS_OPENKO;
                    break;
                case STATE_LIMBO:
                    d->Status = CPS_OPENWAIT;
                    break;
                default:
                    d->Status = CPS_OPENKO;
                    break;
            }
        }

        if(!bridge) {
            d->Status = CPS_OPENKO;
            // port is Closed.
            break;
        }
    }

    return 0;
}