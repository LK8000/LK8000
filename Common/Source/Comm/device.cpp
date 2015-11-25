/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: device.cpp,v 8.6 2010/12/13 10:21:06 root Exp root $
 */

#include "externs.h"
#include "Dialogs/dlgProgress.h"
#include "utils/stl_utils.h"
#include "BtHandler.h"
#include "SerialPort.h"
#include "Bluetooth/BthPort.h"
#include "GpsIdPort.h"
#include "devPVCOM.h"
#include <functional>
#ifdef __linux__
#include <dirent.h>
#include <unistd.h>

#ifdef KOBO
#include "Kobo/System.hpp"
#endif // KOBO

#endif // __linux__

using namespace std::placeholders;

// A note about locking.
//  The ComPort RX threads lock using FlightData critical section.
//  ComPort::StopRxThread and ComPort::Close both wait for these threads to
//  exit before returning.  Both these functions are called with the Comm
//  critical section locked.  Therefore, there is a locking dependency upon
//  Comm -> FlightData.
//
//  If someone locks FlightData and then Comm, there is a high possibility
//  of deadlock.  So, FlightData must never be locked after Comm.  Ever.
//  Thankfully WinCE "critical sections" are recursive locks.

// this lock is used for protect DeviceList array.
Poco::Mutex CritSec_Comm;

COMMPort_t COMMPort;

static const unsigned dwSpeed[] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};

DeviceRegister_t DeviceRegister[NUMREGDEV];
DeviceDescriptor_t DeviceList[NUMDEV];

DeviceDescriptor_t *pDevPrimaryBaroSource = NULL;
DeviceDescriptor_t *pDevSecondaryBaroSource = NULL;

int DeviceRegisterCount = 0;




void LockComm() {
    CritSec_Comm.lock();
}

void UnlockComm() {
    CritSec_Comm.unlock();
}

static BOOL FlarmDeclare(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[]);
static void devFormatNMEAString(TCHAR *dst, size_t sz, const TCHAR *text);

// This function is used to determine whether a generic
// baro source needs to be used if available

BOOL devHasBaroSource(void) {
    if (pDevPrimaryBaroSource || pDevSecondaryBaroSource) {
        return TRUE;
    } else {
        return FALSE;
    }
}

#if 0

BOOL devGetBaroAltitude(double *Value) {
    // hack, just return GPS_INFO->BaroAltitude
    if (Value == NULL)
        return (FALSE);
    if (GPS_INFO.BaroAltitudeAvailable)
        *Value = GPS_INFO.BaroAltitude;
    return (TRUE);

}
#endif

BOOL ExpectString(PDeviceDescriptor_t d, const TCHAR *token) {

    int i = 0, ch;

    if (!d->Com)
        return FALSE;

    while ((ch = d->Com->GetChar()) != EOF) {

        if (token[i] == (TCHAR) ch)
            i++;
        else
            i = 0;

        if ((unsigned) i == _tcslen(token))
            return (TRUE);

    }

    return (FALSE);

}

BOOL devRegister(const TCHAR *Name, int Flags,
        BOOL(*Installer)(PDeviceDescriptor_t d)) {
    if (DeviceRegisterCount >= NUMREGDEV) {
        LKASSERT(FALSE);
        return (FALSE);
    }
    DeviceRegister[DeviceRegisterCount].Name = Name;
    DeviceRegister[DeviceRegisterCount].Flags = Flags;
    DeviceRegister[DeviceRegisterCount].Installer = Installer;
    DeviceRegisterCount++;
    return (TRUE);
}

BOOL devRegisterGetName(int Index, TCHAR *Name) {
    Name[0] = '\0';
    if (Index < 0 || Index >= DeviceRegisterCount)
        return (FALSE);
    _tcscpy(Name, DeviceRegister[Index].Name);
    return (TRUE);
}

// This device is not available if Disabled
// Index 0 or 1 

bool devIsDisabled(int Index) {
    if (Index < 0 || Index > 1)
        return (true);

    return DeviceList[Index].Disabled;
}

static BOOL devIsFalseReturn(PDeviceDescriptor_t d) {
    (void) d;
    return (FALSE);

}

