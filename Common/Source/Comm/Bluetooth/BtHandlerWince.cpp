/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   BtHandlerWince.cpp
 * Author: Bruno de Lacheisserie
 *
 * Adapted from original code provided by Naviter
 */

#include "BtHandlerWince.h"
#include "externs.h"

#if defined(PNA) && defined(UNDER_CE)


CBtHandler* CBtHandlerWince::Instance() {
    if (!m_pBtHandler) {
        m_pBtHandler = new CBtHandlerWince();
    }
    return m_pBtHandler;
}

CBtHandlerWince::CBtHandlerWince() {
    WSADATA wsd;

    m_hLibBtDrv = LoadLibrary(L"BTDrv.dll");
    if (m_hLibBtDrv) {
        m_StartBlueTooth = (BTOnOffProc) GetProcAddress(m_hLibBtDrv, L"StartBlueTooth");
        m_StopBlueTooth = (BTOnOffProc) GetProcAddress(m_hLibBtDrv, L"StopBlueTooth");
    } else {
#if TESTBENCH
        StartupStore(_T("Unable to Load <BTDrv.dll>%s"),NEWLINE);
#endif
    }

    m_hLibBthUtil = LoadLibrary(L"bthutil.dll");
    if (m_hLibBthUtil) {
        m_BthSetMode = (BthSetModeProc) GetProcAddress(m_hLibBthUtil, L"BthSetMode");
    } else {
#if TESTBENCH
        StartupStore(_T("Unable to Load <bthutil.dll>%s"),NEWLINE);
#endif
    }

    m_hLibBt = LoadLibrary(L"BTDrt.dll");
    if (m_hLibBt) {
        m_BthGetHardwareStatus = (BTHWStatusProc) GetProcAddress(m_hLibBt, L"BthGetHardwareStatus");
        m_BthReadScanEnableMask = (BTReadScanProc) GetProcAddress(m_hLibBt, L"BthReadScanEnableMask");
        m_BthWriteScanEnableMask = (BTWriteScanProc) GetProcAddress(m_hLibBt, L"BthWriteScanEnableMask");
        m_BthSetPIN = (BTSetPinProc) GetProcAddress(m_hLibBt, L"BthSetPIN");
        m_BthRevokePIN = (BTRevokePinProc) GetProcAddress(m_hLibBt, L"BthRevokePIN");
        m_BthRevokeLinkKey = (BTRevokeLinkKeyProc) GetProcAddress(m_hLibBt, L"BthRevokeLinkKey");
        m_BthAuthenticate = (BTAuthenticateProc) GetProcAddress(m_hLibBt, L"BthAuthenticate");
        m_BthCreateACLConnection = (BTCreateACLConnectionProc) GetProcAddress(m_hLibBt, L"BthCreateACLConnection");
        m_BthCloseConnection = (BTCloseConnectionProc) GetProcAddress(m_hLibBt, L"BthCloseConnection");
    } else {
#if TESTBENCH
        StartupStore(_T("Unable to Load <BTDrt.dll>%s"),NEWLINE);
#endif
    }

    WSAStartup(MAKEWORD(1, 1), &wsd);
}

CBtHandlerWince::~CBtHandlerWince() {
    WSACleanup();

    if (m_hLibBtDrv) {
        FreeLibrary(m_hLibBtDrv);
        m_hLibBtDrv = NULL;
    }

    if (m_hLibBthUtil) {
        FreeLibrary(m_hLibBthUtil);
        m_hLibBthUtil = NULL;
    }

    if (m_hLibBt) {
        FreeLibrary(m_hLibBt);
        m_hLibBt = NULL;
    }
}

bool CBtHandlerWince::StartHW() {
    int iStatus = GetHWState();
    if (iStatus == HCI_HARDWARE_RUNNING) {
        return true;
    }

    if (m_hLibBtDrv && m_StartBlueTooth) {
        StartupStore(_T("swith on Bluetooth%s"),NEWLINE);
        m_StartBlueTooth();
    } else if (m_hLibBthUtil && m_BthSetMode) {
        StartupStore(_T("swith on Bluetooth%s"),NEWLINE);
        m_BthSetMode(BTH_CONNECTABLE);
        Sleep(10000); // connecting failed with error WSAECONNREFUSED (10061) without that...
    } else {
        return false;
    }

    iStatus = GetHWState();
    return (iStatus == HCI_HARDWARE_RUNNING);
}

