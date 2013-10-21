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

#include <list>
#include "bthapi.h"
#include "boost/noncopyable.hpp"

#define BDSRC_LOOKUP 1
#define BDSRC_REGSVC 2
#define BDSRC_REGNAV 4
#define BDSRC_REGPIN 8

std::wstring BTAddrToStr(BT_ADDR ba);
BT_ADDR StrToBTAddr(const wchar_t* szAddr);

extern const std::wstring BTPortPrefix;

class CBtDevice {
    friend class CBtHandler;
public:
    CBtDevice(const BT_ADDR& ba, const std::wstring& csName );
    
    BT_ADDR m_ba;
    BYTE m_src;

    std::wstring GetName() const;
	std::wstring BTPortName() const;

    inline bool Equal_to(const BT_ADDR& ba) const {
        return m_ba == ba;
    }
protected:
    std::wstring m_csName;
};


typedef std::list<CBtDevice*> BtDeviceList_t;

class CBtHandler : public boost::noncopyable {
public:
	static void Release();
    static CBtHandler * Get();  

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

    void IntSavePowerState();
    void IntRestorePowerState();

    std::wstring CleanPort(const wchar_t* szPort) const;
    std::wstring GetPortSection(const wchar_t* szPort) const;
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

	static CBtHandler* m_pBtHandler;
};

#endif
