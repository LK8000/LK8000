#if !defined(AFX_PARSER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_PARSER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Flarm.h"
#include "Fanet.h"
#if defined(PNA) && defined(UNDER_CE)
#include "lkgpsapi.h"
#endif

struct DeviceDescriptor_t;

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
  int ID;
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

//
// FLARM TRACES
//
typedef struct
{
        double fLat;
        double fLon;
        double fAlt;
//      double fIntegrator;
        int iColorIdx;
} FLARM_TRACE;


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

struct NMEA_INFO
{

  double Latitude;
  double Longitude;
  double TrackBearing;
  double Speed;
  double Altitude; // GPS Altitude
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
  BOOL ExternalWindAvailable;
  double ExternalWindSpeed;
  double ExternalWindDirection;
  BOOL NettoVarioAvailable;
  BOOL AirspeedAvailable;

  unsigned VarioSourceIdx;
  double Vario;

  double NettoVario;
  double Ballast;
  double Bugs;
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
  bool haveRMZfromFlarm;
  double FLARM_SW_Version;
  double FLARM_HW_Version;
  FLARM_TRAFFIC FLARM_Traffic[FLARM_MAX_TRAFFIC];
  FLARM_TRACE	FLARM_RingBuf[MAX_FLARM_TRACES];
  bool FLARMTRACE_bBuffFull;
  int  FLARMTRACE_iLastPtr;
  FANET_WEATHER FANET_Weather[MAXFANETWEATHER];
  FANET_NAME FanetName[MAXFANETDEVICES];

  #if LOGFRECORD
  int SatelliteIDs[MAXSATELLITES];
  #endif

  double SupplyBatteryVoltage;

  #if USESWITCHES
  SWITCH_INFO SwitchState;
  #endif

  double StallRatio;

  BOOL MagneticHeadingAvailable;
  double MagneticHeading;

  BOOL GyroscopeAvailable;
  double Pitch;
  double Roll;

};

double TimeModify(NMEA_INFO* pGPS, int& StartDay);
double TimeModify(const TCHAR* FixTime, NMEA_INFO* info, int& StartDay);

class NMEAParser {
 public:
  NMEAParser();
  static BOOL devParseStream(int portnum,
			      char *String,int len, NMEA_INFO *GPS_INFO);
  void Reset();

  BOOL ParseNMEAString_Internal(const DeviceDescriptor_t& d, TCHAR *String, NMEA_INFO *GPS_INFO);


  bool IsValidBaroSource() {
      return ( RMZAvailable || TASAvailable);
  }

  void ResetRMZ() {
      RMZAvailable = false;
  }

#if defined(PNA) && defined(UNDER_CE)
  static BOOL ParseGPS_POSITION(int portnum,
			      const GPS_POSITION& loc, NMEA_INFO& GPSData);
  BOOL ParseGPS_POSITION_internal(const GPS_POSITION& loc, NMEA_INFO& GPSData);
#endif
  bool connected;
  bool gpsValid;
  bool dateValid; // true if we got RMC sentence with valid fix.
  int nSatellites;

  bool activeGPS;
  bool isFlarm;

  static int StartDay;

 public:

  // these routines can be used by other parsers.
  static double ParseAltitude(TCHAR *, const TCHAR *);
  static size_t ValidateAndExtract(const TCHAR *src, TCHAR *dst, size_t dstsz, TCHAR **arr, size_t arrsz);
  static size_t ExtractParameters(const TCHAR *src, TCHAR *dst, TCHAR **arr, size_t sz);
  static BOOL NMEAChecksum(const TCHAR *String);

  static void ExtractParameter(const TCHAR *Source,
			       TCHAR *Destination,
			       int DesiredFieldNumber);

  static uint8_t AppendChecksum(char *String, size_t size);

  template<size_t size>
  static uint8_t AppendChecksum(char (&String)[size]) {
    return AppendChecksum(String, size);
  }

 private:
  bool GGAAvailable;
  bool RMZAvailable;
  bool RMCAvailable;
  bool TASAvailable;
  double RMZAltitude;
  double LastTime;
  short RMZDelayed;

  double GGAtime;
  double RMCtime;
  double GLLtime;

  bool TimeHasAdvanced(double ThisTime, NMEA_INFO *GPS_INFO);

  BOOL GLL(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL GGA(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL GSA(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL RMC(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL VTG(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL RMB(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL RMZ(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);

  BOOL WP0(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL WP1(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL WP2(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);

  // Additional sentences
  BOOL PTAS1(const DeviceDescriptor_t& d, TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);  // RMN: Tasman instruments.  TAS, Vario, QNE-altitude
  // Garmin magnetic compass
  BOOL HCHDG(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  // LK8000 custom special sentences, always active
  BOOL PLKAS(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  
  // FLARM sentences
  BOOL PFLAV(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL PFLAU(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
  BOOL PFLAA(TCHAR *String, TCHAR **, size_t, NMEA_INFO *GPS_INFO);
};

void Fanet_RefreshSlots(NMEA_INFO *pGPS);
void FLARM_RefreshSlots(NMEA_INFO *GPS_INFO);
void FLARM_EmptySlot(NMEA_INFO *GPS_INFO,int i);
void FLARM_DumpSlot(NMEA_INFO *GPS_INFO, int i);

extern bool EnableLogNMEA;
void LogNMEA(TCHAR* text, int);

#endif
