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
#include "TCPPort.h"
#include "devPVCOM.h"
#include <functional>
#ifdef __linux__
  #include <dirent.h>
  #include <unistd.h>

#ifdef KOBO
  #include "Kobo/System.hpp"
  #include "Kobo/Kernel.hpp"
#endif // KOBO

#endif // __linux__

#ifdef  ANDROID
#include "Java/Global.hxx"
#include "Java/String.hxx"
#include "Android/InternalPort.h"
#include "Android/BluetoothHelper.hpp"
#include <sstream>
#endif


#ifdef RADIO_ACTIVE
bool devDriverActivated(const TCHAR *DeviceName) ;
#endif    
    
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
#if USELKASSERT
class DeviceMutex : public Poco::Mutex {
public:
    inline void Lock() {
        Poco::Mutex::lock();
        // Check if we are not inside RXThread, otherwise, we can have deadlock when we stop RxThread.
        for(DeviceDescriptor_t device : DeviceList) {
            if(device.Com) {
                LKASSERT(!device.Com->IsCurrentThread());
            }
        }
    }
    
    inline void Unlock() { 
        Poco::Mutex::unlock(); 
    }    
};

class DeviceScopeLock : public Poco::ScopedLock<DeviceMutex> {
public:
    DeviceScopeLock(DeviceMutex& m) : Poco::ScopedLock<DeviceMutex>(m) { }

};
#else
typedef Mutex DeviceMutex;
typedef ScopeLock DeviceScopeLock;
#endif
static DeviceMutex  CritSec_Comm;        

COMMPort_t COMMPort;

static  const unsigned   dwSpeed[] = {1200,2400,4800,9600,19200,38400,57600,115200};

DeviceRegister_t   DeviceRegister[NUMREGDEV];
DeviceDescriptor_t DeviceList[NUMDEV];

DeviceDescriptor_t *pDevPrimaryBaroSource=NULL;
DeviceDescriptor_t *pDevSecondaryBaroSource=NULL;

int DeviceRegisterCount = 0;

void LockComm() {
  CritSec_Comm.Lock();
}

void UnlockComm() {
  CritSec_Comm.Unlock();
}


/**
 * Call DeviceDescriptor_t::*func on all connected device without Argument.
 * @return FALSE if error on one device.
 * 
 * TODO : report witch device failed (useless still return value are never used).
 */
BOOL for_all_device(BOOL (*(DeviceDescriptor_t::*func))(DeviceDescriptor_t* d)) {
    if (SIMMODE) {
      return TRUE;
    }
    unsigned nbDeviceFailed = 0;

    DeviceScopeLock Lock(CritSec_Comm);
    for( DeviceDescriptor_t& d : DeviceList) {
        if( !d.Disabled && d.Com && (d.*func) ) {
          nbDeviceFailed +=  (d.*func)(&d) ? 0 : 1;
      }

    }
    return (nbDeviceFailed > 0);
}

/**
 * Call DeviceDescriptor_t::*func on all connected device with one Argument.
 * @return FALSE if error on one device.
 * 
 * TODO : report witch device failed (useless still return value are never used).
 */
template<typename _Arg1>
BOOL for_all_device(BOOL (*(DeviceDescriptor_t::*func))(DeviceDescriptor_t* d, _Arg1), _Arg1 Val1) {
    if (SIMMODE) {
      return TRUE;
    }
    unsigned nbDeviceFailed = 0;

    DeviceScopeLock Lock(CritSec_Comm);
    for( DeviceDescriptor_t& d : DeviceList) {
        if( !d.Disabled && d.Com && (d.*func) ) {
          nbDeviceFailed +=  (d.*func)(&d, Val1) ? 0 : 1;
      }
    }
    return (nbDeviceFailed > 0);
}
/**
 * Call DeviceDescriptor_t::*func on all connected device with two Argument.
 * @return FALSE if error on one device.
 * 
 * TODO : report witch device failed (useless still return value are never used).
 */
