#if !defined(AFX_PARSER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_PARSER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include "Sizes.h"
#include "Flarm.h"


typedef struct _FLARM_TRAFFIC
{
  double Latitude;
  double Longitude;
  double TrackBearing;
  double Speed;
  double Altitude;
  double TurnRate;
  double ClimbRate;
  double RelativeNorth;
  double RelativeEast;
  double RelativeAltitude;
  long ID;
  TCHAR Name[MAXFLARMNAME+1];
  TCHAR Cn[MAXFLARMCN+1];
  unsigned short IDType;
  unsigned short AlarmLevel;
  double Time_Fix;
  unsigned short Type;
  unsigned short Status; // 100120
  bool Locked; // 100120
  // When set true, name has been changed and Cn must be updated
  bool UpdateNameFlag;
  double Average30s;
  // These are calculated values, updated only inside an offline copy
  double Distance;
  double Bearing;
  double AltArriv;
  double GR;
  double EIAS;
} FLARM_TRAFFIC;

typedef enum {
  NONE,
  GPS_RMA,
  GPS_RMZ,
  GPS_RMC,
  BASE,
  BORGELT500,
  CAI302,
  CAI_GPS_NAV,
  COMPEO,
  CONDOR,
  DIGIFLY,
  DSX,
  EW,
  EW_MICRO_REC,
  FLYMASTER_F1,
  FLYTEC,
  GENERIC,
  ILEC,
  IMI,
  LK_EXT1,
  LX,
  LX16xx,
  LXV7,
  LX_NANO,
  NMEA_OUT,
  POSIGRAPH,
  VOLKSLOGGER,
  WESTERBOER,
  XCOM760,
  ZANDER,
  HOLUX,
  TASMAN,
  ROYALTEK3200,
  FLARM

} DEVICE_TYPE;

#if USESWITCHES
typedef struct _SWITCH_INFO
{
  bool AirbrakeLocked;
  bool FlapPositive;
  bool FlapNeutral;
  bool FlapNegative;
  bool GearExtended;
  bool Acknowledge;
  bool Repeat;
  bool SpeedCommand;
  bool UserSwitchUp;
  bool UserSwitchMiddle;
  bool UserSwitchDown;
  bool VarioCircling;
  bool FlapLanding;
  // bool Stall;
} SWITCH_INFO;
#endif

typedef struct _NMEA_INFO
{

  double Latitude;
  double Longitude;
  double TrackBearing;
  double Speed;
  double Altitude;
  //  TCHAR  WaypointID[WAY_POINT_ID_SIZE + 1];
  //  double WaypointBearing;
  //  double WaypointDistance;
  //  double WaypointSpeed; IGNORED NOW
  double CrossTrackError;
  double Time;
  int Hour;
  int Minute;
  int Second;
  int Month;
  int Day;
  int Year;
  bool NAVWarning;
  double IndicatedAirspeed;
  double TrueAirspeed;
  double BaroAltitude;
  double MacReady;
  BOOL BaroAltitudeAvailable;
  DEVICE_TYPE BaroDevice;
  long BaroTime;
  BOOL ExternalWindAvailalbe;
  double ExternalWindSpeed;
  double ExternalWindDirection;
  BOOL VarioAvailable;
  BOOL NettoVarioAvailable;
  BOOL AirspeedAvailable;
  double Vario;
  double NettoVario;
  double Ballast;
  double Bugs;
  double Gload;
  BOOL AccelerationAvailable;
  double AccelX;
  double AccelY;
  double AccelZ;
  int SatellitesUsed;
  BOOL TemperatureAvailable;
  double OutsideAirTemperature;
  BOOL HumidityAvailable;
  double RelativeHumidity;

  int	ExtBatt_Bank;
  double ExtBatt1_Voltage;
  double ExtBatt2_Voltage;

  unsigned short FLARM_RX;
  unsigned short FLARM_TX;
  unsigned short FLARM_GPS;
  unsigned short FLARM_AlarmLevel;
  bool FLARM_Available;
  FLARM_TRAFFIC FLARM_Traffic[FLARM_MAX_TRAFFIC];
  int SatelliteIDs[MAXSATELLITES];

  double SupplyBatteryVoltage;

  #if USESWITCHES
  SWITCH_INFO SwitchState;
  #endif
  BOOL MovementDetected;

  double StallRatio;

} NMEA_INFO;


void StatusMessageBaro(TCHAR zzMsgID[],DEVICE_TYPE DeviceIdx);
int GetBaroDeviceName(DEVICE_TYPE eDevice ,TCHAR  szDevName[]);
bool UpdateBaroSource( NMEA_INFO* GPS_INFO, DEVICE_TYPE DeviceIdx, double fAlt);
class NMEAParser {
 public:
  NMEAParser();
  static void UpdateMonitor(void);
  static BOOL ParseNMEAString(int portnum,
			      TCHAR *String, NMEA_INFO *GPS_INFO);
  static void Reset(void);
  static bool PortIsFlarm(int portnum);
  void _Reset(void);

  BOOL ParseNMEAString_Internal(TCHAR *String, NMEA_INFO *GPS_INFO);
  bool gpsValid;
  int nSatellites;

  bool activeGPS;
  bool isFlarm;

  static int StartDay;

 public:
  static void TestRoutine(NMEA_INFO *GPS_INFO);

  // these routines can be used by other parsers.
  static double ParseAltitude(TCHAR *, const TCHAR *);
  static size_t ValidateAndExtract(const TCHAR *src, TCHAR *dst, size_t dstsz, TCHAR **arr, size_t arrsz);
  static size_t ExtractParameters(const TCHAR *src, TCHAR *dst, TCHAR **arr, size_t sz);
  static BOOL NMEAChecksum(const TCHAR *String);

  static void ExtractParameter(const TCHAR *Source, 
			       TCHAR *Destination, 
			       int DesiredFieldNumber);

 private:
  BOOL GSAAvailable;
  BOOL GGAAvailable;
  BOOL RMZAvailable;
  BOOL RMAAvailable;
  bool RMCAvailable; 
  bool TASAvailable;
  double RMZAltitude;
  double RMAAltitude;
  double LastTime;

  bool TimeHasAdvanced(double ThisTime, NMEA_INFO *GPS_INFO);
  static double TimeModify(double FixTime, NMEA_INFO* info);
  static double TimeConvert(double FixTime, NMEA_INFO* info);

  BOOL GLL(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL GGA(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL GSA(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL RMC(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL VTG(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL RMB(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL RMA(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL RMZ(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  
  BOOL WP0(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL WP1(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL WP2(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);

  // Additional sentences
  BOOL PTAS1(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);  // RMN: Tasman instruments.  TAS, Vario, QNE-altitude

  // LK8000 custom special sentences, always active
  BOOL PLKAS(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  // FLARM sentances
  BOOL PFLAU(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL PFLAA(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
#ifdef DSX
  BOOL PDSXT(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
#endif
};

void FLARM_RefreshSlots(NMEA_INFO *GPS_INFO);
void FLARM_EmptySlot(NMEA_INFO *GPS_INFO,int i);
void FLARM_DumpSlot(NMEA_INFO *GPS_INFO, int i);

extern bool EnableLogNMEA;
void LogNMEA(TCHAR* text);

#endif