void RefreshComPortList() {
    COMMPort.clear();
#ifdef WIN32    
    TCHAR szPort[10];
    for (unsigned i = 1; i < 10; ++i) {
        _stprintf(szPort, _T("COM%u"), i);
        COMMPort.push_back(szPort);
    }

#ifndef UNDER_CE
    for (unsigned i = 10; i < 41; ++i) {
        _stprintf(szPort, _T("COM%u"), i);
        COMMPort.push_back(szPort);
    }
#endif

    COMMPort.push_back(_T("COM0"));

#if defined(PNA) && defined(UNDER_CE)
    COMMPort.push_back(_T("VSP0"));
    COMMPort.push_back(_T("VSP1"));
#endif
#endif

#ifdef __linux__

    struct dirent **namelist;
    int n;
    if (IsKobo()) {
        n = scandir("/dev", &namelist, 0, alphasort); //need test
    } else {
        n = scandir("/sys/class/tty", &namelist, 0, alphasort); //which is faster than /dev/
    }
    if (n != -1) {
        for (int i = 0; i < n; ++i) {
            bool portok = true;
            if (memcmp(namelist[i]->d_name, "tty", 3) == 0) {
                // filter out "/dev/tty0", ... (valid integer after "tty") 
                char *endptr;
                strtoul(namelist[i]->d_name + 3, &endptr, 10);
                if (*endptr == 0) {
                    portok = false;
                }
            } else if ((memcmp(namelist[i]->d_name, "rfcomm", 6) != 0)
                    && (memcmp(namelist[i]->d_name, "tnt", 3) != 0)) {
                // '/dev/tntX' are tty0tty virtual port  
                portok = false;
            }
            if (portok) {
                char path[64];
                snprintf(path, sizeof (path), "/dev/%s", namelist[i]->d_name);
                if (access(path, R_OK | W_OK) == 0 && access(path, X_OK) < 0) {
                    COMMPort.push_back(path);
                }
            }
            free(namelist[i]);
        }
    }
    free(namelist);

#ifdef KOBO
    if (KoboExportSerialAvailable()) {
        if (std::find_if(COMMPort.begin(), COMMPort.end(), std::bind(&COMMPortItem_t::IsSamePort, _1, _T("/dev/ttyGS0"))) == COMMPort.end()) {
            COMMPort.push_back(_T("/dev/ttyGS0"));
        }
    }

#elif TESTBENCH
    COMMPort.push_back(_T("/lk/ptycom1"));
    COMMPort.push_back(_T("/lk/ptycom2"));
    COMMPort.push_back(_T("/lk/ptycom3"));
    COMMPort.push_back(_T("/lk/ptycom4"));
#endif

#endif

#ifndef NO_BLUETOOTH
    CBtHandler* pBtHandler = CBtHandler::Get();
    if (pBtHandler) {
        std::copy(
                pBtHandler->m_devices.begin(),
                pBtHandler->m_devices.end(),
                std::back_insert_iterator<COMMPort_t > (COMMPort)
                );
    }
#endif

    if (COMMPort.empty()) {
        // avoid segfault on device config  dialog if no comport detected.
        COMMPort.push_back(_T("Null"));
    }
}

void DeviceDescriptor_t::InitStruct(int i) {
    Port = -1;
    Name[0] = '\0';
    ParseNMEA = NULL;
    PutMacCready = NULL;
    DirectLink = NULL;
    PutBugs = NULL;
    PutBallast = NULL;
    Open = NULL;
    Close = NULL;
    Init = NULL;
    LinkTimeout = NULL;
    Declare = NULL;
    IsLogger = devIsFalseReturn;
    IsGPSSource = devIsFalseReturn;
    IsBaroSource = devIsFalseReturn;
    IsRadio = devIsFalseReturn;

    PutVoice = NULL;
    PortNumber = i;
    PutQNH = NULL;
    OnSysTicker = NULL;

    pDevPipeTo = NULL;
    PutVolume = NULL;
    PutFreqActive = NULL;
    PutFreqStandby = NULL;
    Disabled = true;
}

bool devNameCompare(const DeviceRegister_t& dev, const TCHAR *DeviceName) {
    return (_tcscmp(dev.Name, DeviceName) == 0);
}

