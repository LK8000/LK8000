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
#include "utils/stringext.h"
#include "LKInterface.h"
#include "Baro.h"
#include "Comm/wait_ack.h"
#include "OS/Sleep.h"
#include "Util/StringAPI.hxx"

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
#include "Android/NativeView.hpp"
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

COMMPort_t::iterator FindCOMMPort(const TCHAR* port) {
  return std::find_if(COMMPort.begin(), COMMPort.end(), [&](const auto& item) {
    return item.IsSamePort(port);
  });
}

DeviceDescriptor_t DeviceList[NUMDEV];

/**
 * Call DeviceDescriptor_t::*func on all connected device except @Sender.
 * @return TRUE if at least one device success.
 *
 * TODO : report witch device failed (useless still return value are never used).
 */
template<typename Callable, typename ...Args>
BOOL for_all_device(DeviceDescriptor_t* Sender, Callable&& func, Args&&... args) {
    if (SIMMODE) {
      return TRUE;
    }
    unsigned nbDeviceFailed = 0;

    for (DeviceDescriptor_t& d : DeviceList) {
      if (&d == Sender) {
        continue; // ignore sender.
      }
      ScopeLock Lock(CritSec_Comm);
      if (d.IsReady()) {
        nbDeviceFailed += (d.*func)(std::forward<Args>(args)...) ? 0 : 1;
      }
    }
    return (nbDeviceFailed > 0);
}

/**
 * Call DeviceDescriptor_t::*func on all connected device.
 * @return TRUE if at least one device success.
 */
template<typename Callable, typename ...Args>
BOOL for_all_device(Callable&& func, Args&&... args) {
  constexpr DeviceDescriptor_t* null_descriptor = nullptr;
  return for_all_device(null_descriptor, std::forward<Callable>(func), std::forward<Args>(args)...);
}

static BOOL FlarmDeclare(DeviceDescriptor_t* d, const Declaration_t *decl);