template<typename _Arg1, typename _Arg2>
BOOL for_all_device(BOOL (*(DeviceDescriptor_t::*func))(DeviceDescriptor_t* d, _Arg1, _Arg2), _Arg1 Val1, _Arg2 Val2) {
    if (SIMMODE) {
      return TRUE;
    }
    unsigned nbDeviceFailed = 0;

    DeviceScopeLock Lock(CritSec_Comm);
    for( DeviceDescriptor_t& d : DeviceList) {
        if( !d.Disabled && d.Com && (d.*func) ) {
          nbDeviceFailed +=  (d.*func)(&d, Val1, Val2) ? 0 : 1;
      }

    }
    return (nbDeviceFailed > 0);
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
BOOL devGetBaroAltitude(double *Value){
  // hack, just return GPS_INFO->BaroAltitude
  if (Value == NULL)
    return(FALSE);
  if (GPS_INFO.BaroAltitudeAvailable)
    *Value = GPS_INFO.BaroAltitude;
  return(TRUE);

}
#endif

BOOL ExpectString(PDeviceDescriptor_t d, const TCHAR *token){

  int i=0, ch;

  if (!d->Com)
    return FALSE;

  while ((ch = d->Com->GetChar()) != EOF){

    if (token[i] == (TCHAR)ch) 
      i++;
    else
      i=0;

    if ((unsigned)i == _tcslen(token))
      return(TRUE);

  }

  return(FALSE);

}


BOOL devRegister(const TCHAR *Name, int Flags, 
                 BOOL (*Installer)(PDeviceDescriptor_t d)) {
  if (DeviceRegisterCount >= NUMREGDEV) {
    LKASSERT(FALSE);
    return(FALSE);
  }
  DeviceRegister[DeviceRegisterCount].Name = Name;
  DeviceRegister[DeviceRegisterCount].Flags = Flags;
  DeviceRegister[DeviceRegisterCount].Installer = Installer;
  DeviceRegisterCount++;
  return(TRUE);
}

LPCTSTR devRegisterGetName(int Index){
  if (Index < 0 || Index >= DeviceRegisterCount) {
    return _T("");
  }
  return DeviceRegister[Index].Name;
}

// This device is not available if Disabled
// Index 0 or 1 
bool devIsDisabled(int Index) {
  if (Index < 0 || Index >1)
	return (true);
	
  return DeviceList[Index].Disabled;
}

static BOOL devIsFalseReturn(PDeviceDescriptor_t d){
  (void)d;
  return(FALSE);

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
    n = scandir("/dev", &namelist, 0, alphasort);//need test
  } else {  
    n = scandir("/sys/class/tty", &namelist, 0, alphasort); //which is faster than /dev/
  }
  if (n != -1){
    for (int i = 0; i < n; ++i) {
      bool portok = true;
      if (memcmp(namelist[i]->d_name, "tty", 3) == 0) {
        // filter out "/dev/tty0", ... (valid integer after "tty") 
        char *endptr;
        strtoul(namelist[i]->d_name + 3, &endptr, 10);
        if (*endptr == 0) {
          portok = false;
        }
      } else if ( (memcmp(namelist[i]->d_name, "rfcomm", 6) != 0) 
                  && (memcmp(namelist[i]->d_name, "tnt", 3) != 0) ){
        // '/dev/tntX' are tty0tty virtual port  
        portok = false;
      }
      if(portok) {
        char path[64];
        snprintf(path, sizeof(path), "/dev/%s", namelist[i]->d_name);
        if (access(path, R_OK|W_OK) == 0 && access(path, X_OK) < 0) {
          COMMPort.push_back(path);
        }
      }
      free(namelist[i]);
    } 
    free(namelist);
  }    
  
  // scan usb serial by id
  n = scandir("/dev/serial/by-id", &namelist, 0, alphasort);
  if(n != -1) {
    for (int i = 0; i < n; ++i) {
      char path[1024];
      snprintf(path, sizeof(path), "/dev/serial/by-id/%s", namelist[i]->d_name);
      if (access(path, R_OK|W_OK) == 0 && access(path, X_OK) < 0) {
        snprintf(path, sizeof(path), "id:%s", namelist[i]->d_name);
        COMMPort.push_back(path);
      }
      free(namelist[i]);
    }      
    free(namelist);
  }

#ifdef KOBO
  if(KoboExportSerialAvailable() && !IsKoboOTGKernel()) {
    if(std::find_if(COMMPort.begin(), COMMPort.end(), std::bind(&COMMPortItem_t::IsSamePort, _1, _T("/dev/ttyGS0"))) == COMMPort.end()) {
      COMMPort.push_back(_T("/dev/ttyGS0"));
    }
  }
#elif TESTBENCH
  if(lk::filesystem::exist(_T("/lk"))) {
    COMMPort.push_back(_T("/lk/ptycom1"));
    COMMPort.push_back(_T("/lk/ptycom2"));
    COMMPort.push_back(_T("/lk/ptycom3"));
    COMMPort.push_back(_T("/lk/ptycom4"));
  }
#endif

#endif

#ifndef NO_BLUETOOTH
    CBtHandler* pBtHandler = CBtHandler::Get();
    if (pBtHandler) {
        std::copy(
        	pBtHandler->m_devices.begin(),
        	pBtHandler->m_devices.end(),
        	std::back_insert_iterator<COMMPort_t>(COMMPort)
        );
    }
#endif

    COMMPort.push_back(_T("TCPClient"));
    COMMPort.push_back(_T("TCPServer"));
    COMMPort.push_back(_T("UDPServer"));

#ifdef ANDROID
    JNIEnv *env = Java::GetEnv();
    if(env) {
        Java::LocalRef<jobjectArray> bonded(env, BluetoothHelper::list(env));
        if (bonded) {

            jsize nBT = env->GetArrayLength(bonded) / 2;
            for (jsize i = 0; i < nBT; ++i) {
                Java::String j_address(env, (jstring) env->GetObjectArrayElement(bonded, i * 2));
                if(!j_address)
                    continue;

                const std::string address = j_address.ToString();
                if (address.empty())
                    continue;

                Java::String j_name(env, (jstring) env->GetObjectArrayElement(bonded, i * 2 + 1));
                std::stringstream prefixed_address, name;

                prefixed_address << "BT:" << address;
                name << "BT:" << ( j_name ? j_name.ToString() : std::string() );

                COMMPort.push_back(COMMPortItem_t(prefixed_address.str(), name.str()));
            }
        }
    }
#endif

    if(COMMPort.empty()) {
        // avoid segfault on device config  dialog if no comport detected.
        COMMPort.push_back(_T("Null"));
    }
}