bool ReadPortSettings(int idx, LPTSTR szPort, unsigned *SpeedIndex, BitIndex_t *Bit1Index) {
    switch (idx) {
        case 0:
            ReadPort1Settings(szPort, SpeedIndex, Bit1Index);
            return true;
        case 1:
            ReadPort2Settings(szPort, SpeedIndex, Bit1Index);
            return true;
        default:
            return false;
    }
}

void RestartCommPorts() {

    StartupStore(TEXT(". RestartCommPorts begin @%s%s"), WhatTimeIsIt(), NEWLINE);

    LockComm();
    /* 29/10/2013 : 
     * if RxThread wait for LockComm, It never can terminate -> Dead Lock
     *  can appen at many time when reset comport is called ....
     *    devRequestFlarmVersion called by NMEAParser::PFLAU is first exemple
     * 
     * in fact if it appens, devClose() kill RxThread after 20s timeout...
     *  that solve the deadlock, but thread is not terminated correctly ...
     * 
     * Bruno.
     */
    devClose(devA());
    devClose(devB());

    NMEAParser::Reset();

    devInit(TEXT(""));

    UnlockComm();
#if TESTBENCH
    StartupStore(TEXT(". RestartCommPorts end @%s%s"), WhatTimeIsIt(), NEWLINE);
#endif

}

BOOL devInit(LPCTSTR CommandLine) {
    TCHAR DeviceName[DEVNAMESIZE + 1];
    PDeviceDescriptor_t pDevNmeaOut = NULL;

    TCHAR Port[MAX_PATH] = {_T('\0')};
    unsigned SpeedIndex = 2U;
    BitIndex_t BitIndex = bit8N1;

    static bool doinit = true;

    pDevPrimaryBaroSource = NULL;
    pDevSecondaryBaroSource = NULL;

    std::set<std::tstring> UsedPort; // list of already used port

    for (unsigned i = 0; i < NUMDEV; i++) {
        DeviceList[i].InitStruct(i);

        ComPortStatus[i] = CPS_UNUSED; // 100210
        ComPortHB[i] = 0; // counter
        if (doinit) {
            ComPortRx[i] = 0;
            ComPortTx[i] = 0;
            ComPortErrTx[i] = 0;
            ComPortErrRx[i] = 0;
            ComPortErrors[i] = 0;

            doinit = false;
        }

        if (SIMMODE) {
            continue;
        }

        ReadDeviceSettings(i, DeviceName);
        DeviceList[i].Disabled = (_tcscmp(DeviceName, _T(DEV_DISABLED_NAME)) == 0);
        if (DeviceList[i].Disabled) {
            StartupStore(_T(". Device %c is DISABLED.%s"), (_T('A') + i), NEWLINE);
            continue;
        }

        DeviceRegister_t* pDev = std::find_if(&DeviceRegister[0], &DeviceRegister[DeviceRegisterCount], std::bind(&devNameCompare, _1, DeviceName));
        if (pDev == &DeviceRegister[DeviceRegisterCount]) {
            DeviceList[i].Disabled = true;
            StartupStore(_T(". Device %c : invalide drivers name <%s>%s"), (_T('A') + i), DeviceName, NEWLINE);
            continue;
        }


        if (_tcscmp(pDev->Name, TEXT("Internal")) == 0) {
            _tcscpy(Port, _T("GPSID"));
        } else {
            Port[0] = _T('\0');
            SpeedIndex = 2U;
            BitIndex = bit8N1;
            ReadPortSettings(i, Port, &SpeedIndex, &BitIndex);
        }
        // remember: Port1 is the port used by device A, port1 may be Com3 or Com1 etc

        if (std::find(UsedPort.begin(), UsedPort.end(), Port) != UsedPort.end()) {
            StartupStore(_T(". Port <%s> Already used, Device %c Disabled ! %s"), Port, (_T('A') + i), NEWLINE);


            continue;
        }
        UsedPort.insert(Port);

        // remember: Port1 is the port used by device A, port1 may be Com3 or Com1 etc
        StartupStore(_T(". Device %c is <%s> Port=%s%s"), (_T('A') + i), DeviceName, Port, NEWLINE);

        ComPort *Com = NULL;
        if (_tcsncmp(Port, _T("BT:"), 3) == 0) {
#ifdef NO_BLUETOOTH
            bool bStartOk = true;
#else
            bool bStartOk = false;
            CBtHandler* pBtHandler = CBtHandler::Get();
            StartupStore(_T(".. Initialise Bluetooth Device %s%s"), Port, NEWLINE);
            if (pBtHandler && pBtHandler->IsOk()) {
                if (pBtHandler->StartHW()) {
                    bStartOk = true;
                }
            }
#endif
            if (bStartOk) {
                Com = new BthPort(i, &Port[3]);
            }
        } else if (_tcscmp(Port, _T("GPSID")) == 0) {
            Com = new GpsIdPort(i, Port);
        } else {
            Com = new SerialPort(i, Port, dwSpeed[SpeedIndex], BitIndex, PollingMode);
        }

        if (Com && Com->Initialize()) {
            ComPortStatus[i] = CPS_OPENOK;
            pDev->Installer(&DeviceList[i]);

            if ((pDevNmeaOut == NULL) && (pDev->Flags & (1l << dfNmeaOut))) {
                pDevNmeaOut = &DeviceList[i];
            }

            DeviceList[i].Com = Com;

            devInit(&DeviceList[i]);
            devOpen(&DeviceList[i], i);

            if (devIsBaroSource(&DeviceList[i])) {
                if (pDevPrimaryBaroSource == NULL) {
                    pDevPrimaryBaroSource = &DeviceList[i];
                } else if (pDevSecondaryBaroSource == NULL) {
                    pDevSecondaryBaroSource = &DeviceList[i];
                }
            }
        } else {
            delete Com;
            ComPortStatus[i] = CPS_OPENKO;
        }
    }

    if (pDevNmeaOut != NULL) {
        if (pDevNmeaOut == devA()) {
            devB()->pDevPipeTo = devA();
        }
        if (pDevNmeaOut == devB()) {
            devA()->pDevPipeTo = devB();
        }
    }

    return (TRUE);
}

