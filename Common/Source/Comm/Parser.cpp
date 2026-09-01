/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Parser.cpp,v 8.12 2010/12/12 16:14:28 root Exp root $

*/

#include "externs.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "Logger.h"
#include "Geoid.h"
#include "GpsWeekNumberFix.h"
#include <algorithm>

using std::string_view_literals::operator""sv;

extern double EastOrWest(double in, TCHAR EoW);
extern double NorthOrSouth(double in, TCHAR NoS);
extern double MixedFormatToDegrees(double mixed);

namespace {

struct ParsedNMEATime {
  int hour = 0;
  int minute = 0;
  int second = 0;
  double time_of_day = 0.0;
};

double NMEATimeOfDay(const NMEA_INFO& gps) {
  return gps.Second + (gps.Minute * 60) + (gps.Hour * 3600);
}

double ApplyDayRollover(double fix_time, int year, int month, int day,
                        int& StartDay) {
  constexpr int SECONDS_PER_DAY = 86400;

  static int day_difference = 0;
  static int previous_months_day_difference = 0;

  // Waiting for the first valid date
  if (StartDay == -1) {
    if (day == 0) {
      return fix_time;
    }
    StartupStore(_T(". First GPS DATE: %d-%d-%d  %s%s"), year, month, day,
                 WhatTimeIsIt(), NEWLINE);
    StartDay = day;
    day_difference = 0;
    previous_months_day_difference = 0;
    return fix_time;  // No offset on first fix
  }

  if (day < StartDay) {
    // Month boundary detected (e.g. day=1, StartDay=26): accumulate elapsed
    // days
    previous_months_day_difference = day_difference + 1;
    day_difference = 0;
    StartDay = day;
    StartupStore(
        _T(". Change GPS DATE to NEW MONTH: %d-%d-%d  (%d days running)%s"),
        year, month, day, previous_months_day_difference, NEWLINE);
  }
  else {
    const int new_day_difference = day - StartDay;
    if (new_day_difference != day_difference) {
      day_difference = new_day_difference;
      StartupStore(_T(". Change GPS DATE: %d-%d-%d  %s%s"), year, month, day,
                   WhatTimeIsIt(), NEWLINE);
    }
  }

  // Add accumulated day offset to keep time monotonic across midnight/month
  // boundaries
  const int total_days = day_difference + previous_months_day_difference;
  if (total_days > 0) {
    fix_time += total_days * SECONDS_PER_DAY;
  }

  return fix_time;
}

template <typename CharT>
ParsedNMEATime ParseNMEATime(const CharT* StrTime) {
  ParsedNMEATime result;
  // NMEA time format: HHMMSS[.ss] - minimum 6 characters required
  size_t len = 0;
  while (StrTime[len] != '\0') ++len;
  if (len < 6) {
    return result;  // Return default (zeros)
  }

  double secs = 0.0;

  if (_istdigit(StrTime[0]) && _istdigit(StrTime[1])) {
    result.hour = (StrTime[0] - '0') * 10 + (StrTime[1] - '0');
  }
  if (_istdigit(StrTime[2]) && _istdigit(StrTime[3])) {
    result.minute = (StrTime[2] - '0') * 10 + (StrTime[3] - '0');
  }
  if (_istdigit(StrTime[4]) && _istdigit(StrTime[5])) {
    result.second = (StrTime[4] - '0') * 10 + (StrTime[5] - '0');
  }

  if (StrTime[6] == '.') {
    int i = 7;
    while (_istdigit(StrTime[i])) {
      double tmp = (StrTime[i] - '0') * 0.1;
      for (int j = 7; j < i; ++j) {
        tmp *= 0.1;
      }
      secs += tmp;
      ++i;
    }
  }

  result.time_of_day =
      secs + result.second + (result.minute * 60) + (result.hour * 3600);
  return result;
}

//
// Make time absolute, over 86400seconds when day is changing
// We need a valid date to use it. We are relying on StartDay.
//
template <typename CharT>
double TimeModify(const CharT* StrTime, NMEA_INFO* pGPS, int& StartDay) {
  const ParsedNMEATime parsed_time = ParseNMEATime(StrTime);
  pGPS->Hour = parsed_time.hour;
  pGPS->Minute = parsed_time.minute;
  pGPS->Second = parsed_time.second;
  return ApplyDayRollover(parsed_time.time_of_day, pGPS->Year, pGPS->Month,
                          pGPS->Day, StartDay);
}

bool NAVWarn(char c) {
  return c != 'A';
}

// minimal speed to use gps bearing
double GetTrackBearingMinSpeed() {
  if (ISCAR) {
    return 0;  // trekking mode/car mode, min speed > 0
  }
  else {
    return 1; // flymode,  min speed >1 knot
  }
}

} // namespace

