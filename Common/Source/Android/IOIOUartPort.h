/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   IOIOUartPort.h
 * Author: Bruno de Lacheisserie
 */

#ifndef ANDROID_IOIOUARTPORT_H
#define ANDROID_IOIOUARTPORT_H


#include "AndroidPort.h"
class PortBridge;

class IOIOUartPort : public AndroidPort {
public:
    IOIOUartPort(int idx, const tstring& sName, unsigned baud) :
          AndroidPort(idx, sName),
          _ID(_tcstoul(&sName[9], nullptr, 10)),
          _baud(baud)
    {
    }

protected:
    PortBridge* CreateBridge() override;

    const unsigned _ID;
    const unsigned _baud;
};


#endif // ANDROID_IOIOUARTPORT_H
