/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   BtHandlerWince.h
 * Author: Bruno de Lacheisserie
 *
 * Adapted from original code provided by Naviter
 */

#ifndef CBTHANDLERWINCE_H
#define	CBTHANDLERWINCE_H

#if defined(PNA) && defined(UNDER_CE)

#include <winsock2.h>
#include <windows.h>
#include <string>
#include <list>
#include "BtHandler.h"

typedef void (APIENTRY* BTOnOffProc)(void);
typedef int (APIENTRY* BTHWStatusProc)(int *pistatus);
typedef int (APIENTRY* BTReadScanProc)(unsigned char *pmask);
typedef int (APIENTRY* BTWriteScanProc)(unsigned char mask);
typedef int (APIENTRY* BTSetPinProc)(BT_ADDR *pba, int cPinLength, unsigned char *ppin);
typedef int (APIENTRY* BTRevokePinProc)(BT_ADDR *pba);
typedef int (APIENTRY* BTRevokeLinkKeyProc)(BT_ADDR *pba);
typedef int (APIENTRY* BTAuthenticateProc)(BT_ADDR *pba);
typedef int (APIENTRY* BTCreateACLConnectionProc)(BT_ADDR *pbt, unsigned short *phandle);
typedef int (APIENTRY* BTCloseConnectionProc)(unsigned short handle);

typedef int (APIENTRY* BthSetModeProc) (DWORD dwMode);

class CBtHandlerWince : public CBtHandler {
public:
    static CBtHandler* Instance();

    virtual bool StartHW();
    virtual bool StopHW();
    virtual int GetHWState();
    virtual bool IsOk();

    virtual bool FillDevices();
    virtual bool LookupDevices();

    virtual bool Pair(BT_ADDR ba, const wchar_t* szDeviceName, const wchar_t* szPin);
    virtual bool Unpair(BT_ADDR ba);

    virtual void SavePowerState(int &iSavedHWState, BYTE &bSavedMask);
    virtual void RestorePowerState(int &iSavedHWState, BYTE &bSavedMask);

protected:
    HINSTANCE m_hLibBtDrv;
    BTOnOffProc m_StartBlueTooth, m_StopBlueTooth;

    HINSTANCE m_hLibBthUtil;
    BthSetModeProc m_BthSetMode;

    HINSTANCE m_hLibBt;
    BTHWStatusProc m_BthGetHardwareStatus;
    BTReadScanProc m_BthReadScanEnableMask;
    BTWriteScanProc m_BthWriteScanEnableMask;
    BTSetPinProc m_BthSetPIN;
    BTRevokePinProc m_BthRevokePIN;
    BTRevokeLinkKeyProc m_BthRevokeLinkKey;
    BTAuthenticateProc m_BthAuthenticate;
    BTCreateACLConnectionProc m_BthCreateACLConnection;
    BTCloseConnectionProc m_BthCloseConnection;

    CBtHandlerWince();
    virtual ~CBtHandlerWince();
};

class CBthDevice {
public:
    // singleton accessor
    static CBthDevice* Instance();
    static void Release();

protected:
    CBthDevice();
    ~CBthDevice();

    CBthDevice( const CBthDevice& ) = delete;
    CBthDevice& operator=( const CBthDevice& ) = delete;

private:
    static CBthDevice* m_pInstance;
};

#endif
#endif	/* CBTHANDLERWINCE_H */