BOOL devCloseAll(void) {
    int i;

    for (i = 0; i < NUMDEV; i++) {
        devClose(&DeviceList[i]);
        ComPortStatus[i] = CPS_CLOSED; // 100210
    }
    return (TRUE);
}

PDeviceDescriptor_t devGetDeviceOnPort(int Port) {

    int i;

    for (i = 0; i < NUMDEV; i++) {
        if (DeviceList[i].Port == Port)
            return (&DeviceList[i]);
    }
    return (NULL);
}

// devParseStream(devIdx, c, &GPS_INFO);

BOOL devParseStream(int portNum, char* stream, int length, NMEA_INFO *pGPS) {

    PDeviceDescriptor_t d;
    d = devGetDeviceOnPort(portNum);
    if (d->ParseStream != NULL) {
        //   StartupStore(_T(". devParseStream %s"), NEWLINE);
        if (portNum >= 0 && portNum <= 1) {
            ComPortHB[portNum] = LKHearthBeats;
        }
        if (d->ParseStream(d, stream, length, pGPS)) {
            //GPSCONNECT  = TRUE; // NO! 121126

        }
        return (TRUE);
    } else
        return (FALSE);

}



// Called from Port task, after assembly of a string from serial port, ending with a LF

BOOL devParseNMEA(int portNum, TCHAR *String, NMEA_INFO *pGPS) {
    PDeviceDescriptor_t d;
    d = devGetDeviceOnPort(portNum);

    LogNMEA(String, portNum); // We must manage EnableLogNMEA internally from LogNMEA

    if (portNum >= 0 && portNum <= 1) {
        ComPortHB[portNum] = LKHearthBeats;
    }

    // intercept device specific parser routines 
    if (d != NULL) {
        if (d->pDevPipeTo && d->pDevPipeTo->Com) {
            // stream pipe, pass nmea to other device (NmeaOut)
            // TODO code: check TX buffer usage and skip it if buffer is full (outbaudrate < inbaudrate)
            d->pDevPipeTo->Com->WriteString(String);
        }

        if (d->ParseNMEA != NULL)
            if ((d->ParseNMEA)(d, String, pGPS)) {
                //GPSCONNECT  = TRUE; // NO! 121126
                return (TRUE);
            }
    }

    if (String[0] == '$') { // Additional "if" to find GPS strings
        if (NMEAParser::ParseNMEAString(portNum, String, pGPS)) {
            //GPSCONNECT  = TRUE; // NO! 121126
            return (TRUE);
        }
    }
    return (FALSE);

}

