#if !defined(AFX_PARSER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_PARSER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#include "types.h"
#include "Flarm.h"
#include "Devices/Fanet/Fanet.h"
#include "Geographic/GeoPoint.h"
#include "Util/ScopeExit.hxx"
#include "Time/PeriodClock.hpp"
#include "Sizes.h"

#if defined(PNA) && defined(UNDER_CE)
#include "lkgpsapi.h"
#endif

#include "NMEA/Info.h"

struct DeviceDescriptor_t;

double TimeModify(NMEA_INFO* pGPS, int& StartDay);
double TimeModify(const char* FixTime, NMEA_INFO* info, int& StartDay);
double TimeModify(const wchar_t* FixTime, NMEA_INFO* info, int& StartDay);

class NMEAParser {
 public:
  NMEAParser();

  void Reset();

  BOOL ParseNMEAString_Internal(DeviceDescriptor_t& d, const char* String, NMEA_INFO *GPS_INFO);

  void CheckRMZ();

  void CheckGpsValid() {
    // reset gpsValid if last valid fix is older than 6 second
    if (lastGpsValid.Elapsed() > 6000) {
      gpsValid = false;
      lastGpsValid.Reset();
    }
  }

#if defined(PNA) && defined(UNDER_CE)
  static BOOL ParseGPS_POSITION(int portnum,
			      const GPS_POSITION& loc, NMEA_INFO& GPSData);
  BOOL ParseGPS_POSITION_internal(const GPS_POSITION& loc, NMEA_INFO& GPSData);
#endif
  bool connected; // true if GGA or RMC is received.
  bool gpsValid;  // true if we have Valid GPS fix
  bool dateValid; // true if we got RMC sentence with valid fix.
  int nSatellites;

  bool activeGPS;
  bool isFlarm;

  PeriodClock lastGpsValid; // to check time elapsed since last valid gps fix

  static int StartDay;

  void setFlarmAvailable(NMEA_INFO *GPS_INFO);

public:

  // these routines can be used by other parsers.

  static double ParseAltitude(const char* value, const char* format);

  static size_t ValidateAndExtract(const char* src, char (&dst)[MAX_NMEA_LEN], char* (&arr)[MAX_NMEA_PARAMS]);
  static size_t ExtractParameters(const char* src, char (&dst)[MAX_NMEA_LEN], char* (&arr)[MAX_NMEA_PARAMS]);

  static void ExtractParameter(const char* Source, char* Destination, int DesiredFieldNumber);
  static BOOL NMEAChecksum(const char* String);

#ifdef UNICODE
  static void ExtractParameter(const wchar_t* Source, wchar_t* Destination, int DesiredFieldNumber);
#endif

  static uint8_t AppendChecksum(char *String, size_t size);

  template<size_t size>
  static uint8_t AppendChecksum(char (&String)[size]) {
    return AppendChecksum(String, size);
  }

 private:
  bool GGAAvailable;
  bool RMZAvailable;
  unsigned LastRMZHB;
  bool RMCAvailable;
  bool TASAvailable;

  double LastTime;
  short RMZDelayed;

  double GGAtime;
  double RMCtime;
  double GLLtime;

  bool TimeHasAdvanced(double ThisTime, NMEA_INFO *GPS_INFO);

  BOOL GLL(const char* String, char**, size_t, NMEA_INFO *GPS_INFO);
  BOOL GGA(const char* String, char**, size_t, NMEA_INFO *GPS_INFO);
  BOOL GSA(const char* String, char**, size_t, NMEA_INFO *GPS_INFO);
  BOOL RMC(const char* String, char**, size_t, NMEA_INFO *GPS_INFO);
  BOOL VTG(const char* String, char**, size_t, NMEA_INFO *GPS_INFO);
  BOOL RMB(const char* String, char**, size_t, NMEA_INFO *GPS_INFO);
  BOOL RMZ(DeviceDescriptor_t& d, const char* String, char**, size_t, NMEA_INFO *GPS_INFO);

  BOOL WP0(const char* String, char**, size_t, NMEA_INFO *GPS_INFO);
  BOOL WP1(const char* String, char**, size_t, NMEA_INFO *GPS_INFO);
  BOOL WP2(const char* String, char**, size_t, NMEA_INFO *GPS_INFO);

  // Additional sentences
  BOOL PTAS1(DeviceDescriptor_t& d, const char* String, char**, size_t, NMEA_INFO *GPS_INFO);  // RMN: Tasman instruments.  TAS, Vario, QNE-altitude
  // Garmin magnetic compass
  BOOL HCHDG(const char* String, char**, size_t, NMEA_INFO *GPS_INFO);
  // LK8000 custom special sentences, always active
  BOOL PLKAS(const char* String, char**, size_t, NMEA_INFO *GPS_INFO);
  
  // FLARM sentences
  BOOL PFLAV(const char* String, char**, size_t, NMEA_INFO *GPS_INFO);
  BOOL PFLAU(const char* String, char**, size_t, NMEA_INFO *GPS_INFO);
  BOOL PFLAA(const char* String, char**, size_t, NMEA_INFO *GPS_INFO);

  void UpdateFlarmScale(NMEA_INFO *pGPS);

private:
  double FLARM_NorthingToLatitude = 0.0;
  double FLARM_EastingToLongitude = 0.0;
  GeoPoint FLARM_lastPosition;
};

void Fanet_RefreshSlots(NMEA_INFO *pGPS);
void FLARM_RefreshSlots(NMEA_INFO *GPS_INFO);
void FLARM_EmptySlot(NMEA_INFO *GPS_INFO,int i);
void FLARM_DumpSlot(NMEA_INFO *GPS_INFO, int i);
int FLARM_FindSlot(NMEA_INFO *GPS_INFO, uint32_t RadioId);

extern bool EnableLogNMEA;
void LogNMEA(const char* text, unsigned PortNum);

#endif
