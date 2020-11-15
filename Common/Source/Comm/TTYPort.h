/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   TTYPort.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 11 août 2014, 10:42
 */

#ifndef TTYPORT_H
#define	TTYPORT_H
#ifdef __linux__
#include "ComPort.h"
#include <atomic>
#include <termios.h>

class TTYPort : public ComPort {
public:
    TTYPort(int idx, const tstring& sName, unsigned dwSpeed, BitIndex_t BitSize, bool polling);
    virtual ~TTYPort();

    bool Initialize() override;
    bool Close() override;

    void Flush() override;
    void Purge() override;
    void CancelWaitEvent() override;

    int SetRxTimeout(int) override;
    unsigned long SetBaudrate(unsigned long) override;
    unsigned long GetBaudrate() const override;

    void UpdateStatus() override;

    size_t Read(void *szString, size_t size) override;
    bool Write(const void *data, size_t size) override;

protected:
    unsigned RxThread() override;

private:

    unsigned _dwPortSpeed;
    BitIndex_t _dwPortBit;

    int _tty;
    struct termios _oldtio;
    int _Timeout;

};
#endif
#endif	/* TTYPORT_H */
