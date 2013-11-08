/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   CObexPush.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 1 novembre 2013, 20:04
 */

#include "externs.h"
#include "CObexPush.h"
#include <algorithm>
#include <functional>
#include <BtHandler.h>

CObexPush::CObexPush() : _pObex(NULL), _SavedBtState(HCI_HARDWARE_UNKNOWN) {

}

CObexPush::~CObexPush() {

}

bool CObexPush::Startup() {
    
    // Save state and Start Bt
    CBtHandler* pBtHandler = CBtHandler::Get();
    if(pBtHandler) {
        _SavedBtState = pBtHandler->GetHWState();
        pBtHandler->StartHW();
    }
    
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    HRESULT hr = CoCreateInstance(CLSID_Obex, NULL, CLSCTX_INPROC_SERVER, IID_IObex, (void **) &_pObex);
    if (FAILED(hr) || (_pObex == NULL)) {
        // TODO : Log Error
        return false;
    }
    hr = _pObex->Initialize();
    if (FAILED(hr)) {
        StartupStore(_T("Obex failed to Initialize %s"), NEWLINE);
        return false;
    }
    return true;
}

void CObexPush::Shutdown() {
    ClearDeviceList();
    if (_pObex) {
        _pObex->Shutdown();
    }
    CoUninitialize();
    
    // restore previous Bt state
    CBtHandler* pBtHandler = CBtHandler::Get();
    if(pBtHandler && HCI_HARDWARE_SHUTDOWN == _SavedBtState) {
        pBtHandler->StopHW();
    }
}

void ReleaseInterface(IUnknown* pUnk) {
    if (pUnk) {
        pUnk->Release();
    }
}

void CObexPush::ClearDeviceList() {
    std::for_each(_LstDevice.begin(), _LstDevice.end(), std::ptr_fun(ReleaseInterface));
    _LstDevice.clear();
}

size_t CObexPush::LookupDevice() {
    ClearDeviceList();

    IDeviceEnum *pDeviceEnum = NULL;
    HRESULT hRes = _pObex->StartDeviceEnum();
    if (FAILED(hRes)) {
        StartupStore(_T("Obex failed to Start Device Enum %s"), NEWLINE);
        return 0U;
    }
    Sleep(15000);
    //enumerate through the devices
    hRes = _pObex->StopDeviceEnum();
    if (FAILED(hRes)) {
        StartupStore(_T("Obex failed to Stop Device Enum %s"), NEWLINE);
        return 0U;
    }

    hRes = _pObex->EnumDevices(&pDeviceEnum, GUID_NULL);
    if (FAILED(hRes) || (pDeviceEnum == NULL)) {
        StartupStore(_T("Obex failed to Enum Device %s"), NEWLINE);
        return 0U;
    }

    DWORD num = 0;
    IObexDevice *pDevice = NULL;
    while (pDeviceEnum->Next(1, &pDevice, &num) == S_OK) {
        _LstDevice.push_back(pDevice);
    }
    pDeviceEnum->Release();

    return _LstDevice.size();
}

bool CObexPush::GetDeviceName(size_t DeviceIdx, TCHAR* szFileName, size_t cb) {
    ObexDeviceList_t::iterator ItDevice = _LstDevice.begin();
    std::advance(ItDevice, DeviceIdx);

    IPropertyBag *propBag = NULL;
    if (SUCCEEDED((*ItDevice)->EnumProperties(IID_IPropertyBag, (LPVOID *) & propBag))) {
        //print the name out
        VARIANT v;
        VariantInit(&v);
        if (SUCCEEDED(propBag->Read(L"Name", &v, NULL))) {
            _tcsncpy(szFileName, v.bstrVal, cb);
            szFileName[cb - 1] = _T('\0');
        } else {
            StartupStore(_T("Obex <%d> failed to get device name %s"), DeviceIdx, NEWLINE);
        }
        VariantClear(&v);
        propBag->Release();
        return true;
    }
    return false;
}

