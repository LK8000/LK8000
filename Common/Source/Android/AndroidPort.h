/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   AndroidPort.h
 * Author: Bruno de Lacheisserie
 */

#ifndef ANDROID_ANDROIDPORT_H
#define ANDROID_ANDROIDPORT_H


#include "externs.h"
#include "Comm/NullComPort.h"
#include "IO/DataHandler.hpp"
#include "Device/Port/Listener.hpp"
#include <boost/container/static_vector.hpp>

class PortBridge;

class AndroidPort : public ComPort, protected DataHandler, protected PortListener {
public:
    AndroidPort(int idx, const tstring& sName);

    /* override ComPort */
    bool Initialize() override;
    bool Close() override;

    bool StopRxThread() override;
    bool StartRxThread() override;

    void Purge() override;
    void Flush() override;
    void CancelWaitEvent() override;
    int SetRxTimeout(int TimeOut) override;

    void UpdateStatus() override {}

    unsigned long SetBaudrate(unsigned long) override;
    unsigned long GetBaudrate() const  override;

    bool Write(const void *data, size_t size) override;
    size_t Read(void *szString, size_t size) override;

protected:

    virtual bool CreateBridge() = 0;

    unsigned RxThread() override;

    /* protect fifo and flags*/
    Mutex mutex;
    /* singal new data available */
    Cond newdata;
    Cond newstate;

    int timeout;
    bool running;
    bool closing;

    boost::container::static_vector<uint8_t, 16*1024> buffer;

public:
    /* override Datahandler */
    void DataReceived(const void *data, size_t length) override;

public:
    /* override PortListner */
    void PortStateChanged() override;


protected:

    PortBridge *bridge;




};


#endif //ANDROID_ANDROIDPORT_H