BOOL devDirectLink(PDeviceDescriptor_t d, BOOL bLinkEnable) {
    BOOL result = TRUE;

    if (SIMMODE)
        return TRUE;

    if (d != NULL && d->DirectLink != NULL)
        result = d->DirectLink(d, bLinkEnable);


    return result;
}

BOOL devPutMacCready(PDeviceDescriptor_t d, double MacCready) {
    BOOL result = TRUE;

    if (SIMMODE)
        return TRUE;
    LockComm();
    if (d != NULL && d->PutMacCready != NULL)
        result = d->PutMacCready(d, MacCready);
    UnlockComm();

    return result;
}

BOOL devRequestFlarmVersion(PDeviceDescriptor_t d) {
#if FLARMDEADLOCK
    if (SIMMODE)
        return TRUE;

    if (GPS_INFO.FLARM_Available) {
        if (d != NULL) {
            if (!d->Disabled) {
                devWriteNMEAString(d, _T("PFLAV,R"));
                return TRUE;
            }
        }
    }
#endif
    return FALSE;
}

BOOL devPutBugs(PDeviceDescriptor_t d, double Bugs) {
    BOOL result = TRUE;

    if (SIMMODE)
        return TRUE;
    LockComm();
    if (d != NULL && d->PutBugs != NULL)
        result = d->PutBugs(d, Bugs);
    UnlockComm();

    return result;
}

BOOL devPutBallast(PDeviceDescriptor_t d, double Ballast) {
    BOOL result = TRUE;

    if (SIMMODE)
        return TRUE;
    LockComm();
    if (d != NULL && d->PutBallast != NULL)
        result = d->PutBallast(d, Ballast);
    UnlockComm();

    return result;
}

// Only called from devInit() above which
// is in turn called with LockComm

BOOL devOpen(PDeviceDescriptor_t d, int Port) {
    BOOL res = TRUE;

    if (d != NULL && d->Open != NULL)
        res = d->Open(d, Port);

    if (res == TRUE) {
        d->Port = Port;
#ifdef RADIO_ACTIVE
        if (devIsRadio(d))
            RadioPara.Enabled = true;
#endif        
    }
    return res;
}

// Tear down methods should always succeed.
// Called from devInit() above under LockComm
// Also called when shutting down via devCloseAll()

BOOL devClose(PDeviceDescriptor_t d) {
    if (d != NULL) {
        if (d->Close != NULL) {
            d->Close(d);
        }

        ComPort *Com = d->Com;
        if (Com) {
            Com->Close();
            d->Com = NULL; // if we do that before Stop RXThread , Crash ....
            delete Com;
        }
    }

    return TRUE;
}

// Only called from devInit() above which
// is in turn called with LockComm

BOOL devInit(PDeviceDescriptor_t d) {
    if (d != NULL && d->Init != NULL)
        return ((d->Init)(d));
    else
        return (TRUE);
}

BOOL devLinkTimeout(PDeviceDescriptor_t d) {
    BOOL result = FALSE;

    if (SIMMODE)
        return TRUE;
    LockComm();
    if (d == NULL) {
        for (int i = 0; i < NUMDEV; i++) {
            d = &DeviceList[i];
            if (d->LinkTimeout != NULL)
                (d->LinkTimeout)(d);
        }
        result = TRUE;
    } else {
        if (d->LinkTimeout != NULL)
            result = d->LinkTimeout(d);
    }
    UnlockComm();

    return result;
}