bool CBtHandlerWince::StopHW() {
    int iStatus = GetHWState();
    if (iStatus == HCI_HARDWARE_UNKNOWN || iStatus == HCI_HARDWARE_SHUTDOWN) {
        return true;
    }

    if (m_hLibBtDrv && m_StopBlueTooth) {
        StartupStore(_T("swith off Bluetooth%s"), NEWLINE);
        m_StopBlueTooth();
    } else if (m_hLibBthUtil && m_BthSetMode) {
        StartupStore(_T("swith off Bluetooth%s"), NEWLINE);
        m_BthSetMode(BTH_POWER_OFF);
    } else {
        return false;
    }

    iStatus = GetHWState();
    return (iStatus == HCI_HARDWARE_UNKNOWN || iStatus == HCI_HARDWARE_SHUTDOWN);
}

int CBtHandlerWince::GetHWState() {
    int iRet = ERROR_INVALID_FUNCTION;
    int iStatus = 0;
    int iLoop = 0;

    if (m_BthGetHardwareStatus) {
        while (1) {
            iRet = m_BthGetHardwareStatus(&iStatus);
            if (iRet != ERROR_SUCCESS)
                break;
            if (iStatus == HCI_HARDWARE_INITIALIZING || iStatus == HCI_HARDWARE_ERROR) {
                Sleep(100);
                ++iLoop;
                if (iLoop >= 100) {
                    break;
                }
            } else {
                break;
            }
        }
    }
    if (iRet != ERROR_SUCCESS) {
        iStatus = HCI_HARDWARE_UNKNOWN;
    }
    return iStatus;
}

bool CBtHandlerWince::IsOk() {
    bool b = (m_hLibBtDrv != NULL) && (m_StartBlueTooth != NULL) && (m_StopBlueTooth != NULL);
    if (!b) {
        b = (m_hLibBthUtil != NULL) && (m_BthSetMode != NULL);
    }
    if (b) {
        b = (m_hLibBt != NULL) && (m_BthGetHardwareStatus != NULL) && (m_BthReadScanEnableMask != NULL) && (m_BthWriteScanEnableMask != NULL);
    }
    if (b) {
        b = (m_BthSetPIN != NULL) && (m_BthRevokePIN != NULL) && (m_BthRevokeLinkKey != NULL) && (m_BthAuthenticate != NULL) && (m_BthCreateACLConnection != NULL) && (m_BthCloseConnection != NULL);
    }
    return b;
}