int NMEAParser::StartDay = -1;


// #define DEBUGSEQ	1
// #define DEBUGBARO	1

NMEAParser::NMEAParser() {
  activeGPS = false;
  Reset();
}

void NMEAParser::Reset() {
  connected = false;
  nSatellites = 0;
  gpsValid = false;
  dateValid = false;
  isFlarm = false;
  GGAAvailable = false;
  RMZAvailable = false;
  LastRMZHB = false;
  RMCAvailable = false;
  RMZDelayed = 3; // wait for this to be zero before using RMZ.

  LastTime = 0;
}

BOOL NMEAParser::ParseNMEAString_Internal(DeviceDescriptor_t& d, const char* String, NMEA_INFO* pGPS) {
  if (!String) {
    return FALSE;
  }

  auto wait_ack = d.lock_wait_ack();
  if (wait_ack && wait_ack->check(String)) {
    return TRUE;
  }

  if (String[0] !='$') {
    return FALSE;
  }

  char ctemp[MAX_NMEA_LEN];
  char* params[MAX_NMEA_PARAMS];

  size_t n_params = ValidateAndExtract(String, ctemp, params);
  if (n_params < 1 || params[0][0] != '$') {
    return FALSE;
  }

  if (params[0][1] == 'P') {
    std::string_view token = params[0] + 2;
    // Proprietary String
    if (token == "TAS1"sv) {
      return PTAS1(d, &String[7], params + 1, n_params - 1, pGPS);
    }
    if (token == "FLAV"sv) {
      return PFLAV(&String[7], params + 1, n_params - 1, pGPS);
    }
    if (token == "FLAA"sv) {
      return PFLAA(&String[7], params + 1, n_params - 1, pGPS);
    }
    if (token == "FLAU"sv) {
      return PFLAU(&String[7], params + 1, n_params - 1, pGPS);
    }
    if (token == "GRMZ"sv) {
      return RMZ(d, &String[7], params + 1, n_params - 1, pGPS);
    }
    if (token == "LKAS"sv) {
      return PLKAS(d, &String[7], params + 1, n_params - 1, pGPS);
    }
    return FALSE;
  }

  if (params[0][1] == 'G') {
    // GNSS String
    std::string_view token = params[0] + 3;
    if (token == "GSA"sv) {
      return GSA(&String[7], params + 1, n_params - 1, pGPS);
    }
    if (token == "GLL"sv) {
      return GLL(&String[7], params + 1, n_params-1, pGPS);
    }
    if (token == "RMB"sv) {
      return RMB(&String[7], params + 1, n_params-1, pGPS);
    }
    if (token == "RMC"sv) {
      return RMC(&String[7], params + 1, n_params - 1, pGPS);
    }
    if (token == "GGA"sv) {
      return GGA(&String[7], params + 1, n_params - 1, pGPS);
    }
    if (token == "VTG"sv) {
      return VTG(&String[7], params + 1, n_params - 1, pGPS);
    }
  }

  if (std::string_view(params[0] + 1) == "HCHDG"sv) {
    return HCHDG(d, &String[7], params + 1, n_params - 1, pGPS);
  }

  return FALSE;
}

void NMEAParser::CheckRMZ() {
  if ((LastRMZHB > 0) && LKHearthBeats > (LastRMZHB+5)) {
    RMZAvailable = false;
  }
}

double TimeModify(const char* FixTime, NMEA_INFO* info, int& StartDay) {
  return TimeModify<char>(FixTime, info, StartDay);
}

double TimeModify(const wchar_t* FixTime, NMEA_INFO* info, int& StartDay) {
  return TimeModify<wchar_t>(FixTime, info, StartDay);
}

