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

class IOIOUartPort : public AndroidPort {
public:
    IOIOUartPort(int idx, const tstring& sName, unsigned ID, unsigned baud) : AndroidPort(idx, sName), _ID(ID), _baud(baud) { }

    bool CreateBridge() override;

protected:
    const unsigned _ID;
    const unsigned _baud;
};


#endif // ANDROID_IOIOUARTPORT_H