BOOL devDeclare(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[]) {
    BOOL result = FALSE;

    if (SIMMODE)
        return TRUE;

    const unsigned BUFF_LEN = 128;
    TCHAR buffer[BUFF_LEN];

    // We must be sure we are not going to attempt task declaration
    // while a port reset is already in progress. If this happens, a Flarm device will not be Flarm anymore
    // until a Flarm nmea sentence is parsed again once. 

    // LKTOKEN  _@M1400_ = "Task declaration"
    // LKTOKEN  _@M571_ = "START"
    _sntprintf(buffer, BUFF_LEN, _T("%s: %s..."), gettext(_T("_@M1400_")), gettext(_T("_@M571_")));
    CreateProgressDialog(buffer);

    /***********************************************************/
    devDirectLink(d, true);
    /***********************************************************/
    LockComm();

    if ((d != NULL) && (d->Declare != NULL))
        result = d->Declare(d, decl, errBufferLen, errBuffer);
    else {
        if ((d != NULL) && NMEAParser::PortIsFlarm(d->Port)) {
            result |= FlarmDeclare(d, decl, errBufferLen, errBuffer);
        }
    }


    UnlockComm();
    /***********************************************************/
    devDirectLink(d, false);
    /***********************************************************/
    CloseProgressDialog();

    return result;
}

BOOL devIsLogger(PDeviceDescriptor_t d) {
    bool result = false;

    LockComm();
    if ((d != NULL) && (d->IsLogger != NULL)) {
        if (d->IsLogger(d)) {
            result = true;
        }
    }
    if ((d != NULL) && !result) {
        result |= NMEAParser::PortIsFlarm(d->Port);
    }
    UnlockComm();

    return result;
}

BOOL devIsGPSSource(PDeviceDescriptor_t d) {
    BOOL result = FALSE;

    LockComm();
    if ((d != NULL) && (d->IsGPSSource != NULL))
        result = d->IsGPSSource(d);
    UnlockComm();

    return result;
}

BOOL devIsBaroSource(PDeviceDescriptor_t d) {
    BOOL result = FALSE;

    LockComm();
    if ((d != NULL) && (d->IsBaroSource != NULL))
        result = d->IsBaroSource(d);
    UnlockComm();

    return result;
}

BOOL devIsRadio(PDeviceDescriptor_t d) {
    BOOL result = FALSE;

    LockComm();
    if ((d != NULL) && (d->IsRadio != NULL))
        result = d->IsRadio(d);
    UnlockComm();

    return result;
}

BOOL devPutQNH(DeviceDescriptor_t *d, double NewQNH) {
    BOOL result = FALSE;

    LockComm();
    if (d == NULL) {
        for (int i = 0; i < NUMDEV; i++) {
            d = &DeviceList[i];
            if (d->PutQNH != NULL)
                d->PutQNH(d, NewQNH);
        }
        result = TRUE;
    } else {
        if (d->PutQNH != NULL)
            result = d->PutQNH(d, NewQNH);
    }
    UnlockComm();

    return result;
}

BOOL devOnSysTicker(DeviceDescriptor_t *d) {
    BOOL result = FALSE;

    LockComm();
    if (d == NULL) {
        for (int i = 0; i < NUMDEV; i++) {
            d = &DeviceList[i];
            if (d->OnSysTicker != NULL)
                d->OnSysTicker(d);
        }
        result = TRUE;
    } else {
        if (d->OnSysTicker != NULL)
            result = d->OnSysTicker(d);

    }

    UnlockComm();

    return result;
}

static void devFormatNMEAString(TCHAR *dst, size_t sz, const TCHAR *text) {
    BYTE chk;
    int i, len = _tcslen(text);

    LKASSERT(text != NULL);
    LKASSERT(dst != NULL);

    for (chk = i = 0; i < len; i++)
        chk ^= (BYTE) text[i];

    _sntprintf(dst, sz, TEXT("$%s*%02X\r\n"), text, chk);
}

//
// NOTICE V5: this function is used only by LXMiniMap device driver .
// The problem is that it is locking Comm from RXThread and this is 
// creating a possible deadlock situation.
//

void devWriteNMEAString(PDeviceDescriptor_t d, const TCHAR *text) {
    if (d != NULL) {
        if (!d->Disabled) {
            TCHAR tmp[512];
            devFormatNMEAString(tmp, 512, text);

            devDirectLink(d, true);
            LockComm();
            if (d->Com)
                d->Com->WriteString(tmp);
            UnlockComm();
            devDirectLink(d, false);
        }
    }
}