bool CObexPush::SendFile(size_t DeviceIdx, const TCHAR* szFileName) {
    ObexDeviceList_t::iterator ItDevice = _LstDevice.begin();
    std::advance(ItDevice, DeviceIdx);
    
    IObexDevice* pObexDevice = *ItDevice;
    IHeaderCollection *pHeaderCollection = NULL;
    //get a header collection
    HRESULT hr = CoCreateInstance(CLSID_HeaderCollection, NULL, CLSCTX_INPROC_SERVER, IID_IHeaderCollection, (void **) &pHeaderCollection);
    if (FAILED(hr) || (!pHeaderCollection)) {
        StartupStore(_T("Obex failed to Create Header Collection %s"), NEWLINE);
        pObexDevice->Release();
        return false;
    }
    
    //now, using the object, connect up
    hr = pObexDevice->Connect(0, 0, pHeaderCollection);
    if(SUCCEEDED(hr)) {
        pHeaderCollection->Release();
        pHeaderCollection = NULL;
    }
    
    if(FAILED(hr)) {
        TCHAR szDeviceName[100] = _T("");
        GetDeviceName(DeviceIdx, szDeviceName, 100);
        StartupStore(_T("Obex <%d> failed to connect <%s>%s"), DeviceIdx, szDeviceName, NEWLINE);
        return false;
    }

    // send the file content
    const WCHAR *name = wcsrchr(szFileName, '\\');
    if (!name)
        name = szFileName;
    else
        ++name;
    HANDLE hFile = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        hr = pObexDevice->Disconnect(pHeaderCollection);
        if(SUCCEEDED(hr)) {
            pHeaderCollection->Release();
            pHeaderCollection = NULL;
        }
        
        StartupStore(_T("Obex <%d> Failed to open file <%s>%s"), DeviceIdx, szFileName, NEWLINE);
        return false;
    }

    DWORD dwBytesSent = 0;
    IStream *myStream = 0;
    //get a header collection
    hr = CoCreateInstance(CLSID_HeaderCollection, NULL, CLSCTX_INPROC_SERVER, IID_IHeaderCollection, (void **) &pHeaderCollection);
    if (SUCCEEDED(hr)) {
        hr = pHeaderCollection->AddName(name);
    } else {
        StartupStore(_T("Obex <%d> Failed to create HeaderCollection%s"), DeviceIdx, NEWLINE);
    }
    if (SUCCEEDED(hr)) {
        hr = pObexDevice->Put(pHeaderCollection, &myStream);
    } else {
        StartupStore(_T("Obex <%d> Failed to set file name to header%s"), DeviceIdx, NEWLINE);
    }
    if (SUCCEEDED(hr)) {
        char inBuf[5000];
        DWORD written;
        ULONG cbJustRead;
        do {
            if (!ReadFile(hFile, inBuf, sizeof (inBuf), &cbJustRead, 0)) {
                StartupStore(_T("Obex <%d> Failed to read File shunck%s"), DeviceIdx, NEWLINE);
                break;
            }
            StartupStore(_T("Obex <%d> Write File shunck%s"), DeviceIdx, NEWLINE);
            hr = myStream->Write(inBuf, cbJustRead, &written);
            dwBytesSent += written;
        } while (SUCCEEDED(hr) && (cbJustRead == sizeof (inBuf)));
        hr = S_OK;
    } else {
        StartupStore(_T("Obex <%d> Failed to put File Name header%s"), DeviceIdx, NEWLINE);
    }
    
    bool bSendOK = false;
    if(hr == S_OK) {
        bSendOK = true;
        StartupStore(_T("Obex <%d> Send File Success%s"), DeviceIdx, NEWLINE);
    }

    if (myStream) {
        myStream->Release();
    }
    CloseHandle(hFile);

    if (pHeaderCollection) {
        pHeaderCollection->Release();
        pHeaderCollection = NULL;
    }
    
    hr = pObexDevice->Disconnect(pHeaderCollection);
    if(SUCCEEDED(hr)) {
        pHeaderCollection->Release();
        pHeaderCollection = NULL;
    }

    return bSendOK;
}