bool CBtHandlerWince::FillDevices() {
    CBtDevice *bt;
    HKEY key, kn;

    const wchar_t* szKey = L"Drivers\\BuiltIn\\BtPairSvc\\Devices";
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, 0, &key) == ERROR_SUCCESS) {
        wchar_t szSubKey[300];
        wchar_t szName[300];
        DWORD idx = 0;
        DWORD dwSize = 300;
        std::wstring csTmp, csName;

        while (RegEnumKeyEx(key, idx, szSubKey, &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            csTmp = szSubKey;
            csName.clear();
            if (RegOpenKeyEx(key, csTmp.c_str(), 0, 0, &kn) == ERROR_SUCCESS) {
                DWORD dwType;
                DWORD dwLen = 300;

                if (RegQueryValueEx(kn, L"DeviceName", NULL, &dwType, (LPBYTE) szName, &dwLen) == ERROR_SUCCESS)
                    csName = szName;
                RegCloseKey(kn);
            }

            bt = new CBtDevice(StrToBTAddr(csTmp.c_str()), csName);
#ifdef TESTBENCH
            StartupStore(_T("Bth (BuiltIn\\BtPairSvc\\Devices) : %s (%s)%s"), bt->GetName().c_str(), bt->BTPortName().c_str(), NEWLINE);
#endif
            AddDevice(bt, BDSRC_REGSVC);

            idx++;
            dwSize = 300;
        }

        RegCloseKey(key);
    }

    szKey = L"Software\\Naviter\\BtPair\\Devices";
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, 0, &key) == ERROR_SUCCESS) {
        wchar_t szSubKey[300];
        wchar_t szName[300];
        DWORD idx = 0;
        DWORD dwSize = 300;
        std::wstring csTmp, csName;

        while (RegEnumKeyEx(key, idx, szSubKey, &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            csTmp = szSubKey;
            csName.clear();
            if (RegOpenKeyEx(key, csTmp.c_str(), 0, 0, &kn) == ERROR_SUCCESS) {
                DWORD dwType;
                DWORD dwLen = 300;

                if (RegQueryValueEx(kn, L"DeviceName", NULL, &dwType, (LPBYTE) szName, &dwLen) == ERROR_SUCCESS)
                    csName = szName;
                RegCloseKey(kn);
            }

            bt = new CBtDevice(StrToBTAddr(csTmp.c_str()), csName);
#ifdef TESTBENCH
            StartupStore(_T("Bth (Software\\Naviter\\BtPair\\Devices) : %s (%s)%s"), bt->GetName().c_str(), bt->BTPortName().c_str(), NEWLINE);
#endif
            AddDevice(bt, BDSRC_REGNAV);

            idx++;
            dwSize = 300;
        }

        RegCloseKey(key);
    }

    szKey = L"Software\\Microsoft\\Bluetooth\\Device";
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, 0, &key) == ERROR_SUCCESS) {
        wchar_t szSubKey[300];
        wchar_t szName[300];
        DWORD idx = 0;
        DWORD dwSize = 300;
        std::wstring csTmp, csName;

        while (RegEnumKeyEx(key, idx, szSubKey, &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            csTmp = szSubKey;
            csName.clear();
            if (RegOpenKeyEx(key, csTmp.c_str(), 0, 0, &kn) == ERROR_SUCCESS) {
                DWORD dwType;
                DWORD dwLen = 300;

                if (RegQueryValueEx(kn, L"name", NULL, &dwType, (LPBYTE) szName, &dwLen) == ERROR_SUCCESS)
                    csName = szName;
                RegCloseKey(kn);
            }

            const BT_ADDR btAddr = StrToBTAddr(csTmp.c_str());
            if(btAddr) {
                bt = new CBtDevice(btAddr, csName);
#ifdef TESTBENCH
                StartupStore(_T("Bth (Software\\Microsoft\\Bluetooth\\Device) : %s (%s)%s"), bt->GetName().c_str(), bt->BTPortName().c_str(), NEWLINE);
#endif
                AddDevice(bt, BDSRC_REGNAV);
            }

            idx++;
            dwSize = 300;
        }

        RegCloseKey(key);
    }

    szKey = L"Comm\\Security\\bluetooth";
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, 0, &key) == ERROR_SUCCESS) {
        wchar_t szName[300];
        DWORD idx = 0;
        DWORD dwSize = 300;
        DWORD dwType;
        std::wstring csTmp;
        const std::wstring csEmpty = L"";

        while (RegEnumValue(key, idx, szName, &dwSize, NULL, &dwType, NULL, NULL) == ERROR_SUCCESS) {
            csTmp = szName;
            if (csTmp.find(L"pin") == 0) {
                csTmp = csTmp.substr(3);
                bt = new CBtDevice(StrToBTAddr(csTmp.c_str()), csEmpty);
#ifdef TESTBENCH
                StartupStore(_T("Bth (Comm\\Security\\bluetooth) : %s (%s)%s"), bt->GetName().c_str(), bt->BTPortName().c_str(), NEWLINE);
#endif
                AddDevice(bt, BDSRC_REGPIN);
            }
            idx++;
            dwSize = 300;
        }
        RegCloseKey(key);
    }
    return true;
}

