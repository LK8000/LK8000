
#ifndef	DEVICE_H
#define	DEVICE_H

#include "MapWindow.h"
#include "ComPort.h"
#include "Comm/PortConfig.h"
#include "BtHandler.h"
#include <vector>
#include "Util/tstring.hpp"
#include "utils/stl_utils.h"
#include "Comm/wait_ack.h"

#define	NUMDEV		 6


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


extern Mutex CritSec_Comm;

struct DeviceDescriptor_t {

  DeviceDescriptor_t();

  DeviceDescriptor_t(const DeviceDescriptor_t&) = delete;
  DeviceDescriptor_t(DeviceDescriptor_t&&) = delete;
  DeviceDescriptor_t& operator= (const DeviceDescriptor_t&) = delete;
  DeviceDescriptor_t& operator= (DeviceDescriptor_t&&) = delete;

  bool IsReady() const;

  ComPort *Com = nullptr;
  TCHAR	Name[DEVNAMESIZE+1];

  BOOL (*DirectLink)(DeviceDescriptor_t* d, BOOL	bLinkEnable);
  BOOL (*ParseNMEA)(DeviceDescriptor_t* d, const char *String, NMEA_INFO *GPS_INFO);
  BOOL (*ParseStream)(DeviceDescriptor_t* d, char *String, int len, NMEA_INFO *GPS_INFO);
  BOOL (*PutMacCready)(DeviceDescriptor_t	*d,	double McReady);
  BOOL (*PutBugs)(DeviceDescriptor_t* d, double	Bugs);
  BOOL (*PutBallast)(DeviceDescriptor_t	*d,	double Ballast);
  BOOL (*PutVolume)(DeviceDescriptor_t	*d,	int Volume);
  BOOL (*PutRadioMode)(DeviceDescriptor_t	*d,	int mode);
  BOOL (*PutSquelch)(DeviceDescriptor_t	*d,	int Squelch);
  BOOL (*PutFreqActive)(DeviceDescriptor_t	*d,	unsigned khz, const TCHAR* StationName);
  BOOL (*StationSwap)(DeviceDescriptor_t	*d);
  BOOL (*PutFreqStandby)(DeviceDescriptor_t	*d,	unsigned khz, const TCHAR* StationName);
  BOOL (*Open)(DeviceDescriptor_t* d);
  BOOL (*Close)(DeviceDescriptor_t* d);
  BOOL (*LinkTimeout)(DeviceDescriptor_t* d);
  BOOL (*Declare)(DeviceDescriptor_t* d, const Declaration_t *decl, unsigned errorBuffLen, TCHAR errBuffer[]);

  BOOL (*PutQNH)(DeviceDescriptor_t* d, double NewQNH);
  BOOL (*OnSysTicker)(DeviceDescriptor_t* d);
  BOOL (*Config)(DeviceDescriptor_t	*d);
  BOOL (*HeartBeat)(DeviceDescriptor_t* d);
  BOOL (*NMEAOut)(DeviceDescriptor_t* d, const char* String);
  BOOL (*PutTarget)(DeviceDescriptor_t* d, const WAYPOINT& wpt);

  /**
   * called at the the end of calculation thread loop for each GPS FIX
   */
  BOOL (*SendData)(DeviceDescriptor_t* d, const NMEA_INFO& Basic, const DERIVED_INFO& Calculated);

  bool IsBaroSource;
  bool IsRadio;

  bool m_bAdvancedMode;
  int iSharedPort;
  int PortNumber;
  bool Disabled;

  // Com port diagnostic
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

  BOOL _PutMacCready(double McReady);
  BOOL _PutBugs(double	Bugs);
  BOOL _PutBallast(double Ballast);
  BOOL _PutVolume(int Volume);
  BOOL _PutRadioMode(int mode);
  BOOL _PutSquelch(int Squelch);
  BOOL _PutFreqActive(unsigned khz, const TCHAR* StationName);
  BOOL _StationSwap();
  BOOL _PutFreqStandby(unsigned khz, const TCHAR* StationName);
  BOOL _PutTarget(const WAYPOINT& wpt);

  BOOL _SendData(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated);

  BOOL _PutQNH(double NewQNH);
  BOOL _LinkTimeout();
  BOOL _HeartBeat();


  BOOL RecvMacCready(double McReady);
  PeriodClock IgnoreMacCready;

  BOOL RecvBugs(double Bugs);
  PeriodClock IgnoreBugs;

  BOOL RecvBallast(double Ballast);
  PeriodClock IgnoreBallast;

  wait_ack_shared_ptr make_wait_ack(const char* str) {
    auto wait_ack = make_wait_ack_shared(CritSec_Comm, str);
    wait_ack_weak = wait_ack;
    return wait_ack;
  }

  wait_ack_shared_ptr lock_wait_ack() {
    return wait_ack_weak.lock();
  }

 private:
  wait_ack_weak_ptr wait_ack_weak;
};

uint8_t nmea_crc(const char *text);

void devWriteNMEAString(DeviceDescriptor_t* d, const TCHAR *Text);

#ifdef ANDROID
extern Mutex COMMPort_mutex; // needed for Bluetooth LE scan
#endif
extern COMMPort_t COMMPort;

extern DeviceDescriptor_t	DeviceList[NUMDEV];

void RefreshComPortList();

void RestartCommPorts();

BOOL devInit();
void devCloseAll();
DeviceDescriptor_t* devGetDeviceOnPort(unsigned Port);
BOOL ExpectString(DeviceDescriptor_t* d, const TCHAR *token);

// return true if all device are disabled 
bool devIsDisabled();
BOOL devOpen(DeviceDescriptor_t* d);
BOOL devDirectLink(DeviceDescriptor_t* d,	BOOL bLink);
void devParseNMEA(int portNum, const char *String,	NMEA_INFO	*GPS_INFO);
BOOL devParseStream(int portNum, char *String,int len,	NMEA_INFO	*GPS_INFO);
BOOL devPutMacCready(double MacCready, DeviceDescriptor_t* Sender);
BOOL devRequestFlarmVersion(DeviceDescriptor_t* d);
BOOL devPutBugs(double	Bugs, DeviceDescriptor_t* Sender);
BOOL devPutBallast(double Ballast, DeviceDescriptor_t* Sender);
BOOL devPutVolume(int Volume);
BOOL devPutFreqSwap();
BOOL devPutRadioMode(int Mode);
BOOL devPutSquelch(int Volume);
BOOL devPutFreqActive(unsigned Freq, const TCHAR* StationName);
BOOL devPutFreqStandby(unsigned Freq, const TCHAR* StationName);
BOOL devLinkTimeout();
BOOL devDeclare(DeviceDescriptor_t* d, const Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[]);
BOOL devIsLogger(DeviceDescriptor_t& d);
BOOL devIsBaroSource(const DeviceDescriptor_t& d);
BOOL devIsRadio(DeviceDescriptor_t* d);

BOOL devHeartBeat();
BOOL devPutQNH(double NewQNH);

BOOL devPutTarget(const WAYPOINT& wpt);

BOOL devSetAdvancedMode(DeviceDescriptor_t* d,	BOOL bAdvMode);
BOOL devGetAdvancedMode(DeviceDescriptor_t* d);

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

void SendDataToExternalDevice(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated);

#endif
