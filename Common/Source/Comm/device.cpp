/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: device.cpp,v 8.6 2010/12/13 10:21:06 root Exp root $
*/

#include "externs.h"
#include "Dialogs/dlgProgress.h"
#include "utils/stl_utils.h"
#include "Util/TruncateString.hpp"
#include "BtHandler.h"
#include "SerialPort.h"
#include "FilePort.h"
#include "Bluetooth/BthPort.h"
#include "GpsIdPort.h"
#include "TCPPort.h"
#include <functional>
#include "Calc/Vario.h"
#include "Radio.h"
#include "Devices/DeviceRegister.h"
#include "utils/printf.h"
#include "LKInterface.h"
#include "Baro.h"
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
#include "Android/UsbSerialHelper.h"
#include <sstream>
#include <Android/Main.hpp>
#include <Android/IOIOUartPort.h>
#include <Android/UsbSerialPort.h>

#endif


bool devDriverActivated(const TCHAR *DeviceName) ;
    
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
Mutex CritSec_Comm;

#ifdef ANDROID
Mutex COMMPort_mutex; // needed for Bluetooth LE scan
#endif
COMMPort_t COMMPort;

DeviceDescriptor_t DeviceList[NUMDEV];

/**
 * Call DeviceDescriptor_t::*func on all connected device.
 * @return FALSE if error on one device.
 *
 * TODO : report witch device failed (useless still return value are never used).
 */
template<typename Callable, typename ...Args>
BOOL for_all_device(Callable&& func, Args&&... args) {
    if (SIMMODE) {
      return TRUE;
    }
    unsigned nbDeviceFailed = 0;

    ScopeLock Lock(CritSec_Comm);
    for( DeviceDescriptor_t& d : DeviceList) {
      if( !d.Disabled && d.Com && (d.*func) ) {
        nbDeviceFailed += (d.*func)(&d, std::forward<Args>(args)...) ? 0 : 1;
      }
    }
    return (nbDeviceFailed > 0);
}

static BOOL FlarmDeclare(PDeviceDescriptor_t d, const Declaration_t *decl);
static void devFormatNMEAString(TCHAR *dst, size_t sz, const TCHAR *text);

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


BOOL ExpectFlarmString(PDeviceDescriptor_t d, const TCHAR *token){
#define TIMEOUT 500
#define TMP_STR_SIZE 512
#define FLARMDECL_DEBUG 1
  unsigned int i=0;
  int ch;
  TCHAR rec[TMP_STR_SIZE] =_T("");
  unsigned int timeout=0;

  if (!d->Com)
    return FALSE;
  i=0;rec[i] =0;

  while ((ch = d->Com->GetChar()) != 10 )
  {
    if(ch != EOF)
    {
      if(ch == '$') i=0;

      rec[i++] =(TCHAR)ch;
      rec[i] =0;

      if (_tcsnicmp(rec,token,   _tcslen(token)-2)==0)
      {
        if(FLARMDECL_DEBUG) StartupStore(_T("... Flarm Declare received: %s %s"),rec, NEWLINE);
        return(TRUE);
      }
    }
    else
      Poco::Thread::sleep(1);

    if( timeout++ >= TIMEOUT)
    {
      if(FLARMDECL_DEBUG) StartupStore(_T("... Flarm Declare   timeout while receive: %s: %s %s "),token,rec, NEWLINE);
      return(FALSE);
    }
  }
  return(FALSE);
}

// This device is not available if Disabled
// Index 0 or 1 
bool devIsDisabled() {
  for (auto& item : DeviceList) {
    if (!item.Disabled) {
      return false;
    }
  }
  return true;
}