double TimeModify(NMEA_INFO* pGPS, int& StartDay) {
  return ApplyDayRollover(NMEATimeOfDay(*pGPS), pGPS->Year, pGPS->Month,
                          pGPS->Day, StartDay);
}

bool NMEAParser::TimeHasAdvanced(double ThisTime, NMEA_INFO *pGPS) {

  // If simulating, we might be in the future already.
  // We CANNOT check for <= because this check may be done by several GGA RMC GLL etc. sentences
  // among the same quantum time
  if(ThisTime< LastTime) {
    TestLog(_T("... TimeHasAdvanced BACK in time: Last=%f This=%f   %s"), LastTime, ThisTime, WhatTimeIsIt());
    LastTime = ThisTime;
    StartDay = -1; // reset search for the first day
    MasterTimeReset();
    return false;
  } else {
    pGPS->Time = ThisTime;
    LastTime = ThisTime;
    return true;
  }
}

BOOL NMEAParser::GSA(const char* String, char** params, size_t nparams,
                     NMEA_INFO* pGPS) {
  /*
   * GSA - GNSS DOP and Active Satellites
   * Format:
   *   $GPGSA,<mode>,<fix_type>,<satellite_id_1>,...,<satellite_id_12>,<pdop>,<hdop>,<vdop>*<checksum>
   * Example:
   *   $GPGSA,A,3,04,05,09,12,24,25,29,31,32,34,1.8,1.0,1.5*33
   * Fields:
   *   <mode> - M = manual, A = automatic
   *   <fix_type> - 1 = no fix, 2 = 2D fix, 3 = 3D fix
   *   <satellite_id_n> - ID of satellite used in fix (up to 12)
   *   <pdop> - Position Dilution of Precision
   *   <hdop> - Horizontal Dilution of Precision
   *   <vdop> - Vertical Dilution of Precision
   */
  return TRUE;
} // END GSA

BOOL NMEAParser::GLL(const char* String, char** params, size_t nparams,
                     NMEA_INFO* pGPS) {
  /*
   * GLL - Geographic Position - Latitude/Longitude
   * Format:
   *   $GPGLL,<lat>,<N/S>,<lon>,<E/W>,<time>,<status>,<mode>*<checksum>
   * Example:
   *   $GPGLL,4916.45,N,12311.12,W,225444,A,*1D
   * Fields:
   *   <lat> - Latitude in ddmm.mmmm format
   *   <N/S> - North or South
   *   <lon> - Longitude in dddmm.mmmm format
   *   <E/W> - East or West
   *   <time> - UTC time in hhmmss.sss format
   *   <status> - A = active, V = void
   *   <mode> - Mode indicator (optional)
   */
  return TRUE;
} // END GLL

BOOL NMEAParser::RMB(const char* String, char** params, size_t nparams,
                     NMEA_INFO* pGPS) {
  /*
   * RMB - Recommended Minimum Navigation Information (Waypoint)
   * Format:
   *   $GPRMB,<status>,<cross_track_error>,<direction_to_steer>,<origin_wp>,<destination_wp>,<range_to_destination>,<bearing_to_destination>,<destination_closure_velocity>,<arrival_status>*<checksum>
   * Example:
   *   $GPRMB,A,0.5,L,WP1,WP2,10.0,045.0,5.0,A*hh
   * Fields:
   *   <status> - A = active, V = void
   *   <cross_track_error> - Cross track error in nautical miles
   *   <direction_to_steer> - L = left, R = right
   *   <origin_wp> - Origin waypoint ID
   *   <destination_wp> - Destination waypoint ID
   *   <range_to_destination> - Range to destination in nautical miles
   *   <bearing_to_destination> - Bearing to destination in degrees
   *   <destination_closure_velocity> - Destination closure velocity in knots
   *   <arrival_status> - A = arrived, V = not arrived
*/
  return TRUE;
} // END RMB