#ifdef RADIO_ACTIVE

bool devDriverActivated(const TCHAR *DeviceName) {
    if ((_tcscmp(dwDeviceName1, DeviceName) == 0) || (_tcscmp(dwDeviceName2, DeviceName) == 0)) {
        if (_tcscmp(szPort1, szPort2) == 0) {
            return true;
        }
    }
    return false;
}


BOOL devPutVolume(PDeviceDescriptor_t d, int Volume) {
    BOOL result = TRUE;

    if (SIMMODE)
        return TRUE;
    LockComm();


    if (d != NULL) {
        if (!d->Disabled)
            if (d->Com) {
                if (d->PutVolume != NULL)
                    result = d->PutVolume(d, Volume);

                if (devDriverActivated(TEXT("PVCOM")))
                    PVCOMPutVolume(d, Volume);
            }
    }

    UnlockComm();

    return result;
}

BOOL devPutSquelch(PDeviceDescriptor_t d, int Squelch) {
    BOOL result = TRUE;

    if (SIMMODE)
        return TRUE;
    LockComm();
    if (d != NULL) {
        if (!d->Disabled)
            if (d->Com) {
                if (d->PutSquelch != NULL)
                    result = d->PutSquelch(d, Squelch);

                if (devDriverActivated(TEXT("PVCOM")))
                    PVCOMPutSquelch(d, Squelch);
            }
    }
    UnlockComm();

    return result;
}

BOOL devPutRadioMode(PDeviceDescriptor_t d, int mode) {
    BOOL result = TRUE;

    LockComm();
    if (d != NULL) {
        if (!d->Disabled)
            if (d->Com) {
                if (d->RadioMode != NULL)
                    result = d->RadioMode(d, mode);

                if (devDriverActivated(TEXT("PVCOM")))
                    PVCOMRadioMode(d, mode);
            }
    }
    UnlockComm();
    return result;
}

BOOL devPutFreqSwap(PDeviceDescriptor_t d) {
    BOOL result = TRUE;

    LockComm();

    if (d != NULL) {
        if (!d->Disabled)
            if (d->Com) {
                if (d->StationSwap != NULL)
                    result = d->StationSwap(d);
                if (devDriverActivated(TEXT("PVCOM")))
                    PVCOMStationSwap(d);
            }
    }
    UnlockComm();
    return result;
}

BOOL devPutFreqActive(PDeviceDescriptor_t d, double Freq, TCHAR StationName[]) {
    BOOL result = TRUE;

    if (SIMMODE)
        return TRUE;

    LockComm();

    if (d != NULL) {
        if (!d->Disabled) {
            if (d->Com) {
                if (d->PutFreqActive != NULL)
                    result = d->PutFreqActive(d, Freq, StationName);

                if (devDriverActivated(TEXT("PVCOM")))
                    PVCOMPutFreqActive(d, Freq, StationName);
            }
        }
    }

    UnlockComm();

    return result;
}

BOOL devPutFreqStandby(PDeviceDescriptor_t d, double Freq, TCHAR StationName[]) {
    BOOL result = TRUE;

    if (SIMMODE)
        return TRUE;
    LockComm();
    if (d != NULL) {
        if (!d->Disabled)
            if (d->Com) {
                if (d->PutFreqStandby != NULL)
                    result = d->PutFreqStandby(d, Freq, StationName);

                if (devDriverActivated(TEXT("PVCOM")))
                    PVCOMPutFreqStandby(d, Freq, StationName);
            }
    }
    UnlockComm();

    return result;
}
#endif  // RADIO_ACTIVE        


static BOOL
FlarmDeclareSetGet(PDeviceDescriptor_t d, TCHAR *Buffer) {
    //devWriteNMEAString(d, Buffer);

    TCHAR tmp[512];

    _sntprintf(tmp, 512, TEXT("$%s\r\n"), Buffer);

    if (d->Com)
        d->Com->WriteString(tmp);

    Buffer[6] = _T('A');
    for (int i = 0; i < 20; i++) /* try to get expected answer max 20 times*/ {
        if (ExpectString(d, Buffer))
            return true;
        Poco::Thread::sleep(20);
    }

    return false;

};

