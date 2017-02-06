
#ifndef	DEVICE_H
#define	DEVICE_H

#include "MapWindow.h"
#include "ComPort.h"
#include "BtHandler.h"
#include <vector>
#include "Util/tstring.hpp"
#include "utils/stl_utils.h"

#define DEVNAMESIZE  32
#define	NUMDEV		 6

#ifdef RADIO_ACTIVE
#define	NUMREGDEV	 40 // Max number of registered devices
#else
#define	NUMREGDEV	 36 // Max number of registered devices
#endif // RADIO_ACTIVE

#define	devA()	    (&DeviceList[0])
#define	devB()	    (&DeviceList[1])
#define devC()      (&DeviceList[2])
#define devD()      (&DeviceList[3])
#define devE()      (&DeviceList[4])
#define devF()      (&DeviceList[5])
#define devAll()    (NULL)

class COMMPortItem_t {
public:
    inline COMMPortItem_t(const TCHAR* szName, const TCHAR* szLabel =_T("")) {
		_sName = szName;
		_sLabel = szLabel;
	}
#ifndef NO_BLUETOOTH
    inline COMMPortItem_t(const CBtDevice* pDev) : _sName(pDev->BTPortName()), _sLabel() {
        _sLabel = _T("BT:") + pDev->GetName();
    }

    inline COMMPortItem_t& operator=(const CBtDevice* pDev) {
        _sName = pDev->BTPortName();
        _sLabel = _T("BT:") + pDev->GetName();
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

typedef	enum {dfGPS, dfLogger, dfSpeed,	dfVario, dfBaroAlt,	dfWind, dfVoice, dfNmeaOut, dfRadio} DeviceFlags_t;

typedef struct Declaration {
  TCHAR PilotName[64];
  TCHAR AircraftType[32];
  TCHAR AircraftRego[32];
  TCHAR CompetitionClass[32];
  TCHAR CompetitionID[32];
  int num_waypoints;
  const WAYPOINT *waypoint[MAXTASKPOINTS];
} Declaration_t;

typedef	struct DeviceDescriptor_t{
  int	Port;
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
  BOOL (*PutFreqActive)(DeviceDescriptor_t	*d,	double Freq, TCHAR StationName[]);
  BOOL (*StationSwap)(DeviceDescriptor_t	*d);
  BOOL (*PutFreqStandby)(DeviceDescriptor_t	*d,	double Standby, TCHAR StationName[]);
  BOOL (*Open)(DeviceDescriptor_t	*d,	int	Port);
  BOOL (*Close)(DeviceDescriptor_t *d);
  BOOL (*Init)(DeviceDescriptor_t	*d);
  BOOL (*LinkTimeout)(DeviceDescriptor_t *d);
  BOOL (*Declare)(DeviceDescriptor_t *d, Declaration_t *decl, unsigned errorBuffLen, TCHAR errBuffer[]);
  BOOL (*IsLogger)(DeviceDescriptor_t	*d);
  BOOL (*IsGPSSource)(DeviceDescriptor_t *d);
  BOOL (*IsBaroSource)(DeviceDescriptor_t *d);
  BOOL (*IsRadio)(DeviceDescriptor_t *d);
  BOOL (*PutQNH)(DeviceDescriptor_t *d, double NewQNH);
  BOOL (*OnSysTicker)(DeviceDescriptor_t *d);
  BOOL (*PutVoice)(DeviceDescriptor_t *d, TCHAR *Sentence);
  BOOL (*Config)(DeviceDescriptor_t	*d);

  int iSharedPort;
  int PortNumber;
  bool Disabled;
  bool bNMEAOut;
  // Com port diagnostic
  int Status;
  unsigned Rx;
  unsigned ErrRx;
  unsigned Tx;
  unsigned ErrTx;
  // Com ports hearth beats, based on LKHearthBeats
  unsigned HB;
  
	NMEAParser nmeaParser;
	
  void InitStruct(int i);
}DeviceDescriptor_t;

typedef	DeviceDescriptor_t *PDeviceDescriptor_t;

#define Port1WriteNMEA(s)	devWriteNMEAString(devA(), s)
#define Port2WriteNMEA(s)	devWriteNMEAString(devB(), s)

void devWriteNMEAString(PDeviceDescriptor_t d, const TCHAR *Text);
void VarioWriteSettings(void);

typedef	struct{
  const TCHAR	         *Name;
  unsigned int		 Flags;
  BOOL   (*Installer)(PDeviceDescriptor_t d);
} DeviceRegister_t;


extern COMMPort_t COMMPort;

extern DeviceDescriptor_t	DeviceList[NUMDEV];
extern DeviceRegister_t   DeviceRegister[NUMREGDEV];
extern int DeviceRegisterCount;
extern DeviceDescriptor_t *pDevPrimaryBaroSource;
extern DeviceDescriptor_t *pDevSecondaryBaroSource;

inline 
PDeviceDescriptor_t devX(unsigned idx) {
	if(idx < array_size(DeviceList)) {
		return &DeviceList[idx];
	}
	return nullptr;
}

void UnlockComm();
void LockComm();

void RefreshComPortList();

BOOL devRegister(const TCHAR *Name,	int	Flags, BOOL (*Installer)(PDeviceDescriptor_t d));
LPCTSTR devRegisterGetName(int Index);
bool devNameCompare(const DeviceRegister_t& dev, const TCHAR *DeviceName);

void RestartCommPorts();

BOOL devInit();
BOOL devCloseAll(void);
PDeviceDescriptor_t devGetDeviceOnPort(int Port);
BOOL ExpectString(PDeviceDescriptor_t d, const TCHAR *token);
BOOL devHasBaroSource(void);
bool devIsDisabled(int devindex);

BOOL devDirectLink(PDeviceDescriptor_t d,	BOOL bLink);
BOOL devParseNMEA(int portNum, TCHAR *String,	NMEA_INFO	*GPS_INFO);
BOOL devParseStream(int portNum, char *String,int len,	NMEA_INFO	*GPS_INFO);
BOOL devPutMacCready(double MacCready);
BOOL devRequestFlarmVersion(PDeviceDescriptor_t d);
BOOL devPutBugs(double	Bugs);
BOOL devPutBallast(double Ballast);
BOOL devPutVolume(int Volume);
BOOL devPutFreqSwap();
BOOL devPutRadioMode(int Mode);
BOOL devPutSquelch(int Volume);
BOOL devPutFreqActive(double Freq, TCHAR StationName[]);
BOOL devPutFreqStandby(double Freq, TCHAR StationName[]);
BOOL devLinkTimeout(PDeviceDescriptor_t	d);
BOOL devDeclare(PDeviceDescriptor_t	d, Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[]);
BOOL devIsLogger(PDeviceDescriptor_t d);
BOOL devIsGPSSource(PDeviceDescriptor_t	d);
BOOL devIsBaroSource(PDeviceDescriptor_t d);
BOOL devIsRadio(PDeviceDescriptor_t d);

BOOL devPutQNH(DeviceDescriptor_t *d, double NewQNH);
BOOL devOnSysTicker(DeviceDescriptor_t *d);

BOOL devGetBaroAltitude(double *Value);

BOOL devPutVoice(PDeviceDescriptor_t d, TCHAR *Sentence);


#endif