void DeviceDescriptor_t::InitStruct(int i) {
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

    PutVolume = NULL;
    PutFreqActive = NULL;
    PutFreqStandby = NULL;
    Disabled = true;

    Status = CPS_UNUSED; // 100210
    HB = 0; // counter

    nmeaParser._Reset();
    iSharedPort = -1;
    bNMEAOut     = false;
    static bool doinit = true;
    if (doinit) {
        Rx = 0;
        Tx = 0;
        ErrTx = 0;
        ErrRx = 0;

        doinit = false;
    }
}

bool devNameCompare(const DeviceRegister_t& dev, const TCHAR *DeviceName) {
    return (_tcscmp(dev.Name, DeviceName) == 0);
}


bool GetPortSettings(int idx, LPTSTR szPort, unsigned *SpeedIndex, BitIndex_t *Bit1Index) {
    if (idx >= 0)
      if (idx < NUMDEV)
      {
          ReadPortSettings(idx, szPort, SpeedIndex, Bit1Index);
        return true;
      }
    return false;

}

void RestartCommPorts() {

    StartupStore(TEXT(". RestartCommPorts begin @%s%s"), WhatTimeIsIt(), NEWLINE);

    devCloseAll();

    devInit();

#if TESTBENCH
    StartupStore(TEXT(". RestartCommPorts end @%s%s"), WhatTimeIsIt(), NEWLINE);
#endif

}

// Only called from devInit() above which
// is in turn called with LockComm
static BOOL devOpen(PDeviceDescriptor_t d){
  if (d && d->Open) {
    return d->Open(d);
  }
  return TRUE;
}

// Only called from devInit() above which
// is in turn called with LockComm
static BOOL devInit(PDeviceDescriptor_t d){
  if (d != NULL && d->Init != NULL)
    return ((d->Init)(d));
  else
    return(TRUE);
}