bool CBtHandlerWince::LookupDevices() {
    WSAQUERYSET wsaq;
    HANDLE hLookup;

    ZeroMemory(&wsaq, sizeof (wsaq));
    wsaq.dwSize = sizeof (wsaq);
    wsaq.dwNameSpace = NS_BTH;
    wsaq.lpcsaBuffer = NULL;
    if (ERROR_SUCCESS != WSALookupServiceBegin(&wsaq, LUP_CONTAINERS, &hLookup)) {
        StartupStore(_T("CBtHandlerWince::LookupDevices: Failed%s"), NEWLINE);
        return false;
    }

    DWORD dwSize = 5000;
    LPWSAQUERYSET pwsaResults = (LPWSAQUERYSET)calloc(1, dwSize);
    pwsaResults->dwSize = sizeof (WSAQUERYSET);
    pwsaResults->dwNameSpace = NS_BTH;
    pwsaResults->lpBlob = NULL;

    while (ERROR_SUCCESS == WSALookupServiceNext(hLookup, LUP_RETURN_NAME | LUP_RETURN_ADDR, &dwSize, pwsaResults)) {
        if (pwsaResults->dwNumberOfCsAddrs > 0) {
            std::wstring csName = L"";
            if (pwsaResults->lpszServiceInstanceName) {
                csName = pwsaResults->lpszServiceInstanceName;
            }
            CBtDevice* bt = new CBtDevice(((SOCKADDR_BTH *) pwsaResults->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr, csName);
#ifdef TESTBENCH
            StartupStore(_T("Bth Lookup : %s (%s)%s"), bt->GetName().c_str(), bt->BTPortName().c_str(), NEWLINE);
#endif
            AddDevice(bt, BDSRC_LOOKUP);
        }
    }
    free(pwsaResults);
    WSALookupServiceEnd(hLookup);
    return true;
}

void CBtHandlerWince::SavePowerState(int &iSavedHWState, BYTE &bSavedMask) {
    iSavedHWState = HCI_HARDWARE_UNKNOWN;
    bSavedMask = 2;
    if (!m_BthReadScanEnableMask) {
        return;
    }
    iSavedHWState = GetHWState();
    m_BthReadScanEnableMask(&bSavedMask);
}

void CBtHandlerWince::RestorePowerState(int &iSavedHWState, BYTE &bSavedMask) {
    if (iSavedHWState != -1 && m_BthWriteScanEnableMask) {
        m_BthWriteScanEnableMask(bSavedMask);
        if (iSavedHWState == HCI_HARDWARE_RUNNING) {
            StartHW();
        } else {
            StopHW();
        }
        iSavedHWState = -1;
    }
}

bool CBtHandlerWince::Pair(BT_ADDR ba, const wchar_t* szDeviceName, const wchar_t* szPin) {
    unsigned char pin[16];
    int cPin = 0;
    while ((*szPin) && (cPin < 16)) {
        pin[cPin++] = (unsigned char) *(szPin++);
    }
    if (cPin) {
        int iRet = ERROR_INVALID_FUNCTION;
        if (m_BthSetPIN) {
            iRet = m_BthSetPIN(&ba, cPin, pin);
        }

        if (iRet != ERROR_SUCCESS) {
            return false;
        }
        iRet = ERROR_INVALID_PARAMETER;
        unsigned short h = 0;
        if (m_BthCreateACLConnection && m_BthAuthenticate && m_BthCloseConnection) {
            iRet = m_BthCreateACLConnection(&ba, &h);
            if (iRet == ERROR_SUCCESS) {
                iRet = m_BthAuthenticate(&ba);
                m_BthCloseConnection(h);
            }
        } else {
            iRet = ERROR_INVALID_FUNCTION;
        }
        if (iRet != ERROR_SUCCESS) {
            return false;
        }
    } else {
        // UNCOM what to do if no pin???
    }

    std::wstring csKey, csTmp;
    HKEY key;
    DWORD dwDisp;

    csKey = L"Software\\Naviter\\BtPair";
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, csKey.c_str(), 0, NULL, 0, 0, NULL, &key, &dwDisp) == ERROR_SUCCESS) {
        RegCloseKey(key);
        csTmp = L"\\Devices\\" + BTAddrToStr(ba);
        csKey += csTmp;
        if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, csKey.c_str(), 0, NULL, 0, 0, NULL, &key, &dwDisp) == ERROR_SUCCESS) {
            DWORD dw = 1;
            RegSetValueEx(key, L"ProfileCount", 0, REG_DWORD, (LPBYTE) & dw, sizeof (dw));
            if (szDeviceName && *szDeviceName)
                RegSetValueEx(key, L"DeviceName", 0, REG_SZ, (LPBYTE) szDeviceName, (wcslen(szDeviceName) + 1) * sizeof (wchar_t));
            RegCloseKey(key);
            csKey += L"\\0";
            if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, csKey.c_str(), 0, NULL, 0, 0, NULL, &key, &dwDisp) == ERROR_SUCCESS) {
                dw = 0x00001101;
                RegSetValueEx(key, L"ProfileID", 0, REG_DWORD, (LPBYTE) & dw, sizeof (dw));
                RegCloseKey(key);
            }
        }
    }

    return true;
}

/*  RegDelnodeRecurse()
 *  Purpose:    Deletes a registry key and all its subkeys / values.
 *  Parameters: hKeyRoot    -   Root key
 *              lpSubKey    -   SubKey to delete
 *  Return:     true if successful.
 *              false if an error occurs.
 */