BOOL NMEAParser::VTG(const char* String, char** params, size_t nparams,
                     NMEA_INFO* pGPS) {
  /* VTG - Track Made Good and Ground Speed
   * Format:
   *   $GPVTG,<track_true>,T,<track_magnetic>,M,<speed_knots>,N,<speed_kmh>,K*<checksum>
   * Example:
   *   $GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48
   * Fields:
   *   <track_true> - Track made good relative to true north
   *   <T> - True
   *   <track_magnetic> - Track made good relative to magnetic north
   *   <M> - Magnetic
   *   <speed_knots> - Speed over ground in knots
   *   <N> - Knots
   *   <speed_kmh> - Speed over ground in kilometers per hour
   *   <K> - Kilometers per hour
   */

  // VTG sentence provides track and speed information, already available from
  // RMC, so we do not need to parse it separately

  return TRUE;
} // END VTG

BOOL NMEAParser::RMC(const char* String, char** params, size_t nparams,
                     NMEA_INFO* pGPS) {
  /*
   * RMC - Recommended Minimum Navigation Information
   * Format:
   *   $GPRMC,<time>,<status>,<lat>,<N/S>,<lon>,<E/W>,<speed>,<track>,<date>,<magvar>,<E/W>*<checksum>
   * Example:
   *   $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
   * Fields:
   *   <time> - UTC time in hhmmss.sss format
   *   <status> - A = active, V = void
   *   <lat> - Latitude in ddmm.mmmm format
   *   <N/S> - North or South
   *   <lon> - Longitude in dddmm.mmmm format
   *   <E/W> - East or West
   *   <speed> - Speed over ground in knots
   *   <track> - Track angle in degrees
   *   <date> - Date in ddmmyy format
   *   <magvar> - Magnetic variation in degrees
   *   <E/W> - East or West for magnetic variation
   */

  if (nparams < 9) {
    TESTBENCH_DO_ONLY(
        10, StartupStore(_T(". NMEAParser invalid RMC sentence, nparams=%u%s"),
                         (unsigned)nparams, NEWLINE));
    // max index used is 8...
    return FALSE;
  }

  gpsValid = !NAVWarn(params[1][0]);
  if (gpsValid) {
    lastGpsValid.Update();
  }

  connected = true;
  RMCAvailable = true;  // 100409

  if (!activeGPS) {
    return TRUE;
  }

  const std::lock_guard lock(CritSec_FlightData);

  // If no GGA is available, or if RMC indicates invalid fix, update NAV warning.
  // If GGA is available and RMC has valid fix, respect GGA's stricter fix evaluation.
  if (!GGAAvailable || !gpsValid) {
    pGPS->NAVWarning = !gpsValid;
  }

  if (!gpsValid && !dateValid) {
    // we have valid date with invalid fix only if we have already got valid fix
    return TRUE;
  }

  const size_t size_date = strlen(params[8]);
  // Even with no valid position, we let RMC set the time and date if valid
  int year, month, day;
  if (parse_rmc_date(params[8], size_date, year, month, day)) {
    pGPS->Year = year;
    pGPS->Month = month;
    pGPS->Day = day;
  }
  else {
    //.. Condor not sending valid date;
    if (!DevIsCondor) {
      static bool logbaddate = true;
      if (gpsValid && logbaddate) {  // 091115
        StartupStore(
            _T("------ NMEAParser:RMC Receiving an invalid or null DATE from ")
            _T("GPS"));
        StartupStore(
            _T("------ NMEAParser: Date received is \"%04d-%02d-%02d\""), year,
            month, day);  // 100422
        StartupStore(_T("------ This message will NOT be repeated. %s"),
                     WhatTimeIsIt());
        // _@M875_ "WARNING: GPS IS SENDING INVALID DATE, AND PROBABLY WRONG
        // TIME"
        DoStatusMessage(MsgToken<875>());
        logbaddate = false;
      }
      return TRUE; // skip sentence if date is invalid and not Condor
    }
  }

  dateValid = true;

  const ParsedNMEATime parsed_time = ParseNMEATime(params[0]);
  const double ThisTime = ApplyDayRollover(parsed_time.time_of_day, pGPS->Year,
                                           pGPS->Month, pGPS->Day, StartDay);
  // RMC time has priority on GGA and GLL etc. so if we have it we use it at
  // once
  if (!TimeHasAdvanced(ThisTime, pGPS)) {
    DebugLog(_T("..... RMC time not advanced, skipping \n"));  // 31C
    return FALSE;
  }

  pGPS->Hour = parsed_time.hour;
  pGPS->Minute = parsed_time.minute;
  pGPS->Second = parsed_time.second;

  if (gpsValid) {
    double tmplat = MixedFormatToDegrees(StrToDouble(params[2], nullptr));
    tmplat = NorthOrSouth(tmplat, params[3][0]);

    double tmplon = MixedFormatToDegrees(StrToDouble(params[4], nullptr));
    tmplon = EastOrWest(tmplon, params[5][0]);

    if (!((tmplat == 0.0) && (tmplon == 0.0))) {
      pGPS->Latitude = tmplat;
      pGPS->Longitude = tmplon;
    }

    pGPS->Speed = Units::From(unKnots, StrToDouble(params[6], nullptr));

    if (pGPS->Speed > GetTrackBearingMinSpeed()) {
      pGPS->TrackBearing = AngleLimit360(StrToDouble(params[7], nullptr));
    }
  }  // gpsvalid 091108

  if (!GGAAvailable) {
    // update SatInUse, some GPS receiver dont emmit GGA sentences
    if (!gpsValid) {
      pGPS->SatellitesUsed = 0;
    }
    else {
      pGPS->SatellitesUsed = -1;
    }
  }

  TriggerGPSUpdate();

  return TRUE;
}  // END RMC