void RefreshComPortList() {
#ifdef ANDROID
    ScopeLock lock(COMMPort_mutex);
#endif

    COMMPort.clear();
#ifdef WIN32    
    TCHAR szPort[10];
    for (unsigned i = 1; i < 10; ++i) {
        _stprintf(szPort, _T("COM%u"), i);
        COMMPort.emplace_back(szPort);
    }

#ifndef UNDER_CE
    for (unsigned i = 10; i < 41; ++i) {
        _stprintf(szPort, _T("COM%u"), i);
        COMMPort.emplace_back(szPort);
    }
#endif

    COMMPort.emplace_back(_T("COM0"));

#if defined(PNA) && defined(UNDER_CE)
    COMMPort.emplace_back(_T("VSP0"));
    COMMPort.emplace_back(_T("VSP1"));
#endif
#endif
    
#ifdef HAVE_POSIX
  
  struct dirent **namelist;
  int n = scandir("/dev", &namelist, 0, alphasort);//need test
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
        char path[512]; // at least MAX_NAME + prefix size
        sprintf(path, "/dev/%s", namelist[i]->d_name);
        if (access(path, R_OK|W_OK) == 0 && access(path, X_OK) < 0) {
          COMMPort.emplace_back(path);
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
      char path[512]; // at least MAX_NAME + prefix size
      sprintf(path, "/dev/serial/by-id/%s", namelist[i]->d_name);
      if (access(path, R_OK|W_OK) == 0 && access(path, X_OK) < 0) {
        sprintf(path, "id:%s", namelist[i]->d_name);
        COMMPort.emplace_back(path);
      }
      free(namelist[i]);
    }      
    free(namelist);
  }

#endif

#ifdef KOBO

  if(KoboExportSerialAvailable() && !IsKoboOTGKernel()) {
    if(std::find_if(COMMPort.begin(), COMMPort.end(), std::bind(&COMMPortItem_t::IsSamePort, _1, _T("/dev/ttyGS0"))) == COMMPort.end()) {
      COMMPort.emplace_back(_T("/dev/ttyGS0"));
    }
  }

#elif defined(TESTBENCH) && defined (__linux__)

  if(lk::filesystem::exist(_T("/lk"))) {
    COMMPort.emplace_back(_T("/lk/ptycom1"));
    COMMPort.emplace_back(_T("/lk/ptycom2"));
    COMMPort.emplace_back(_T("/lk/ptycom3"));
    COMMPort.emplace_back(_T("/lk/ptycom4"));
  }

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

    COMMPort.emplace_back(_T("TCPClient"));
    COMMPort.emplace_back(_T("TCPServer"));
    COMMPort.emplace_back(_T("UDPServer"));
    if(EngineeringMenu)
      COMMPort.emplace_back(NMEA_REPLAY);
#ifdef ANDROID

  JNIEnv *env = Java::GetEnv();
  if (env) {
    if (BluetoothHelper::isEnabled(env)) {

      COMMPort.emplace_back(_T("Bluetooth Server"));

      static constexpr jsize BLUETOOTH_LIST_STRIDE = 3;

      Java::LocalRef<jobjectArray> bonded = BluetoothHelper::list(env);
      if (bonded) {

        jsize nBT = env->GetArrayLength(bonded) / BLUETOOTH_LIST_STRIDE;
        for (jsize i = 0; i < nBT; ++i) {
          Java::String j_address(env, (jstring) env->GetObjectArrayElement(bonded, i * BLUETOOTH_LIST_STRIDE));
          if (!j_address)
            continue;

          const std::string address = j_address.ToString();
          if (address.empty())
            continue;

          Java::String j_name(env, (jstring) env->GetObjectArrayElement(bonded, i * BLUETOOTH_LIST_STRIDE + 1));
          Java::String j_type(env, (jstring) env->GetObjectArrayElement(bonded, i * BLUETOOTH_LIST_STRIDE + 2));

          auto prefix = [&]() {
            if (j_type.ToString() == "HM10") {
              return "BT_HM10:";
            }
            else {
              return "BT_SPP:";
            }
          };

          std::stringstream prefixed_address, name;
          prefixed_address << prefix() << address;
          name << prefix() << (j_name ? j_name.ToString() : std::string());

          COMMPort.emplace_back(prefixed_address.str(), name.str());
        }
      }
    }

    if(UsbSerialHelper::isEnabled(env)) {
      Java::LocalRef<jobjectArray> devices = UsbSerialHelper::list(env);
      if (devices) {
        const jsize device_count = env->GetArrayLength(devices);
        for (jsize i = 0; i < device_count; ++i) {

          Java::String j_name(env, (jstring) env->GetObjectArrayElement(devices, i));
          if (!j_name) {
            continue;
          }

          std::stringstream prefixed_name;
          prefixed_name << "USB:" << j_name.ToString();
          const std::string name = prefixed_name.str();
          COMMPort.emplace_back(name.c_str(), name.c_str());
        }
      }
    }
  }

  if(ioio_helper) {
    COMMPort.emplace_back("IOIOUart_0", "IOIO Uart 0");
    COMMPort.emplace_back("IOIOUart_1", "IOIO Uart 1");
    COMMPort.emplace_back("IOIOUart_2", "IOIO Uart 2");
    COMMPort.emplace_back("IOIOUart_3", "IOIO Uart 3");
  }

