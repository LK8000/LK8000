/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   BtHandler.h
 * Author: Bruno de Lacheisserie
 * 
 * Adapted from original code provided by Naviter
 */

#if !defined(_COMMON_BTHANDLER_H_)
#define _COMMON_BTHANDLER_H_
#ifndef NO_BLUETOOTH
#include <list>
#include "bthapi.h"
#include "utils/tstring.h"

#define BDSRC_LOOKUP 1
#define BDSRC_REGSVC 2
#define BDSRC_REGNAV 4
#define BDSRC_REGPIN 8

std::tstring BTAddrToStr(BT_ADDR ba);
BT_ADDR StrToBTAddr(const TCHAR* szAddr);

extern const std::tstring BTPortPrefix;

class CBtDevice {
    friend class CBtHandler;
public:
    CBtDevice(const BT_ADDR& ba, const std::tstring& csName );
    
    BT_ADDR m_ba;
    BYTE m_src;

    std::tstring GetName() const;
	std::tstring BTPortName() const;

    inline bool Equal_to(const BT_ADDR ba) const {
        return m_ba == ba;
    }
protected:
    std::tstring m_csName;
};


typedef std::list<CBtDevice*> BtDeviceList_t;

class CBtHandler {
public:
	static void Release();
    static CBtHandler * Get();  

    virtual bool StartHW();
    virtual bool StopHW();
    virtual int GetHWState();
    virtual bool IsOk();

    virtual bool FillDevices();
    virtual bool LookupDevices();

    virtual bool Pair(BT_ADDR ba, const TCHAR* szDeviceName, const TCHAR* szPin);
    virtual bool Unpair(BT_ADDR ba);

    virtual void SavePowerState(int &iSavedHWState, BYTE &bSavedMask);
    virtual void RestorePowerState(int &iSavedHWState, BYTE &bSavedMask);

    void IntSavePowerState();
    void IntRestorePowerState();

    std::tstring CleanPort(const TCHAR* szPort) const;
    std::tstring GetPortSection(const TCHAR* szPort) const;
    CBtDevice* FindDevice(const BT_ADDR& ba) const;
    CBtDevice* AddDevice(CBtDevice *bt, BYTE bSrc);
    CBtDevice* GetDevice(size_t idx) const;
    void RemoveDevice(const BT_ADDR& ba);

    void ClearDevices();
    BtDeviceList_t m_devices;

protected:
    int m_iSavedHWState;
    BYTE m_bSavedScanMask;

    CBtHandler();
    virtual ~CBtHandler();
    
    CBtHandler( const CBtHandler& ) = delete;
    CBtHandler& operator=( const CBtHandler& ) = delete;    

    static CBtHandler* m_pBtHandler;
};

#endif
#endif