BOOL NMEAParser::GGA(const char* String, char** params, size_t nparams,
                     NMEA_INFO* pGPS) {

  /*
   * GGA - Global Positioning System Fix Data
   * Format:
   *   $GPGGA,<time>,<lat>,<N/S>,<lon>,<E/W>,<fix>,<sat>,<HDOP>,<alt>,M,<geoid>,M,<DGPS age>,<DGPS ref>*<checksum>
   * Example:
   *   $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
   * Fields:
   *   <time> - UTC time in hhmmss.sss format
   *   <lat> - Latitude in ddmm.mmmm format
   *   <N/S> - North or South
   *   <lon> - Longitude in dddmm.mmmm format
   *   <E/W> - East or West
   *   <fix> - Fix quality (0 = invalid, 1 = GPS fix, 2 = DGPS fix, etc.)
   *   <sat> - Number of satellites being tracked
   *   <HDOP> - Horizontal dilution of position
   *   <alt> - Altitude above mean sea level
   *   M - Units of altitude (meters)
   *   <geoid> - Height of geoid above WGS84 ellipsoid
   *   M - Units of geoid height (meters)
   *   <DGPS age> - Time since last DGPS update
   *   <DGPS ref> - DGPS reference station id
   *   <checksum> - Checksum
   */
  
  if (nparams < 11) {
    TESTBENCH_DO_ONLY(
        10,
        StartupStore(_T(". NMEAParser invalid GGA sentence, nparams=%u <%s>"),
                     (unsigned)nparams, String));
    // max index used is 10...
    return FALSE;
  }

  connected = true;  // 091208

  /*
   * Fix quality :
   *  0 = invalid
   *  1 = GPS fix (SPS)
   *  2 = DGPS fix
   *  3 = PPS fix
   *  4 = Real Time Kinematic
   *  5 = Float RTK
   *  6 = estimated (dead reckoning) (2.3 feature)
   *  7 = Manual input mode
   *  8 = Simulation mode
   */

  unsigned ggafix = strtoul(params[5], nullptr, 10);
  gpsValid = (ggafix > 0 && ggafix < 6);
  if (ggafix == 6) {
    DebugLog(_T("------ GGA DEAD RECKON fix skipped"));
  }
#ifdef YDEBUG
  // in debug we need to accept manual or simulated fix
  gpsValid = gpsValid || (ggafix == 7 || ggafix == 8);
#endif

  if (gpsValid) {
    lastGpsValid.Update();
  }

  // Only mark GGA as available if it provides a valid fix.
  // If GGA has invalid fix, allow RMC to control NAVWarning.
  GGAAvailable = gpsValid;

  nSatellites = std::min<int>(16, strtol(params[6], nullptr, 10));

  // some device don't send sat in use count, set it to "-1" if fix is valid and
  // sat in use is 0
  if (gpsValid && (nSatellites == 0)) {
    nSatellites = -1;  // unknown count but valid fix !
  }

  if (!activeGPS) {
    return TRUE;
  }

  // since RMC is always available, we rely on it for the primary GPS fix
  // information, GGA only used for additional altitude and satellite info

  const std::lock_guard lock(CritSec_FlightData);

  pGPS->SatellitesUsed = nSatellites;  // 091208
  pGPS->NAVWarning = !gpsValid;        // 091208

  // "Altitude" should always be GPS Altitude.
  pGPS->Altitude = ParseAltitude(params[8], params[9]);
  pGPS->Altitude += (GPSAltitudeOffset / 1000);  // BUGFIX 100429

  if ((*params[10]) && (params[10] != "0"sv)) {
    // No real need to parse this value,
    // but we do assume that no correction is required in this case
    // double GeoidSeparation = ParseAltitude(params[10], params[11]);
  }
  else if (UseGeoidSeparation) {
    pGPS->Altitude -= LookupGeoidSeparation(pGPS->Latitude, pGPS->Longitude);
  }

  return TRUE;
}  // END GGA

