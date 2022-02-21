#ifndef _COMM_PORTCONFIG_F_
#define _COMM_PORTCONFIG_F_

#include "tchar.h"
#include "types.h"

// Filter switches for different Data directions coming sending from/to external devices
enum DataBiIoDir {
  BiDirOff   =0,	 // OFF    no data exchange with this data (ignore data)
  BiDirIn    =1,	 // IN     only reading of this data from external device
  BiDirOut   =2,	 // OUT    only sending this data to external device
  BiDirInOut =3		 // IN&OUT exchanga data from/to device in both directions (e.g. MC, Radio frequencies)
};


// Filter switches for different Data directions data protocol sentences using for TARGET information transfer
enum DataTP_Type {
  TP_Off  ,  // OFF    no TARGET data exchange
  TP_VTARG,  // IN    $PLXVTARG
  TP_GPRMB,  // OUT   $GPRMB
};


struct DeviceIO {
  DataBiIoDir MCDir = BiDirInOut; // Mac Cready
  DataBiIoDir BUGDir = BiDirInOut; // BUG aka efficency
  DataBiIoDir BALDir = BiDirInOut; // Ballast
  DataBiIoDir STFDir = BiDirIn; // Speed to fly switch Vario/Sollfahrt
  DataBiIoDir WINDDir = BiDirIn; // Wind
  DataBiIoDir BARODir = BiDirIn; // barometric heigt
  DataBiIoDir VARIODir = BiDirOff; // Variometer
  DataBiIoDir SPEEDDir = BiDirIn; // IAS indicated airspeed
  DataTP_Type R_TRGTDir= TP_VTARG; // Receive Navigation Target information protocol sentence

  DataBiIoDir RADIODir = BiDirInOut; // Radio Informations Frequency etc.
  DataBiIoDir TRAFDir = BiDirIn; // Traffix Information (FLARM)
  DataBiIoDir GYRODir = BiDirIn; // Gyroskop information
  DataBiIoDir GFORCEDir = BiDirIn; // G-Force values
  DataBiIoDir OATDir = BiDirIn; // Outside air temperature
  DataBiIoDir BAT1Dir = BiDirIn; // Battery 1 voltage
  DataBiIoDir BAT2Dir = BiDirIn; // Battery 2 voltage
  DataBiIoDir POLARDir = BiDirOff; // Polar 2 voltage
  DataBiIoDir DirLink = BiDirOff; // Direct Link
  DataTP_Type T_TRGTDir = TP_VTARG; // Send Navigation Target information protocol sentence
  DataBiIoDir QNHDir = BiDirInOut; // QNH data exchange
};

BOOL IsDirInput(DataBiIoDir IODir);
BOOL IsDirOutput(DataBiIoDir IODir);

enum BitIndex_t {
  bit8N1=0,
  bit7E1,
};

#define DEVNAMESIZE  32
// max URL length
#define MAX_URL_LEN  50

// Fixed text for a disabled device. Cannot be used for translations.
#define DEV_DISABLED_NAME	"DISABLED"

class PortConfig_t final {
public:
  PortConfig_t() = default;

  bool IsDisabled() const {
    if(szDeviceName[0]) {
      return (_tcscmp(szDeviceName, _T(DEV_DISABLED_NAME)) == 0);
    }
    return true;
  }

  const TCHAR* GetPort() const {
    if(_tcscmp(szDeviceName, TEXT("Internal")) == 0) {
      return _T("internal");
    }
    return szPort;
  }

  void SetPort(const TCHAR* port) {
    _tcscpy(szPort, port);
  }

  // device driver name.
  TCHAR szDeviceName[DEVNAMESIZE+1] = { _T(DEV_DISABLED_NAME) };

  // serial Port Parameters
  unsigned dwSpeedIndex = 2;
  BitIndex_t dwBitIndex = bit8N1;

  // Network Parameters (TCP / UDP)
  TCHAR szIpAddress[MAX_URL_LEN] = {};
  unsigned dwIpPort = 23;

  // replay port
  TCHAR Replay_FileName[MAX_PATH] = {};
  int ReplaySpeed = 1;
  bool RawByteData = true;
  int ReplaySync = 0;

  // ext sound on KOBO
  bool UseExtSound = false;
  
  // LX Driver Config
  DeviceIO PortIO;

protected:
  friend void LKDeviceSave(const TCHAR*);
  friend void LKParseProfileString(const char*, const char*);

  // Port identifier
  TCHAR szPort[MAX_PATH] = {};
};

#endif // _COMM_PORTCONFIG_F_