BOOL devInit() {
    LockComm();

    TCHAR DeviceName[DEVNAMESIZE + 1];


    TCHAR Port[MAX_PATH] = {_T('\0')};
    unsigned SpeedIndex = 2U;
    BitIndex_t BitIndex = bit8N1;
#ifdef RADIO_ACTIVE    
     RadioPara.Enabled = false;
     if(SIMMODE)
       RadioPara.Enabled = true;
#endif     

    pDevPrimaryBaroSource = NULL;
    pDevSecondaryBaroSource = NULL;

    std::set<tstring> UsedPort; // list of already used port
    
    for (unsigned i = 0; i < NUMDEV; i++) {
        DeviceList[i].InitStruct(i);

        if (SIMMODE){
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
        if(_tcscmp(pDev->Name,TEXT("Internal")) == 0) {
            _tcscpy(Port, _T("internal"));
        } else { 
            Port[0] = _T('\0');
            SpeedIndex = 2U;
            BitIndex = bit8N1;
            ReadPortSettings(i, Port, &SpeedIndex, &BitIndex);
        }
        // remember: Port1 is the port used by device A, port1 may be Com3 or Com1 etc

        if(std::find(UsedPort.begin(), UsedPort.end(), Port) != UsedPort.end()) {
            unsigned int j;
            for( j = 0; j < i ; j++)
            {
              if(!DeviceList[i].Disabled)
              if( (_tcscmp(szPort[i] , szPort[j])==0)  &&  DeviceList[j].iSharedPort <0)
              {
                devInit(&DeviceList[i]);
                DeviceList[i].iSharedPort =j;
                StartupStore(_T(". Port <%s> Already used, Device %c shares it with %c ! %s"), Port, (_T('A') + i),(_T('A') + j), NEWLINE);
                DeviceList[i].Com = DeviceList[j].Com ;
                DeviceList[i].Status = CPS_OPENOK;
                pDev->Installer(&DeviceList[i]);
                if (pDev->Flags & (1l << dfNmeaOut)) {
                    DeviceList[i].bNMEAOut = true;
                }
                if(devIsRadio(&DeviceList[i]))
                {
                  RadioPara.Enabled = true;
                  StartupStore(_T(".  RADIO  %c  over  <%s>%s"), (_T('A') + i),  Port, NEWLINE);
                }
              }
            }
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
            if(bStartOk) {
                Com = new BthPort(i, &Port[3]);
            }
        } else if (_tcscmp(Port, _T("internal")) == 0) {
#ifdef ANDROID
            Com = new InternalPort(i, Port);
#else
            Com = new GpsIdPort(i, Port);
#endif
        } else if (_tcscmp(Port, _T("TCPClient")) == 0) {
#ifdef KOBO
            if(!IsKoboWifiOn()) {
              KoboWifiOn();
            }
#endif
            Com = new TCPClientPort(i, Port);
        } else if (_tcscmp(Port, _T("TCPServer")) == 0) {
#ifdef KOBO
            if(!IsKoboWifiOn()) {
              KoboWifiOn();
            }
#endif
            Com = new TCPServerPort(i, Port);
        } else if (_tcscmp(Port, _T("UDPServer")) == 0) {
#ifdef KOBO
            if(!IsKoboWifiOn()) {
              KoboWifiOn();
            }
#endif
            Com = new UDPServerPort(i, Port);

        } else {
            Com = new SerialPort(i, Port, dwSpeed[SpeedIndex], BitIndex, PollingMode);
        }

        if (Com && Com->Initialize()) {
            /*
             * Need to be done before anny #DeviceDescriptor_t::Callback call.
             */
            DeviceList[i].Com = Com;
            DeviceList[i].Status = CPS_OPENOK;
            pDev->Installer(&DeviceList[i]);

            if (pDev->Flags & (1l << dfNmeaOut)) {
                DeviceList[i].bNMEAOut = true;
            }

            devInit(&DeviceList[i]);
            devOpen(&DeviceList[i]);

            if (devIsBaroSource(&DeviceList[i])) {
                if (pDevPrimaryBaroSource == NULL) {
                    pDevPrimaryBaroSource = &DeviceList[i];
                } else if (pDevSecondaryBaroSource == NULL) {
                    pDevSecondaryBaroSource = &DeviceList[i];
                }
            }

            Com->StartRxThread();
        } else {
            delete Com;
            DeviceList[i].Status = CPS_OPENKO;
        }
#ifdef RADIO_ACTIVE    
       if(devIsRadio(&DeviceList[i]))
       {       
          RadioPara.Enabled = true;         
          StartupStore(_T(".  RADIO  %c  over  <%s>%s"), (_T('A') + i),  Port, NEWLINE);
       }
       if(devDriverActivated(TEXT("PVCOM")))
       {       
          RadioPara.Enabled = true;         
          StartupStore(_T(".  RADIO  %c  PVCOM over  shared <%s>%s"), (_T('A') + i),  Port, NEWLINE);
       }        
#endif          
    }


    UnlockComm();
    return (TRUE);
}

// Tear down methods should always succeed.
// Called from devInit() above under LockComm
// Also called when shutting down via devCloseAll()
static BOOL devClose(PDeviceDescriptor_t d)
{
  if (d != NULL) {
    if (d->Close != NULL) {
      d->Close(d);
    }
    
    ComPort *Com = d->Com;
    if (Com) {
      if(d->iSharedPort <0)  // don't close shared Ports,  these are only copies!
      {
        Com->Close();
        delete Com;
      }
      d->Com = NULL; // if we do that before Stop RXThread , Crash ....

    }    
  }

  return TRUE;
}

BOOL devCloseAll(void){
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
  LockComm();
  for (unsigned i=0; i<NUMDEV; i++){
    devClose(&DeviceList[i]);
    DeviceList[i].Status=CPS_CLOSED; // 100210
  }
  UnlockComm();
  
  return(TRUE);
}


PDeviceDescriptor_t devGetDeviceOnPort(int Port){

  if(Port >=0 && Port < array_size(DeviceList)) {
    return &DeviceList[Port];
  }
  return nullptr;
}

 // devParseStream(devIdx, c, &GPS_INFO);
BOOL devParseStream(int portNum, char* stream, int length, NMEA_INFO *pGPS){
bool  ret = FALSE;
PDeviceDescriptor_t din = devGetDeviceOnPort(portNum);
PDeviceDescriptor_t d = NULL;

    for(int dev =0; dev < NUMDEV; dev++)
    {
      d = &DeviceList[dev];
      if (din && d && d->ParseStream)
      {
       if((d->iSharedPort == portNum) ||  (d->PortNumber == portNum))
       {
         d->HB=LKHearthBeats;
         if (d->ParseStream(din, stream, length, pGPS)) {
          
         }
         ret =TRUE;
       }
     }
   }
  return(ret);
}    



// Called from Port task, after assembly of a string from serial port, ending with a LF
BOOL devParseNMEA(int portNum, TCHAR *String, NMEA_INFO *pGPS){
  bool  ret = FALSE;
  LogNMEA(String, portNum); // We must manage EnableLogNMEA internally from LogNMEA

  PDeviceDescriptor_t d = devGetDeviceOnPort(portNum);
  if(!d) {
    return FALSE;
  }
  
  d->HB=LKHearthBeats;

  // intercept device specific parser routines 
    for(DeviceDescriptor_t& d2 : DeviceList) {
        
      if((d2.iSharedPort == portNum) ||  (d2.PortNumber == portNum)) {
        if ( d2.ParseNMEA && d2.ParseNMEA(d, String, pGPS) ) {
          //GPSCONNECT  = TRUE; // NO! 121126
            ret = TRUE;
        } else if( &d2 == d) {
          // call ParseNMEAString_Internal only for master port if string are not device specific.
          if(String[0]=='$') {  // Additional "if" to find GPS strings
            if(d->nmeaParser.ParseNMEAString_Internal(String, pGPS)) {
              //GPSCONNECT  = TRUE; // NO! 121126
              ret = TRUE;
            }
          }
        }
      }
    }

    if(d->nmeaParser.activeGPS) {
        
      for(DeviceDescriptor_t& d2 : DeviceList) {
          
          if(!d2.Disabled && d2.bNMEAOut) { // NMEA out ! even on multiple ports    
            // stream pipe, pass nmea to other device (NmeaOut)
            d2.Com->WriteString(String); // TODO code: check TX buffer usage and skip it if buffer is full (outbaudrate < inbaudrate)
          }
      }
    }
  return(ret);
}

BOOL devDirectLink(PDeviceDescriptor_t d,	BOOL bLinkEnable)
{
  BOOL result = TRUE;

  if (SIMMODE)
	return TRUE;

  if (d != NULL && d->DirectLink != NULL)
	result = d->DirectLink(d, bLinkEnable);


  return result;
}

/**
 * Send MacCready to all connected device.
 * @param MacCready
 * @return FALSE if error on one device.
 */
BOOL devPutMacCready(double MacCready) {
    return for_all_device(&DeviceDescriptor_t::PutMacCready, MacCready);
}


BOOL devRequestFlarmVersion(PDeviceDescriptor_t d)
{
#if FLARMDEADLOCK
  if (SIMMODE)
    return TRUE;

  if(GPS_INFO.FLARM_Available)
  {
    if (d != NULL)
    {
      if(!d->Disabled)
      {
  	    devWriteNMEAString(d,_T("PFLAV,R"));
        return TRUE;
      }
    }
  }
#endif
  return FALSE;
}

/**
 * Send Bugs % to all connected device.
 * @param Bugs [0.0 - 1.0]
 * @return FALSE if error on one device.
 */
BOOL devPutBugs(double Bugs) {
    return for_all_device(&DeviceDescriptor_t::PutBugs, Bugs);
}

/**
 * Send Ballast % to all connected device.
 * @param Ballast [0.0 - 1.0]
 * @return FALSE if error on one device.
 */
BOOL devPutBallast(double Ballast) {
    return for_all_device(&DeviceDescriptor_t::PutBallast, Ballast);
}

BOOL devLinkTimeout(PDeviceDescriptor_t d)
{
  BOOL result = FALSE;

  if (SIMMODE)
    return TRUE;
  LockComm();
  if (d == NULL){
    for (int i=0; i<NUMDEV; i++){
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


BOOL devDeclare(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[])
{
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
  _sntprintf(buffer, BUFF_LEN, _T("%s: %s..."), MsgToken(1400), MsgToken(571));
  CreateProgressDialog(buffer);

  LockComm();
  /***********************************************************/
  devDirectLink(d,true);
  /***********************************************************/
  if ((d != NULL) && (d->Declare != NULL))
	result = d->Declare(d, decl, errBufferLen, errBuffer);
  else {
	if(d && d->nmeaParser.isFlarm) {
    	result |= FlarmDeclare(d, decl, errBufferLen, errBuffer);
  	}
  }
  /***********************************************************/
  devDirectLink(d,false);
  /***********************************************************/
  UnlockComm();
  
  CloseProgressDialog();
  
  return result;
}

BOOL devIsLogger(PDeviceDescriptor_t d)
{
  bool result = false;

  LockComm();
  if ((d != NULL) && (d->IsLogger != NULL)) {
    if (d->IsLogger(d)) {
      result = true;
    }
  }
  if ((d != NULL) && !result) {
    result |= d->nmeaParser.isFlarm;
  }
  UnlockComm();

  return result;
}

BOOL devIsGPSSource(PDeviceDescriptor_t d)
{
  BOOL result = FALSE;

  LockComm();
  if ((d != NULL) && (d->IsGPSSource != NULL))
    result = d->IsGPSSource(d);
  UnlockComm();

  return result;
}

BOOL devIsBaroSource(PDeviceDescriptor_t d)
{
  BOOL result = FALSE;

  LockComm();
  if ((d != NULL) && (d->IsBaroSource != NULL))
    result = d->IsBaroSource(d);
  UnlockComm();

  return result;
}

BOOL devIsRadio(PDeviceDescriptor_t d)
{
  BOOL result = FALSE;
  LockComm();
  if ((d != NULL) && (d->IsRadio != NULL))
    result = d->IsRadio(d);
  UnlockComm();

  return result;
}


BOOL devPutQNH(DeviceDescriptor_t *d, double NewQNH)
{
  BOOL result = FALSE;

  LockComm();
  if (d == NULL){
    for (int i=0; i<NUMDEV; i++){
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

BOOL devOnSysTicker(DeviceDescriptor_t *d)
{
  BOOL result = FALSE;

  LockComm();
  if (d == NULL){
    for (int i=0; i<NUMDEV; i++){
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

static void devFormatNMEAString(TCHAR *dst, size_t sz, const TCHAR *text)
{
  BYTE chk;
  int i, len = _tcslen(text);

  LKASSERT(text!=NULL);
  LKASSERT(dst!=NULL);

  for (chk = i = 0; i < len; i++)
    chk ^= (BYTE)text[i];

  _sntprintf(dst, sz, TEXT("$%s*%02X\r\n"), text, chk);
}

//
// NOTICE V5: this function is used only by LXMiniMap device driver .
// The problem is that it is locking Comm from RXThread and this is 
// creating a possible deadlock situation.
void devWriteNMEAString(PDeviceDescriptor_t d, const TCHAR *text)
{
  if(d != NULL)
  {
    if(!d->Disabled)
    {
	  TCHAR tmp[512];
      devFormatNMEAString(tmp, 512, text);

      LockComm();      
      devDirectLink(d,true);
      if (d->Com) {
        d->Com->WriteString(tmp);
      }
      devDirectLink(d,false);
      UnlockComm();
    }
  }
}


#ifdef RADIO_ACTIVE

bool devDriverActivated(const TCHAR *DeviceName) {
  for(int i=0; i <NUMDEV; i++)
    if ((_tcscmp(dwDeviceName[i], DeviceName) == 0)) {
            return true;        
    }
    return false;
}

/**
 * Send Volume Level to all connected device.
 * @param Volume [1 - 20]
 * @return FALSE if error on one device.
 */
BOOL devPutVolume(int Volume) {
  return for_all_device(&DeviceDescriptor_t::PutVolume, Volume);

}

/**
 * Send Squelch Level to all connected device.
 * @param Squelch [1 - 10]
 * @return FALSE if error on one device.
 */
BOOL devPutSquelch(int Squelch) {
  return for_all_device(&DeviceDescriptor_t::PutSquelch, Squelch);

}    

/**
 * Set RadioMode to all connected device.
 * @param mode 
 *      0 : Dual On
 *      1 : Dual Off
 * @return FALSE if error on one device.
 */
BOOL devPutRadioMode(int mode) {
  return for_all_device(&DeviceDescriptor_t::PutRadioMode, mode);
}

/**
 * Send FreqSwap cmd to all connected device.
 * @return FALSE if error on one device.
 */
BOOL devPutFreqSwap() {
  return for_all_device(&DeviceDescriptor_t::StationSwap);

}  



/**
 * Send FreqActive cmd to all connected device.
 * @return FALSE if error on one device.
 */
BOOL devPutFreqActive(double Freq, TCHAR StationName[]) {
  if (SIMMODE) {
    RadioPara.ActiveFrequency=  Freq;
    _stprintf( RadioPara.ActiveName, _T("%s") , StationName);
    return TRUE;
  }
  return for_all_device(&DeviceDescriptor_t::PutFreqActive, Freq, StationName);




}

/**
 * Send FreqStandby cmd to all connected device.
 * @return FALSE if error on one device.
 */
BOOL devPutFreqStandby(double Freq,TCHAR  StationName[]) {
  if (SIMMODE) {
     RadioPara.PassiveFrequency=  Freq;
     _stprintf( RadioPara.PassiveName, _T("%s") , StationName);
    return TRUE;
  }
  return for_all_device(&DeviceDescriptor_t::PutFreqStandby, Freq, StationName);

}
#endif  // RADIO_ACTIVE        


static BOOL 
FlarmDeclareSetGet(PDeviceDescriptor_t d, TCHAR *Buffer) {
  //devWriteNMEAString(d, Buffer);

  TCHAR tmp[512];

  _sntprintf(tmp, 512, TEXT("$%s\r\n"), Buffer);

  if (d->Com)
    d->Com->WriteString(tmp);

  Buffer[6]= _T('A');
  for(int i=0; i < 20; i++) /* try to get expected answer max 20 times*/
  {
    if (ExpectString(d, Buffer))
	  return true;
    Poco::Thread::sleep(20);
  }

  return false;

};


BOOL FlarmDeclare(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[])
{
  BOOL result = TRUE;
#define BUFF_LEN 512
  TCHAR Buffer[BUFF_LEN];
 for(int i=0; i < 3; i++)
 {
  d->Com->StopRxThread();
  d->Com->SetRxTimeout(100);                     // set RX timeout to 50[ms]


  _stprintf(Buffer,TEXT("PFLAC,S,PILOT,%s"),decl->PilotName);
  if (!FlarmDeclareSetGet(d,Buffer)) result = FALSE;

  _stprintf(Buffer,TEXT("PFLAC,S,GLIDERID,%s"),decl->AircraftRego);
  if(result) if (!FlarmDeclareSetGet(d,Buffer)) result = FALSE;

  _stprintf(Buffer,TEXT("PFLAC,S,GLIDERTYPE,%s"),decl->AircraftType);
  if(result) if (!FlarmDeclareSetGet(d,Buffer)) result = FALSE;
  
  _stprintf(Buffer,TEXT("PFLAC,S,COMPID,%s"),decl->CompetitionID);
  if(result) if (!FlarmDeclareSetGet(d,Buffer)) result = FALSE;
  
  _stprintf(Buffer,TEXT("PFLAC,S,COMPCLASS,%s"),decl->CompetitionClass);
  if(result) if (!FlarmDeclareSetGet(d,Buffer)) result = FALSE;
  
  _stprintf(Buffer,TEXT("PFLAC,S,NEWTASK,Task"));
  if(result) if (!FlarmDeclareSetGet(d,Buffer)) result = FALSE;

  _stprintf(Buffer,TEXT("PFLAC,S,ADDWP,0000000N,00000000E,TAKEOFF"));
  if(result) if (!FlarmDeclareSetGet(d,Buffer)) result = FALSE;

  if(result == TRUE)
    for (int j = 0; j < decl->num_waypoints; j++) {
      int DegLat, DegLon;
      double MinLat, MinLon;
      char NoS, EoW;

      DegLat = (int)decl->waypoint[j]->Latitude;
      MinLat = decl->waypoint[j]->Latitude - DegLat;
      NoS = 'N';
      if((MinLat<0) || ((MinLat-DegLat==0) && (DegLat<0)))
      {
	    NoS = 'S';
	    DegLat *= -1; MinLat *= -1;
      }
      MinLat *= 60;
      MinLat *= 1000;
    
      DegLon = (int)decl->waypoint[j]->Longitude;
      MinLon = decl->waypoint[j]->Longitude - DegLon;
      EoW = 'E';
      if((MinLon<0) || ((MinLon-DegLon==0) && (DegLon<0)))
      {
	    EoW = 'W';
	    DegLon *= -1; MinLon *= -1;
      }
      MinLon *=60;
      MinLon *= 1000;

      TCHAR shortname[6];
      _stprintf(shortname,_T("P%02d"),j);
      _stprintf(Buffer,
	      TEXT("PFLAC,S,ADDWP,%02d%05.0f%c,%03d%05.0f%c,%s"),
	      DegLat, MinLat, NoS, DegLon, MinLon, EoW, 
	      shortname);
      if (!FlarmDeclareSetGet(d,Buffer)) result = FALSE;
  }

  _stprintf(Buffer,TEXT("PFLAC,S,ADDWP,0000000N,00000000E,LANDING"));
  if(result) if (!FlarmDeclareSetGet(d,Buffer)) result = FALSE;

  // Reboot flarm to make declaration active, according to specs

  Poco::Thread::sleep(100);

  devFormatNMEAString(Buffer, BUFF_LEN, TEXT("PFLAR,0") );
  if(result == TRUE)
    d->Com->WriteString(Buffer);
  Poco::Thread::sleep(100);


  d->Com->SetRxTimeout(RXTIMEOUT);                       // clear timeout
  d->Com->StartRxThread();                       // restart RX thread
  if(result == TRUE)
    return result;
  Poco::Thread::sleep(100);
}
 return false; // no success
}