BOOL FlarmDeclare(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[]) {
    BOOL result = TRUE;
#define BUFF_LEN 512
    TCHAR Buffer[BUFF_LEN];
    for (int i = 0; i < 3; i++) {
        d->Com->StopRxThread();
        d->Com->SetRxTimeout(100); // set RX timeout to 50[ms]


        _stprintf(Buffer, TEXT("PFLAC,S,PILOT,%s"), decl->PilotName);
        if (!FlarmDeclareSetGet(d, Buffer)) result = FALSE;

        _stprintf(Buffer, TEXT("PFLAC,S,GLIDERID,%s"), decl->AircraftRego);
        if (result) if (!FlarmDeclareSetGet(d, Buffer)) result = FALSE;

        _stprintf(Buffer, TEXT("PFLAC,S,GLIDERTYPE,%s"), decl->AircraftType);
        if (result) if (!FlarmDeclareSetGet(d, Buffer)) result = FALSE;

        _stprintf(Buffer, TEXT("PFLAC,S,COMPID,%s"), decl->CompetitionID);
        if (result) if (!FlarmDeclareSetGet(d, Buffer)) result = FALSE;

        _stprintf(Buffer, TEXT("PFLAC,S,COMPCLASS,%s"), decl->CompetitionClass);
        if (result) if (!FlarmDeclareSetGet(d, Buffer)) result = FALSE;

        _stprintf(Buffer, TEXT("PFLAC,S,NEWTASK,Task"));
        if (result) if (!FlarmDeclareSetGet(d, Buffer)) result = FALSE;

        _stprintf(Buffer, TEXT("PFLAC,S,ADDWP,0000000N,00000000E,TAKEOFF"));
        if (result) if (!FlarmDeclareSetGet(d, Buffer)) result = FALSE;

        if (result == TRUE)
            for (int j = 0; j < decl->num_waypoints; j++) {
                int DegLat, DegLon;
                double MinLat, MinLon;
                char NoS, EoW;

                DegLat = (int) decl->waypoint[j]->Latitude;
                MinLat = decl->waypoint[j]->Latitude - DegLat;
                NoS = 'N';
                if ((MinLat < 0) || ((MinLat - DegLat == 0) && (DegLat < 0))) {
                    NoS = 'S';
                    DegLat *= -1;
                    MinLat *= -1;
                }
                MinLat *= 60;
                MinLat *= 1000;

                DegLon = (int) decl->waypoint[j]->Longitude;
                MinLon = decl->waypoint[j]->Longitude - DegLon;
                EoW = 'E';
                if ((MinLon < 0) || ((MinLon - DegLon == 0) && (DegLon < 0))) {
                    EoW = 'W';
                    DegLon *= -1;
                    MinLon *= -1;
                }
                MinLon *= 60;
                MinLon *= 1000;

                TCHAR shortname[6];
                _stprintf(shortname, _T("P%02d"), j);
                _stprintf(Buffer,
                        TEXT("PFLAC,S,ADDWP,%02d%05.0f%c,%03d%05.0f%c,%s"),
                        DegLat, MinLat, NoS, DegLon, MinLon, EoW,
                        shortname);
                if (!FlarmDeclareSetGet(d, Buffer)) result = FALSE;
            }

        _stprintf(Buffer, TEXT("PFLAC,S,ADDWP,0000000N,00000000E,LANDING"));
        if (result) if (!FlarmDeclareSetGet(d, Buffer)) result = FALSE;

        // Reboot flarm to make declaration active, according to specs

        Poco::Thread::sleep(100);

        devFormatNMEAString(Buffer, BUFF_LEN, TEXT("PFLAR,0"));
        if (result == TRUE)
            d->Com->WriteString(Buffer);
        Poco::Thread::sleep(100);


        d->Com->SetRxTimeout(RXTIMEOUT); // clear timeout
        d->Com->StartRxThread(); // restart RX thread
        if (result == TRUE)
            return result;
        Poco::Thread::sleep(100);
    }
    return false; // no success
}
