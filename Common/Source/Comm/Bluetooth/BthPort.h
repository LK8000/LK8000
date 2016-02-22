/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *  
 * File:   BthPort.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 2 août 2013, 14:37
 */

#ifndef BTHPORT_H
#define	BTHPORT_H

#ifndef NO_BLUETOOTH
#include "../SocketPort.h"

class BthPort : public SocketPort {
public:
    BthPort(int idx, const std::tstring& sName) : SocketPort(idx, sName) { }
    
protected:
    virtual bool Connect();
};
#else
#include "../NullComPort.h"
typedef NullComPort BthPort;
#endif
#endif	/* BTHPORT_H */