bool RegDelnodeRecurse(HKEY hKeyRoot, const wchar_t* lpKey) {
    wchar_t lpSubKey[MAX_PATH];
    wchar_t* lpEnd;
    LONG lResult;
    DWORD dwSize;
    wchar_t szName[MAX_PATH];
    HKEY hKey;
    FILETIME ftWrite;

    wcscpy(lpSubKey, lpKey);

    // First, see if we can delete the key without having
    // to recurse.
    lResult = RegDeleteKey(hKeyRoot, lpSubKey);
    if (lResult == ERROR_SUCCESS)
        return true;

    lResult = RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);
    if (lResult != ERROR_SUCCESS) {
        if (lResult == ERROR_FILE_NOT_FOUND) {
            return true;
        } else {
            return false;
        }
    }

    // Check for an ending slash and add one if it is missing.
    lpEnd = lpSubKey + lstrlen(lpSubKey);
    if (*(lpEnd - 1) != TEXT('\\')) {
        *lpEnd = TEXT('\\');
        lpEnd++;
        *lpEnd = TEXT('\0');
    }

    // Enumerate the keys
    dwSize = MAX_PATH;
    lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL, NULL, NULL, &ftWrite);
    if (lResult == ERROR_SUCCESS) {
        do {
            wcscpy(lpEnd, szName);
            if (!RegDelnodeRecurse(hKeyRoot, lpSubKey)) {
                break;
            }

            dwSize = MAX_PATH;
            lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                    NULL, NULL, &ftWrite);
        } while (lResult == ERROR_SUCCESS);
    }

    lpEnd--;
    *lpEnd = TEXT('\0');

    RegCloseKey(hKey);

    // Try again to delete the key.
    lResult = RegDeleteKey(hKeyRoot, lpSubKey);
    if (lResult == ERROR_SUCCESS) {
        return true;
    }

    return false;
}

/*
 *  RegDelnode()
 *
 *  Purpose:    Deletes a registry key and all its subkeys / values.
 *  Parameters: hKeyRoot    -   Root key
 *              lpSubKey    -   SubKey to delete
 *  Return:     true if successful.
 *              false if an error occurs.
 */
bool DelRegTree(HKEY hKeyRoot, const std::wstring& sSubKey) {
    wchar_t szDelKey[MAX_PATH * 2];
    wcscpy(szDelKey, sSubKey.c_str());
    return RegDelnodeRecurse(hKeyRoot, szDelKey);
}

bool CBtHandlerWince::Unpair(BT_ADDR ba) {
    int iRet;
    if (m_BthRevokePIN)
        iRet = m_BthRevokePIN(&ba);
    else
        iRet = ERROR_INVALID_FUNCTION;
    if (m_BthRevokeLinkKey)
        iRet = m_BthRevokeLinkKey(&ba);
    else
        iRet = ERROR_INVALID_FUNCTION;

    if (iRet != ERROR_INVALID_FUNCTION) {
        std::wstring csTmp;

        csTmp = L"Drivers\\BuiltIn\\BtPairSvc\\Devices\\" + BTAddrToStr(ba);
        DelRegTree(HKEY_LOCAL_MACHINE, csTmp.c_str());

        csTmp = L"Software\\Microsoft\\Bluetooth\\Device\\" + BTAddrToStr(ba);
        DelRegTree(HKEY_LOCAL_MACHINE, csTmp.c_str());

        csTmp = L"Software\\Naviter\\BtPair\\Devices\\" + BTAddrToStr(ba);
        DelRegTree(HKEY_LOCAL_MACHINE, csTmp.c_str());
    }
    return true;
}

CBthDevice* CBthDevice::m_pInstance = NULL;

CBthDevice* CBthDevice::Instance() {
    if (!m_pInstance) {
		m_pInstance = new CBthDevice();
    }
    return m_pInstance;
}

void CBthDevice::Release() {
    if (m_pInstance) {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}

CBthDevice::CBthDevice() {
    StartupStore(_T("Init Bluetooth%s"), NEWLINE);
    CBtHandler::Release(); // release previous instance of sigleton if exist
    CBtHandler* pBtHandler = CBtHandlerWince::Instance(); // instance singleton
    if (pBtHandler && pBtHandler->IsOk()) {
        StartupStore(_T("Bluetooth Is OK%s"), NEWLINE);
        pBtHandler->IntSavePowerState(); // save HW power state
        pBtHandler->FillDevices(); // fill devices list with already paired device

        StartupStore(_T("Bluetooth : Device Count = %u%s"), pBtHandler->m_devices.size(), NEWLINE);
    }
}

CBthDevice::~CBthDevice() {
    CBtHandler* pBtHandler = CBtHandler::Get();
    if (pBtHandler && pBtHandler->IsOk()) {
		StartupStore(_T("Restore Bluetooth power state%s"), NEWLINE);
        pBtHandler->IntRestorePowerState();
    }
    CBtHandler::Release();
}


#endif