#endif

    if(COMMPort.empty()) {
        // avoid segfault on device config  dialog if no comport detected.
        COMMPort.emplace_back(_T("Null"));
    }
}

DeviceDescriptor_t::DeviceDescriptor_t() {
    Rx = 0;
    Tx = 0;
    ErrTx = 0;
    ErrRx = 0;
}

void DeviceDescriptor_t::InitStruct(int i) {
    PortNumber = i;

    Name[0] = '\0';

    DirectLink = nullptr;
    ParseNMEA = nullptr;
    ParseStream = nullptr;
    PutMacCready = nullptr;
    PutBugs = nullptr;
    PutBallast = nullptr;
    PutVolume = nullptr;
    PutRadioMode = nullptr;
    PutSquelch = nullptr;
    PutFreqActive = nullptr;
    StationSwap = nullptr;
    PutFreqStandby = nullptr;
    Open = nullptr;
    Close = nullptr;
    LinkTimeout = nullptr;
    Declare = nullptr;
    IsBaroSource = nullptr;
    IsRadio = nullptr;
    PutQNH = nullptr;
    OnSysTicker = nullptr;
    PutVoice = nullptr;
    Config = nullptr;
    HeartBeat = nullptr;
    NMEAOut = nullptr;
    PutTarget = nullptr;
    Disabled = true;

    Status = CPS_UNUSED; // 100210
    HB = 0; // counter

    iSharedPort = -1;
    m_bAdvancedMode = false;

#ifdef DEVICE_SERIAL
    HardwareId = 0;
    SerialNumber = 0;
    SoftwareVer = 0;
#endif

    nmeaParser.Reset();
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
BOOL devOpen(PDeviceDescriptor_t d) {
  if (d && d->Open) {
    return d->Open(d);
  }
  return TRUE;
}


static bool IsIdenticalPort(int i, int j) {
  const auto& ConfigA = PortConfig[i];
  const auto& ConfigB = PortConfig[j];

  tstring_view PortA = ConfigA.GetPort();
  tstring_view PortB = ConfigB.GetPort();

  // internal can't be shared
  if (PortA == DEV_INTERNAL_NAME) {
    return false;
  }

  // internal can't be shared
  if (PortB == DEV_INTERNAL_NAME) {
    return false;
  }

  // same port name.
  if (PortA == PortB) {

    if (PortA == _T("UDPServer")) {
      // two udp port on same port.
      return (ConfigA.dwIpPort == ConfigB.dwIpPort);
    }

    if (PortA == _T("TCPServer")) {
      // two tcp server port on same port.
      return (ConfigA.dwIpPort == ConfigB.dwIpPort);
    }

    if (PortA == _T("TCPClient")) {
      // two tcp client port on same adress/port.
      return (ConfigA.dwIpPort == ConfigB.dwIpPort)
          && (_tcscmp(ConfigA.szIpAddress, ConfigB.szIpAddress) == 0);
    }

    return true;
  }

  return false;
}

namespace {

  bool BluetoothStart() {
#ifdef NO_BLUETOOTH
      return true;
#else
      CBtHandler* pBtHandler = CBtHandler::Get();
      if (pBtHandler && pBtHandler->IsOk()) {
        if (pBtHandler->StartHW()) {
          return true;
        }
      }
      return false;
#endif
  }

  void StartWifi() {
#ifdef KOBO
    if(!IsKoboWifiOn()) {
      KoboWifiOn();
    }
#endif
  }


  ComPort* make_ComPort(int idx, const PortConfig_t& Config) {

    const TCHAR* Port = Config.GetPort();

    if (_tcsncmp(Port, _T("BT_SPP:"), 7) == 0) {
      if(BluetoothStart()) {
        return new BthPort(idx, &Port[3]);
      }
    }
#ifdef ANDROID
    if (_tcsncmp(Port, _T("BT_HM10:"), 3) == 0) {
      return new BleHM10Port(idx, &Port[8]);
    }
#endif
    else if (_tcscmp(Port, DEV_INTERNAL_NAME) == 0) {
#ifdef ANDROID
      return new InternalPort(idx, Port);
#else
      return new GpsIdPort(idx, Port);
#endif
    }
    else if (_tcscmp(Port, _T("TCPClient")) == 0) {
      StartWifi();
      return new TCPClientPort(idx, Port);
    }
    else if (_tcscmp(Port, _T("TCPServer")) == 0) {
      StartWifi();
      return new TCPServerPort(idx, Port);
    }
    else if (_tcscmp(Port, _T("UDPServer")) == 0) {
      StartWifi();
      return new UDPServerPort(idx, Port);
    }
    else if (_tcscmp(Port, _T("Bluetooth Server")) == 0) {
#ifdef ANDROID
      return new BluetoothServerPort(idx, Port);
#endif
    }
    else if(_tcsncmp(Port, _T("IOIOUart_"), 9) == 0) {
#ifdef ANDROID
      return new IOIOUartPort(idx, Port, Config.GetBaudrate());
#endif
    }
    else if (_tcsncmp(Port, _T("USB:"), 4) == 0) {
#ifdef ANDROID
      return new UsbSerialPort(idx, &Port[4], Config.GetBaudrate(), Config.dwBitIndex);
#endif
    }
    else if (_tcscmp(Port, NMEA_REPLAY) == 0) {
      return new FilePort(idx, Port);
    } else {
      return new SerialPort(idx, Port, Config.GetBaudrate(), Config.dwBitIndex, PollingMode);
    }

    return nullptr; // unknown port type...
  }

}


BOOL devInit() {
    ScopeLock Lock(CritSec_Comm);

    RadioPara.Enabled = (SIMMODE);

    ResetBaroAvailable(GPS_INFO);
    ResetVarioAvailable(GPS_INFO);

    for (unsigned i = 0; i < NUMDEV; i++) {
        const auto& Config = PortConfig[i];
        auto& dev = DeviceList[i];
        
        dev.InitStruct(i);

        if (SIMMODE){
            continue;
        }

        dev.Disabled = Config.IsDisabled();
        if (dev.Disabled) {
            StartupStore(_T(". Device %c is DISABLED.%s"), (_T('A') + i), NEWLINE);
            continue;
        }

        const DeviceRegister_t* pDev = GetRegisteredDevice(Config.szDeviceName);
        if (!pDev) {
            dev.Disabled = true;
            StartupStore(_T(". Device %c : invalide drivers name <%s>%s"), (_T('A') + i), Config.szDeviceName, NEWLINE);
            continue;
        }

        const TCHAR* Port = Config.GetPort();

        dev.iSharedPort =-1;
        for(uint j = 0; j < i ; j++) {
            if( (_tcscmp(Port, NMEA_REPLAY) != 0)
                 && (!DeviceList[j].Disabled) && (IsIdenticalPort(i,j)) &&  DeviceList[j].iSharedPort <0) {

                dev.iSharedPort =j;
                StartupStore(_T(". Port <%s> Already used, Device %c shares it with %c ! %s"), Port, (_T('A') + i),(_T('A') + j), NEWLINE);
                dev.Com = DeviceList[j].Com ;
                dev.Status = CPS_OPENOK;
                pDev->Installer(&dev);

                if(devIsRadio(&dev)) {
                    RadioPara.Enabled = true;
                    StartupStore(_T(".  RADIO  %c  over  <%s>%s"), (_T('A') + i),  Port, NEWLINE);
                }
            }
        }

        if(dev.iSharedPort >=0) { // skip making new device on shared ports
            continue;
        }

        StartupStore(_T(". Device %c is <%s> Port=%s"), (_T('A') + i), Config.szDeviceName, Port);

        ComPort* Com = make_ComPort(i, Config);
        if (Com && Com->Initialize()) {
            pDev->Installer(&dev);
            /*
             * Need to be done before anny #DeviceDescriptor_t::Callback call.
             */
            dev.Com = Com;
            if (Com->IsReady()) {
                devOpen(&dev);
            }

            Com->StartRxThread();
        } else {
            delete Com;
            dev.Status = CPS_OPENKO;
        }

       if(devIsRadio(&dev)) {
          RadioPara.Enabled = true;
          StartupStore(_T(".  RADIO  %c  over  <%s>%s"), (_T('A') + i),  Port, NEWLINE);
       }
       if(devDriverActivated(TEXT("PVCOM"))) {
          RadioPara.Enabled = true;
          StartupStore(_T(".  RADIO  %c  PVCOM over  shared <%s>%s"), (_T('A') + i),  Port, NEWLINE);
       }

    }

    return TRUE;
}

// Tear down methods should always succeed.
// Called from devInit() above under LockComm
// Also called when shutting down via devCloseAll()
static void devClose(DeviceDescriptor_t& d) {
  ScopeLock Lock(CritSec_Comm);

  if (d.Close) {
    d.Close(&d);
  }

  if (d.iSharedPort < 0) { // don't close shared Ports,  these are only copies!
    if (d.Com) {
      d.Com->Close();
      delete d.Com;
    }
  }
  d.Com = nullptr; // if we do that before Stop RXThread , Crash ....
  d.Status = CPS_CLOSED;
}

void devCloseAll() {
  std::for_each(std::begin(DeviceList), std::end(DeviceList), &devClose);
}


PDeviceDescriptor_t devGetDeviceOnPort(unsigned Port){

  if(Port < std::size(DeviceList)) {
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
void devParseNMEA(int portNum, TCHAR *String, NMEA_INFO *pGPS){
  LogNMEA(String, portNum); // We must manage EnableLogNMEA internally from LogNMEA

  PDeviceDescriptor_t d = devGetDeviceOnPort(portNum);
  if(!d) {
    return;
  }

  d->HB=LKHearthBeats;

  // intercept device specific parser routines 
    for(DeviceDescriptor_t& d2 : DeviceList) {

      if((d2.iSharedPort == portNum) ||  (d2.PortNumber == portNum)) {

        if ( d2.ParseNMEA && WithLock(CritSec_FlightData, d2.ParseNMEA, d, String, pGPS) ) {
          continue;
        }
        // call ParseNMEAString_Internal only for master port if string are not device specific.
        if( &d2 == d) {
          d->nmeaParser.ParseNMEAString_Internal(*d, String, pGPS);
        }
      }
    }

    if(d->nmeaParser.activeGPS) {

      for(DeviceDescriptor_t& d2 : DeviceList) {

          if(d2.Com && !d2.Disabled && d2.NMEAOut) { // NMEA out ! even on multiple ports
            // stream pipe, pass nmea to other device (NmeaOut)
            d2.NMEAOut(&d2, String); // TODO code: check TX buffer usage and skip it if buffer is full (outbaudrate < inbaudrate)
          }
      }
    }
}


BOOL devSetAdvancedMode(PDeviceDescriptor_t d,	BOOL bAdvMode) {
  if(d) {
    d->m_bAdvancedMode = bAdvMode;
    return true;
  }
  return false;
}

BOOL devGetAdvancedMode(PDeviceDescriptor_t d) {
  if(d) {
    return d->m_bAdvancedMode;
  }
  return false;
}


BOOL devDirectLink(PDeviceDescriptor_t d,	BOOL bLinkEnable) {
  if (SIMMODE) {
    return TRUE;
  }
  return d && d->DirectLink && d->DirectLink(d, bLinkEnable);
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

BOOL devHeartBeat() {
  return for_all_device(&DeviceDescriptor_t::HeartBeat);
}

BOOL devLinkTimeout() {
  return for_all_device(&DeviceDescriptor_t::LinkTimeout);
}


BOOL devDeclare(PDeviceDescriptor_t d, const Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[])
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

  WithLock(CritSec_Comm, [&]() {
    if (d && !d->Disabled) {
      devDirectLink(d, true);

      if (d->Declare) {
        result = d->Declare(d, decl, errBufferLen, errBuffer);
      } else if(d->nmeaParser.isFlarm) {
        result |= FlarmDeclare(d, decl);
      }

      devDirectLink(d, false);
    }
  });
  CloseProgressDialog();
  
  return result;
}

BOOL devIsLogger(DeviceDescriptor_t& d) {
  ScopeLock Lock(CritSec_Comm);
  return d.Declare || d.nmeaParser.isFlarm;
}

/**
 * used only in devInit() and UpdateMonitor() : already under LockComm ...
 */
BOOL devIsBaroSource(PDeviceDescriptor_t d) {
  if(d) {
    if (d->IsBaroSource) {
      return d->IsBaroSource(d);
    }
    return d->nmeaParser.IsValidBaroSource();
  }
  return false;
}

/**
 * used only in devInit() : already under LockComm ...
 */
BOOL devIsRadio(PDeviceDescriptor_t d) {
  if (d && d->IsRadio) {
    return d->IsRadio(d);
  }
  return false;
}


BOOL devPutQNH(double NewQNH) {
  return for_all_device(&DeviceDescriptor_t::PutQNH, NewQNH);
}

BOOL devPutTarget(const WAYPOINT& wpt) {
  return for_all_device(&DeviceDescriptor_t::PutTarget, wpt);
}

static void devFormatNMEAString(TCHAR *dst, size_t sz, const TCHAR *text)
{
  BYTE chk;
  int i, len = _tcslen(text);

  LKASSERT(text!=NULL);
  LKASSERT(dst!=NULL);

  for (chk = i = 0; i < len; i++)
    chk ^= (BYTE)text[i];

  lk::snprintf(dst, sz, TEXT("$%s*%02X\r\n"), text, chk);
}

//
// NOTICE V5: this function is used only by LXMiniMap device driver .
// The problem is that it is locking Comm from RXThread and this is 
// creating a possible deadlock situation.
void devWriteNMEAString(PDeviceDescriptor_t d, const TCHAR *text)
{
  ScopeLock Lock(CritSec_Comm);
  if (d && !d->Disabled && d->Com) {
    TCHAR tmp[512];
    devFormatNMEAString(tmp, 512, text);

    devDirectLink(d, true);
    d->Com->WriteString(tmp);
    devDirectLink(d, false);
  }
}

bool devDriverActivated(const TCHAR *DeviceName) {
  for (auto& Port : PortConfig) {
    if ((_tcscmp(Port.szDeviceName, DeviceName) == 0)) {
      return true;
    }
  }
  return false;
}

/**
 * Send Volume Level to all connected device.
 * @param Volume [1 - 20]
 * @return FALSE if error on one device.
 */
BOOL devPutVolume(int Volume) {
  RadioPara.VolValid = false;
  return for_all_device(&DeviceDescriptor_t::PutVolume, Volume);

}

/**
 * Send Squelch Level to all connected device.
 * @param Squelch [1 - 10]
 * @return FALSE if error on one device.
 */
BOOL devPutSquelch(int Squelch) {
  RadioPara.SqValid = false;
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
  RadioPara.DualValid = false;
  return for_all_device(&DeviceDescriptor_t::PutRadioMode, mode);
}

/**
 * Send FreqSwap cmd to all connected device.
 * @return FALSE if error on one device.
 */
BOOL devPutFreqSwap() {
  RadioPara.ActiveValid = false;
  RadioPara.PassiveValid = false;
  return for_all_device(&DeviceDescriptor_t::StationSwap);

}



/**
 * Send FreqActive cmd to all connected device.
 * @return FALSE if error on one device.
 */


BOOL devPutFreqActive(unsigned khz, const TCHAR* StationName) {
  if (ValidFrequency(khz)) {
    RadioPara.ActiveValid = false;
    RadioPara.ActiveKhz = khz;
    CopyTruncateString(RadioPara.ActiveName, NAME_SIZE, StationName);
    return for_all_device(&DeviceDescriptor_t::PutFreqActive, khz, StationName);
  }
  return false;
}

/**
 * Send FreqStandby cmd to all connected device.
 * @return FALSE if error on one device.
 */
BOOL devPutFreqStandby(unsigned khz, const TCHAR* StationName) {
  if (ValidFrequency(khz)) {
    RadioPara.PassiveValid = false;
    RadioPara.PassiveKhz = khz;
    CopyTruncateString(RadioPara.PassiveName, NAME_SIZE, StationName);
    return for_all_device(&DeviceDescriptor_t::PutFreqStandby, khz, StationName);
  }
  return false;
}


static BOOL 
FlarmDeclareSetGet(PDeviceDescriptor_t d, TCHAR *Buffer) {

  TCHAR tmp[TMP_STR_SIZE];
  TCHAR tmp2[TMP_STR_SIZE];

  _sntprintf(tmp, TMP_STR_SIZE, TEXT("%s"), Buffer);
  devFormatNMEAString(tmp2, TMP_STR_SIZE, tmp );
  if(FLARMDECL_DEBUG) StartupStore(_T("... ===================== %s"), NEWLINE);
  if(FLARMDECL_DEBUG) StartupStore(_T("... Flarm Declare     send: %s %s"),tmp2, NEWLINE);

  if (d->Com)
  {
    d->Com->WriteString(tmp2);
  }
 
  tmp[6]= _T('A');
  devFormatNMEAString(tmp2, TMP_STR_SIZE, tmp );
  if(FLARMDECL_DEBUG)StartupStore(_T("... Flarm Declare expected: %s %s"),tmp2, NEWLINE);
  for(int i=0; i < 20; i++) /* try to get expected answer max 5 times*/
  {
    if (ExpectFlarmString(d, tmp2)) 
      return true;
  }

  return false;

};


BOOL FlarmDeclare(PDeviceDescriptor_t d, const Declaration_t *decl)
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

      TCHAR shortname[12];
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

BOOL IsDirInput(DataBiIoDir IODir) {
  switch(IODir) {
    case BiDirOff  : return false; // OFF    no data exchange with this data (ignore data)
    case BiDirIn   : return true;  // IN     only reading of this data from external device
    case BiDirOut  : return false; // OUT    only sending this data to external device
    case BiDirInOut: return true;  // IN&OUT exchanga data from/to device in both directions (e.g. MC, Radio frequencies)
  }
  return false;
}

BOOL IsDirOutput(DataBiIoDir IODir) {
  switch(IODir) {
    case BiDirOff  : return false;  // OFF    no data exchange with this data (ignore data)
    case BiDirIn   : return false;  // IN     only reading of this data from external device
    case BiDirOut  : return true ;  // OUT    only sending this data to external device
    case BiDirInOut: return true ;  // IN&OUT exchanga data from/to device in both directions (e.g. MC, Radio frequencies)
  }
  return false;
}