BOOL ExpectString(DeviceDescriptor_t* d, const TCHAR *token){

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

#define TMP_STR_SIZE 512

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
        if (access(path, R_OK|W_OK) == 0) {
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

  if (KoboExportSerialAvailable() && !IsKoboOTGKernel()) {
    if (FindCOMMPort(_T("/dev/ttyGS0")) == COMMPort.end()) {
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

  // Fanet on Air3 7.3+
  if (lk::filesystem::exist("/dev/ttyMT2")
        && StringIsEqual(native_view->GetProduct(), "AIR3")) {

    auto it = FindCOMMPort("/dev/ttyMT2");
    if (it == COMMPort.end()) {
      COMMPort.emplace_back(_T("/dev/ttyMT2"), "/dev/ttyMT2 (Fanet+)");
    }
    else {
      (*it) = { _T("/dev/ttyMT2"), _T("/dev/ttyMT2 (Fanet+)") };
    }
  }

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

bool DeviceDescriptor_t::IsReady() const {
  if (Disabled) {
    return false;
  }
  return Com && Com->IsReady();
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

    PutQNH = nullptr;
    OnSysTicker = nullptr;
    Config = nullptr;
    HeartBeat = nullptr;
    NMEAOut = nullptr;
    PutTarget = nullptr;
    Disabled = true;

    IsBaroSource = false;
    IsRadio = false;

    HB = 0; // counter

    iSharedPort = -1;
    m_bAdvancedMode = false;

#ifdef DEVICE_SERIAL
    HardwareId = 0;
    SerialNumber = 0;
    SoftwareVer = 0;
#endif

    nmeaParser.Reset();

    IgnoreMacCready.Reset();
    IgnoreBugs.Reset();
    IgnoreBallast.Reset();

}

BOOL DeviceDescriptor_t::_PutMacCready(double McReady) {
  if (PutMacCready) {
    IgnoreMacCready.Update();
    return PutMacCready(this, McReady);
  }
  return FALSE;
}

BOOL DeviceDescriptor_t::_PutBugs(double Bugs) {
  if (PutBugs) {
    IgnoreBugs.Update();
    return PutBugs(this, Bugs);
  }
  return FALSE;
}

BOOL DeviceDescriptor_t::_PutBallast(double Ballast) {
  if (PutBallast) {
    IgnoreBallast.Update();
    return PutBallast(this, Ballast);
  }
  return FALSE;
}

BOOL DeviceDescriptor_t::_PutVolume(int Volume) {
  return PutVolume && PutVolume(this, Volume);
}

BOOL DeviceDescriptor_t::_PutRadioMode(int mode) {
  return PutRadioMode && PutRadioMode(this, mode);
}

BOOL DeviceDescriptor_t::_PutSquelch(int Squelch) {
  return PutSquelch && PutSquelch(this, Squelch);
}

BOOL DeviceDescriptor_t::_PutFreqActive(unsigned khz, const TCHAR* StationName) {
  return PutFreqActive && PutFreqActive(this, khz, StationName);
}

BOOL DeviceDescriptor_t::_StationSwap() {
  return StationSwap && StationSwap(this);
}

BOOL DeviceDescriptor_t::_PutFreqStandby(unsigned khz, const TCHAR* StationName) {
  return PutFreqStandby && PutFreqStandby(this, khz, StationName);
}

BOOL DeviceDescriptor_t::_PutTarget(const WAYPOINT& wpt) {
  return PutTarget && PutTarget(this, wpt);
}

BOOL DeviceDescriptor_t::_SendData(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated) {
  return SendData && SendData(this, Basic, Calculated);
}

BOOL DeviceDescriptor_t::_PutQNH(double NewQNH) {
  return PutQNH && PutQNH(this, NewQNH);
}

BOOL DeviceDescriptor_t::_LinkTimeout() {
  return LinkTimeout && LinkTimeout(this);
}

BOOL DeviceDescriptor_t::_HeartBeat() {
  return HeartBeat && HeartBeat(this);
}

BOOL DeviceDescriptor_t::RecvMacCready(double McReady) {
  if (IgnoreMacCready.Check(5000)) {
    CheckSetMACCREADY(McReady, this);
    return TRUE;
  }
  return FALSE;
}

BOOL DeviceDescriptor_t::RecvBugs(double Bugs) {
  if (IgnoreBugs.Check(5000)) {
    CheckSetBugs(Bugs, this);
    return TRUE;
  }
  return FALSE;
}

BOOL DeviceDescriptor_t::RecvBallast(double Ballast) {
  if (IgnoreBallast.Check(5000)) {
    CheckSetBallast(Ballast, this);
    return TRUE;
  }
  return FALSE;
}

void RestartCommPorts() {

    StartupStore(_T(". RestartCommPorts begin @%s"), WhatTimeIsIt());

    devCloseAll();

    devInit();

    TestLog(_T(". RestartCommPorts end @%s"), WhatTimeIsIt());
}

// Only called from devInit() above which
// is in turn called with LockComm
BOOL devOpen(DeviceDescriptor_t* d) {
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

  bool check_prefix(const tstring_view& Port, tstring_view Prefix) {
    return (Port.substr(0, Prefix.size()) == Prefix);
  }

  ComPort* make_ComPort(int idx, const PortConfig_t& Config) {
    const tstring_view Port = Config.GetPort();

    if (check_prefix(Port, _T("BT_SPP:"))) {
      if(BluetoothStart()) {
        return new BthPort(idx, &Port[7]);
      }
    }
    else if (check_prefix(Port, _T("BT_HM10:"))) {
#ifdef ANDROID
      return new BleHM10Port(idx, &Port[8]);
#endif
    }
    else if (check_prefix(Port, DEV_INTERNAL_NAME)) {
#ifdef ANDROID
      return new InternalPort(idx, Port.data());
#else
      return new GpsIdPort(idx, Port.data());
#endif
    }
    else if (check_prefix(Port, _T("TCPClient"))) {
      StartWifi();
      return new TCPClientPort(idx, Port.data());
    }
    else if (check_prefix(Port, _T("TCPServer"))) {
      StartWifi();
      return new TCPServerPort(idx, Port.data());
    }
    else if (check_prefix(Port, _T("UDPServer"))) {
      StartWifi();
      return new UDPServerPort(idx, Port.data());
    }
    else if (check_prefix(Port, _T("Bluetooth Server"))) {
#ifdef ANDROID
      return new BluetoothServerPort(idx, Port.data());
#endif
    }
    else if (check_prefix(Port, _T("IOIOUart_"))) {
#ifdef ANDROID
      return new IOIOUartPort(idx, Port.data(), Config.GetBaudrate());
#endif
    }
    else if (check_prefix(Port, _T("USB:"))) {
#ifdef ANDROID
      return new UsbSerialPort(idx, &Port[4], Config.GetBaudrate(), Config.dwBitIndex);
#endif
    }
    else if (check_prefix(Port, NMEA_REPLAY)) {
      return new FilePort(idx, Port.data());
    } else {
      return new SerialPort(idx, Port.data(), Config.GetBaudrate(), Config.dwBitIndex, PollingMode);
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

  ComPort* port = WithLock(CritSec_Comm, [&]() {
    if (d.Close) {
      d.Close(&d);
    }

    auto port = std::exchange(d.Com, nullptr);
    if (d.iSharedPort >= 0) {
      // don't close shared Ports, these are only copies!
      port = nullptr;
    }
    return port;
  });

  if (port) {
    port->Close();
    delete port;
  }
}

void devCloseAll() {
  std::for_each(std::begin(DeviceList), std::end(DeviceList), &devClose);
}


DeviceDescriptor_t* devGetDeviceOnPort(unsigned Port){

  if(Port < std::size(DeviceList)) {
    return &DeviceList[Port];
  }
  return nullptr;
}


BOOL devParseStream(int portNum, char* stream, int length, NMEA_INFO *pGPS) {
  DeviceDescriptor_t* din = devGetDeviceOnPort(portNum);
  if (!din) {
    return FALSE;
  }
  if (!din->Com) {
    return FALSE; // Port Closed...
  }

  bool ret = FALSE;
  for (auto& d : DeviceList) {
    if (d.ParseStream) {
      if ((d.iSharedPort == portNum) || (d.PortNumber == portNum)) {
        d.HB = LKHearthBeats;
        if (d.ParseStream(din, stream, length, pGPS)) {

        }
        ret = TRUE;
      }
    }
  }
  return ret;
}



// Called from Port task, after assembly of a string from serial port, ending with a LF
void devParseNMEA(int portNum, const char* String, NMEA_INFO *pGPS){
  LogNMEA(String, portNum); // We must manage EnableLogNMEA internally from LogNMEA

  DeviceDescriptor_t* d = devGetDeviceOnPort(portNum);
  if(!d) {
    return;
  }
  if (!d->Com) {
    return; // Port Closed...
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


BOOL devSetAdvancedMode(DeviceDescriptor_t* d,	BOOL bAdvMode) {
  if(d) {
    d->m_bAdvancedMode = bAdvMode;
    return true;
  }
  return false;
}

BOOL devGetAdvancedMode(DeviceDescriptor_t* d) {
  if(d) {
    return d->m_bAdvancedMode;
  }
  return false;
}


BOOL devDirectLink(DeviceDescriptor_t* d,	BOOL bLinkEnable) {
  if (SIMMODE) {
    return TRUE;
  }
  return d && d->DirectLink && d->DirectLink(d, bLinkEnable);
}

/**
 * Send MacCready to all connected device.
 * @param MacCready new MC value
 * @param Sender device to ignore, use nullptr to send to all device.
 * @return FALSE if error on one device.
 */
BOOL devPutMacCready(double MacCready, DeviceDescriptor_t* Sender) {
    return for_all_device(Sender, &DeviceDescriptor_t::_PutMacCready, MacCready);
}


BOOL devRequestFlarmVersion(DeviceDescriptor_t* d)
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
BOOL devPutBugs(double Bugs, DeviceDescriptor_t* Sender) {
    return for_all_device(Sender, &DeviceDescriptor_t::_PutBugs, Bugs);
}

/**
 * Send Ballast % to all connected device.
 * @param Ballast [0.0 - 1.0]
 * @return FALSE if error on one device.
 */
BOOL devPutBallast(double Ballast, DeviceDescriptor_t* Sender) {
    return for_all_device(Sender, &DeviceDescriptor_t::_PutBallast, Ballast);
}

BOOL devHeartBeat() {
  return for_all_device(&DeviceDescriptor_t::_HeartBeat);
}

BOOL devLinkTimeout() {
  return for_all_device(&DeviceDescriptor_t::_LinkTimeout);
}


BOOL devDeclare(DeviceDescriptor_t* d, const Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[])
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
  _sntprintf(buffer, BUFF_LEN, _T("%s: %s..."), MsgToken<1400>(), MsgToken<571>());
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
BOOL devIsBaroSource(const DeviceDescriptor_t& d) {
  return d.IsBaroSource;
}

/**
 * used only in devInit() : already under LockComm ...
 */
BOOL devIsRadio(DeviceDescriptor_t* d) {
  return d && d->IsRadio;
}


BOOL devPutQNH(double NewQNH) {
  return for_all_device(&DeviceDescriptor_t::_PutQNH, NewQNH);
}

BOOL devPutTarget(const WAYPOINT& wpt) {
  return for_all_device(&DeviceDescriptor_t::_PutTarget, wpt);
}

namespace {

template<size_t size>
void devFormatNMEAString(char (&dst)[size], const char *text) {
  assert(text);
  unsigned crc = nmea_crc(text);
  lk::snprintf(dst, "$%s*%02X\r\n", text, crc);
}

} // namespace

uint8_t nmea_crc(const char *text) {
  uint8_t crc = 0U;
  for (const char* c = text; *c; ++c) {
    crc ^= static_cast<uint8_t>(*c);
  }
  return crc;
}

//
// NOTICE V5: this function is used only by LXMiniMap device driver .
// The problem is that it is locking Comm from RXThread and this is 
// creating a possible deadlock situation.
void devWriteNMEAString(DeviceDescriptor_t* d, const TCHAR *text)
{
  ScopeLock Lock(CritSec_Comm);
  if (d && !d->Disabled && d->Com) {
    char tmp[512];
    devFormatNMEAString(tmp, to_utf8(text).c_str());

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
  return for_all_device(&DeviceDescriptor_t::_PutVolume, Volume);

}

/**
 * Send Squelch Level to all connected device.
 * @param Squelch [1 - 10]
 * @return FALSE if error on one device.
 */
BOOL devPutSquelch(int Squelch) {
  RadioPara.SqValid = false;
  return for_all_device(&DeviceDescriptor_t::_PutSquelch, Squelch);

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
  return for_all_device(&DeviceDescriptor_t::_PutRadioMode, mode);
}

/**
 * Send FreqSwap cmd to all connected device.
 * @return FALSE if error on one device.
 */
BOOL devPutFreqSwap() {
  RadioPara.ActiveValid = false;
  RadioPara.PassiveValid = false;
  return for_all_device(&DeviceDescriptor_t::_StationSwap);

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
    return for_all_device(&DeviceDescriptor_t::_PutFreqActive, khz, StationName);
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
    return for_all_device(&DeviceDescriptor_t::_PutFreqStandby, khz, StationName);
  }
  return false;
}

namespace {

template <size_t size>
void flarm_command(char (&dst)[size], char command, const char* key, const char* value) {
  size_t out_size = lk::snprintf(dst, "$PFLAC,%c,%s,%s", command, key, value);
  if (out_size < size) {
    unsigned crc = nmea_crc(dst + 1);
    lk::snprintf(&dst[out_size], size - out_size, "*%02X\r\n", crc);
  }
}

BOOL FlarmDeclareSetGet(DeviceDescriptor_t* d, const char* key, const TCHAR* value) {
  if (!d->Com) {
    return FALSE;
  }

  char ascii_value[TMP_STR_SIZE];
  to_usascii(value, ascii_value);

  char tmp_s[TMP_STR_SIZE];
  flarm_command(tmp_s, 'S', key, ascii_value);

  char tmp_a[TMP_STR_SIZE];
  flarm_command(tmp_a, 'A', key, ascii_value);

  wait_ack_shared_ptr wait_ack = d->make_wait_ack(tmp_a);

  TestLog(_T(". Flarm Decl: > %s"), to_tstring(tmp_s).c_str());

  d->Com->WriteString(tmp_s);

  bool success = wait_ack->wait(20000);

  TestLog(_T(". Flarm Decl: < %s"), success ? to_tstring(tmp_a).c_str() : _T("failed"));
  return success;
}

}  // namespace

BOOL FlarmDeclare(DeviceDescriptor_t* d, const Declaration_t* decl) {
  if (!FlarmDeclareSetGet(d, "PILOT", decl->PilotName)) {
    return FALSE;
  }

  if (!FlarmDeclareSetGet(d, "GLIDERID", decl->AircraftRego)) {
    return FALSE;
  }

  if (!FlarmDeclareSetGet(d, "GLIDERTYPE", decl->AircraftType)) {
    return FALSE;
  }

  if (!FlarmDeclareSetGet(d, "COMPID", decl->CompetitionID)) {
    return FALSE;
  }

  if (!FlarmDeclareSetGet(d, "COMPCLASS", decl->CompetitionClass)) {
    return FALSE;
  }

  if (!FlarmDeclareSetGet(d, "NEWTASK", _T("Task"))) {
    return FALSE;
  }

  if (!FlarmDeclareSetGet(d, "ADDWP", _T("0000000N,00000000E,TAKEOFF"))) {
    return FALSE;
  }

  for (int j = 0; j < decl->num_waypoints; j++) {
    auto& point = *(decl->waypoint[j]);

    char NoS = (point.Latitude > 0) ? 'N' : 'S';
    double latitude = std::abs(point.Latitude);
    int DegLat = latitude;
    int MinLat = (latitude - DegLat) * 60. * 10000.;

    char EoW = (point.Longitude > 0) ? 'E' : 'W';
    double longitude = std::abs(point.Longitude);
    int DegLon = longitude;
    int MinLon = (longitude - DegLon) * 60. * 10000.;

    TCHAR value[32];
    lk::snprintf(value, _T("%02d%05d%c,%03d%05d%c,P%02d"), DegLat, MinLat, NoS, DegLon, MinLon, EoW, j);
    if (!FlarmDeclareSetGet(d, "ADDWP", value)) {
      return FALSE;
    }
  }

  if (!FlarmDeclareSetGet(d, "ADDWP", _T("0000000N,00000000E,LANDING"))) {
    return FALSE;
  }

  // Reboot flarm to make declaration active, according to specs
  Sleep(100);

  d->Com->WriteString("$PFLAR,0*55\r\n");

  Sleep(100);

  return TRUE;  // success
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

void SendDataToExternalDevice(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated) {
  if (Basic.NAVWarning) {
    return; // only send data with valid GPS FIX
  }

  static auto time = Basic.Time;
  if (std::exchange(time, Basic.Time) == Basic.Time) {
    return; // only send data if GPS Time change
  }

  for_all_device(&DeviceDescriptor_t::_SendData, Basic, Calculated);
}