// LK8000 IAS , in m/s*10  example: 346 for 34.6 m/s  which is = 124.56 km/h
BOOL NMEAParser::PLKAS(DeviceDescriptor_t& d, const char* String, char** params, size_t nparams, NMEA_INFO *pGPS)
{
  if(nparams < 1) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid PLKAS sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 0...
    return FALSE;
  }
  
  double vias=StrToDouble(params[0],NULL)/10.0;

  const std::lock_guard lock(CritSec_FlightData);
  if (vias > 1) {
    double qne_altitude = QNHAltitudeToQNEAltitude(pGPS->Altitude);
    pGPS->TrueAirSpeed.update(d, TrueAirSpeed(vias, qne_altitude));
    pGPS->IndicatedAirSpeed.update(d, vias);
  }
  return FALSE;
}


BOOL NMEAParser::RMZ(DeviceDescriptor_t& d, const char* String, char **params, size_t nparams, NMEA_INFO *pGPS)
{
  if(nparams < 2) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid RMZ sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 1...
    return FALSE;
  }
  
  // We want to wait for a couple of run so we are sure we are receiving RMC GGA etc.
  if (RMZDelayed--) {
    return FALSE;
  }
  RMZDelayed=0;

  RMZAvailable = true;
  LastRMZHB = LKHearthBeats;

  double Altitude = ParseAltitude(params[0], params[1]);

  const std::lock_guard lock(CritSec_FlightData);
  UpdateBaroSource(pGPS, &d, QNEAltitudeToQNHAltitude(Altitude));

  return FALSE;
}


// TASMAN instruments support for Tasman Flight Pack model Fp10
BOOL NMEAParser::PTAS1(DeviceDescriptor_t& d, const char* String, char **params, size_t nparams, NMEA_INFO *pGPS) 
{
  if(nparams < 4) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid PTAS1 sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 3...
    return FALSE;
  }

  const std::lock_guard lock(CritSec_FlightData);

  if(*params[0] != _T('\0')) {
    const double wnet = Units::From(unKnots, (StrToDouble(params[0],NULL) - 200.0) / 10);
    UpdateVarioSource(*pGPS, d, wnet);
  }

  if(*params[2] != _T('\0')) {
    double qne_altitude = Units::From(unFeet, StrToDouble(params[2],NULL) - 2000);
    UpdateBaroSource(pGPS, &d,  QNEAltitudeToQNHAltitude(qne_altitude));

    if(*params[3] != _T('\0')) {
      const double vtas = Units::From(unKnots, StrToDouble(params[3],NULL));
      pGPS->TrueAirSpeed.update(d, vtas);
      pGPS->IndicatedAirSpeed.update(d, IndicatedAirSpeed(vtas, qne_altitude));
    }
  }

  return FALSE;
}


BOOL NMEAParser::HCHDG(DeviceDescriptor_t& d, const char* String, char** params, size_t nparams, NMEA_INFO *pGPS)
{
  if(nparams < 1) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid HCHDG sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 0...
    return FALSE;
  }
  
  double mag = StrToDouble(params[0],NULL);
  if (mag>=0 && mag<=360) {
      const std::lock_guard lock(CritSec_FlightData);
      pGPS->MagneticHeading.update(d, mag);
  }
  return TRUE;
}


