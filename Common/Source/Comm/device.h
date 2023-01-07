
#ifndef	DEVICE_H
#define	DEVICE_H

#include "MapWindow.h"
#include "ComPort.h"
#include "Comm/PortConfig.h"
#include "BtHandler.h"
#include <vector>
#include "Util/tstring.hpp"
#include "utils/stl_utils.h"

#define	NUMDEV		 6

#define	devA()	    (&DeviceList[0])
#define	devB()	    (&DeviceList[1])
#define devC()      (&DeviceList[2])
#define devD()      (&DeviceList[3])
#define devE()      (&DeviceList[4])
#define devF()      (&DeviceList[5])

class COMMPortItem_t {
public:
    inline COMMPortItem_t(const TCHAR* szName, const TCHAR* szLabel =_T("")) :
			_sName(szName), _sLabel(szLabel)
	{
	}

	inline COMMPortItem_t(tstring&& szName, tstring&& szLabel) :
			_sName(szName), _sLabel(szLabel)
	{
	}

#ifndef NO_BLUETOOTH
    inline COMMPortItem_t(const CBtDevice* pDev) : _sName(pDev->BTPortName()), _sLabel() {
        _sLabel = _T("BT_SPP:") + pDev->GetName();
    }

    inline COMMPortItem_t& operator=(const CBtDevice* pDev) {
        _sName = pDev->BTPortName();
        _sLabel = _T("BT_SPP:") + pDev->GetName();
        return (*this);
    }
#endif
    inline bool IsSamePort(const TCHAR* szName) const { return _sName == szName; }

    inline const TCHAR* GetName() const { return _sName.c_str(); }
    inline const TCHAR* GetLabel() const { return _sLabel.empty()?_sName.c_str():_sLabel.c_str(); }

protected:
    tstring _sName;
    tstring _sLabel;
};

typedef std::vector<COMMPortItem_t> COMMPort_t;

typedef struct Declaration {
  TCHAR PilotName[64];
  TCHAR AircraftType[32];
  TCHAR AircraftRego[32];
  TCHAR CompetitionClass[32];
  TCHAR CompetitionID[32];
  int num_waypoints;
  const WAYPOINT *waypoint[MAXTASKPOINTS];
} Declaration_t;



struct DeviceDescriptor_t {
  
  DeviceDescriptor_t();
  
  DeviceDescriptor_t(const DeviceDescriptor_t&) = delete;
  DeviceDescriptor_t(DeviceDescriptor_t&&) = delete;
  DeviceDescriptor_t& operator= (const DeviceDescriptor_t&) = delete;
  DeviceDescriptor_t& operator= (DeviceDescriptor_t&&) = delete;
  
  ComPort *Com;
  TCHAR	Name[DEVNAMESIZE+1];

  BOOL (*DirectLink)(DeviceDescriptor_t *d, BOOL	bLinkEnable);
  BOOL (*ParseNMEA)(DeviceDescriptor_t *d, TCHAR *String, NMEA_INFO *GPS_INFO);
  BOOL (*ParseStream)(DeviceDescriptor_t *d, char *String, int len, NMEA_INFO *GPS_INFO);
  BOOL (*PutMacCready)(DeviceDescriptor_t	*d,	double McReady);
  BOOL (*PutBugs)(DeviceDescriptor_t *d, double	Bugs);
  BOOL (*PutBallast)(DeviceDescriptor_t	*d,	double Ballast);
  BOOL (*PutVolume)(DeviceDescriptor_t	*d,	int Volume);
  BOOL (*PutRadioMode)(DeviceDescriptor_t	*d,	int mode);
  BOOL (*PutSquelch)(DeviceDescriptor_t	*d,	int Squelch);
  BOOL (*PutFreqActive)(DeviceDescriptor_t	*d,	unsigned khz, const TCHAR* StationName);
  BOOL (*StationSwap)(DeviceDescriptor_t	*d);
  BOOL (*PutFreqStandby)(DeviceDescriptor_t	*d,	unsigned khz, const TCHAR* StationName);
  BOOL (*Open)(DeviceDescriptor_t *d);
  BOOL (*Close)(DeviceDescriptor_t *d);
  BOOL (*LinkTimeout)(DeviceDescriptor_t *d);
  BOOL (*Declare)(DeviceDescriptor_t *d, const Declaration_t *decl, unsigned errorBuffLen, TCHAR errBuffer[]);
  
