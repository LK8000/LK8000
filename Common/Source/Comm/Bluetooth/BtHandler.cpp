/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   BtHandler.cpp
 * Author: Bruno de Lacheisserie
 *
 * Adapted from original code provided by Naviter
 */

#include "externs.h"
#include <functional>
#include "utils/stl_utils.h"

using namespace std::placeholders;


#ifndef NO_BLUETOOTH

const tstring BTPortPrefix(_T("BT:"));

tstring BTAddrToStr(BT_ADDR ba) {
    TCHAR szAddress[25] = {0};
    _stprintf(szAddress, _T("%04x%08x"), GET_NAP(ba), (unsigned int)GET_SAP(ba));
    return szAddress;
}

BT_ADDR StrToBTAddr(const TCHAR* szAddr) {
    DWORD nap = 0, sap = 0;
    tstring csS, csTmp(szAddr);
    int iLen = csTmp.length();
    BT_ADDR ba = 0;

    if(iLen>=12) {
        csS = csTmp.substr(0, 4);
        nap = _tcstoul(csS.c_str(), nullptr, 16);

        csS = csTmp.substr(4, std::string::npos);
        sap = _tcstoul(csS.c_str(), nullptr, 16);

        ba = SET_NAP_SAP(nap, sap);
    }
    return ba;
}

tstring GetHandleFile(const TCHAR* szPort) {
    TCHAR szFile[25] = {0};
    _stprintf(szFile, _T("\\N%s.VPH"), szPort);
    return szFile;
}

//-------------

CBtDevice::CBtDevice(const BT_ADDR& ba, const tstring& csName) : m_ba(ba), m_csName(csName) {

}

tstring CBtDevice::GetName() const {
    tstring csTmp(m_csName);
    if (csTmp.empty())
        csTmp = BTAddrToStr(m_ba);
    return csTmp;
}

tstring CBtDevice::BTPortName() const {
    return BTPortPrefix + BTAddrToStr(m_ba);
}

//-------------

CBtHandler* CBtHandler::m_pBtHandler;

void CBtHandler::Release() {
    if (m_pBtHandler) {
        delete m_pBtHandler;
        m_pBtHandler = NULL;
    }
}

CBtHandler * CBtHandler::Get() {
    return m_pBtHandler;
}

CBtHandler::CBtHandler() {
    m_iSavedHWState = -1;
    m_bSavedScanMask = 0;
}

CBtHandler::~CBtHandler() {
    ClearDevices();
}

bool CBtHandler::StartHW() {
    return false;
}

bool CBtHandler::StopHW() {
    return false;
}

int CBtHandler::GetHWState() {
    return HCI_HARDWARE_UNKNOWN;
}

bool CBtHandler::IsOk() {
    return true;
}

bool CBtHandler::FillDevices() {
    return true;
}

bool CBtHandler::LookupDevices() {
    return false;
}

bool CBtHandler::Pair(BT_ADDR ba, const TCHAR* szDeviceName, const TCHAR* szPin) {
    return false;
}

bool CBtHandler::Unpair(BT_ADDR ba) {
    return false;
}

void CBtHandler::SavePowerState(int &iSavedHWState, BYTE &bSavedMask) {
}

void CBtHandler::IntSavePowerState() {
    if (m_iSavedHWState != -1)
        return;
    SavePowerState(m_iSavedHWState, m_bSavedScanMask);
}

void CBtHandler::RestorePowerState(int &iSavedHWState, BYTE &bSavedMask) {
}

void CBtHandler::IntRestorePowerState() {
    RestorePowerState(m_iSavedHWState, m_bSavedScanMask);
}

tstring CBtHandler::CleanPort(const TCHAR* szPort) const {
    tstring csPort(szPort);
    int i;

    i = csPort.find(_T(':'));
    if (i >= 0)
        csPort = csPort.substr(0, i);
    return csPort;
}

tstring CBtHandler::GetPortSection(const TCHAR* szPort) const {
    return _T("VCP_") + CleanPort(szPort);
}

CBtDevice* CBtHandler::FindDevice(const BT_ADDR& ba) const {
    BtDeviceList_t::const_iterator It = std::find_if(m_devices.begin(), m_devices.end(), std::bind(&CBtDevice::Equal_to, _1, ba));
    if (It != m_devices.end()) {
        return (*It);
    }
    return NULL;
}

CBtDevice* CBtHandler::AddDevice(CBtDevice *bt, BYTE bSrc) {
    CBtDevice *b = FindDevice(bt->m_ba);
    if (b) {
        b->m_src |= bSrc;
        if (b->m_csName.empty())
            b->m_csName = bt->m_csName;
        delete bt;
    } else {
        bt->m_src = bSrc;
        m_devices.push_back(bt);
        b = bt;
    }
    return b;
}

CBtDevice* CBtHandler::GetDevice(size_t idx) const {
    if (idx < m_devices.size()) {
        BtDeviceList_t::const_iterator It = m_devices.begin();
        std::advance(It, idx);
        if (It != m_devices.end()) {
            return (*It);
        }
    }
    return NULL;
}

void CBtHandler::RemoveDevice(const BT_ADDR& ba) {
    BtDeviceList_t::iterator It = std::find_if(m_devices.begin(), m_devices.end(), std::bind(&CBtDevice::Equal_to, _1, ba));
    if (It != m_devices.end()) {
        delete (*It);
        m_devices.erase(It);
    }
}

void CBtHandler::ClearDevices() {
    std::for_each(m_devices.begin(), m_devices.end(), safe_delete());
    m_devices.clear();
}

#endif
