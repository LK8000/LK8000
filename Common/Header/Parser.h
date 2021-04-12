#if !defined(AFX_PARSER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_PARSER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#include "Flarm.h"
#include "Fanet.h"
#include "Geographic/GeoPoint.h"
#include "Util/ScopeExit.hxx"

#if defined(PNA) && defined(UNDER_CE)
#include "lkgpsapi.h"
#endif

#include "NMEA/Info.h"

struct DeviceDescriptor_t;

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



GeoPoint GetCurrentPosition(const NMEA_INFO& Info);

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
int FLARM_FindSlot(NMEA_INFO *GPS_INFO, uint32_t RadioId);

extern bool EnableLogNMEA;
void LogNMEA(TCHAR* text, int);

#endif