  BOOL (*IsRadio)(DeviceDescriptor_t *d);
  BOOL (*PutQNH)(DeviceDescriptor_t *d, double NewQNH);
  BOOL (*OnSysTicker)(DeviceDescriptor_t *d);
  BOOL (*PutVoice)(DeviceDescriptor_t *d, const TCHAR *Sentence);
  BOOL (*Config)(DeviceDescriptor_t	*d);
  BOOL (*HeartBeat)(DeviceDescriptor_t *d);
  BOOL (*NMEAOut)(DeviceDescriptor_t *d, const TCHAR* String);
  BOOL (*PutTarget)(DeviceDescriptor_t *d, const WAYPOINT& wpt);
 
  bool IsBaroSource;

  bool m_bAdvancedMode;
  int iSharedPort;
  int PortNumber;
  bool Disabled;
  // Com port diagnostic
  int Status;
  unsigned Rx;
  unsigned ErrRx;
  unsigned Tx;
  unsigned ErrTx;
  // Com ports hearth beats, based on LKHearthBeats
  unsigned HB;
#ifdef DEVICE_SERIAL
  int HardwareId;
  int SerialNumber;
  double SoftwareVer;
#endif
  NMEAParser nmeaParser;
//  DeviceIO PortIO[NUMDEV];
  void InitStruct(int i);
};

typedef	DeviceDescriptor_t *PDeviceDescriptor_t;

void devWriteNMEAString(PDeviceDescriptor_t d, const TCHAR *Text);

#ifdef ANDROID
extern Mutex COMMPort_mutex; // needed for Bluetooth LE scan
#endif
extern COMMPort_t COMMPort;

extern DeviceDescriptor_t	DeviceList[NUMDEV];

extern Mutex CritSec_Comm;

void RefreshComPortList();

void RestartCommPorts();

BOOL devInit();
void devCloseAll();
PDeviceDescriptor_t devGetDeviceOnPort(unsigned Port);
BOOL ExpectString(PDeviceDescriptor_t d, const TCHAR *token);

// return true if all device are disabled 
bool devIsDisabled();
BOOL devOpen(PDeviceDescriptor_t d);
BOOL devDirectLink(PDeviceDescriptor_t d,	BOOL bLink);
void devParseNMEA(int portNum, TCHAR *String,	NMEA_INFO	*GPS_INFO);
BOOL devParseStream(int portNum, char *String,int len,	NMEA_INFO	*GPS_INFO);
BOOL devPutMacCready(double MacCready);
BOOL devRequestFlarmVersion(PDeviceDescriptor_t d);
BOOL devPutBugs(double	Bugs);
BOOL devPutBallast(double Ballast);
BOOL devPutVolume(int Volume);
BOOL devPutFreqSwap();
BOOL devPutRadioMode(int Mode);
BOOL devPutSquelch(int Volume);
BOOL devPutFreqActive(unsigned Freq, const TCHAR* StationName);
BOOL devPutFreqStandby(unsigned Freq, const TCHAR* StationName);
BOOL devLinkTimeout();
BOOL devDeclare(PDeviceDescriptor_t	d, const Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[]);
BOOL devIsLogger(DeviceDescriptor_t& d);
BOOL devIsBaroSource(const DeviceDescriptor_t& d);
BOOL devIsRadio(PDeviceDescriptor_t d);

BOOL devHeartBeat();
BOOL devPutQNH(double NewQNH);

BOOL devPutTarget(const WAYPOINT& wpt);

BOOL devSetAdvancedMode(PDeviceDescriptor_t d,	BOOL bAdvMode);
BOOL devGetAdvancedMode(PDeviceDescriptor_t d);

template<size_t idx>
BOOL devConfig() {
  static_assert(idx < std::size(DeviceList), "invalid index");
  auto& dev = DeviceList[idx];
  return dev.Config && dev.Config(&dev);
}

template<size_t idx>
void devWriteNMEA(const TCHAR *Text) {
  static_assert(idx < std::size(DeviceList), "invalid index");
  devWriteNMEAString(&DeviceList[idx], Text);
}

#endif
