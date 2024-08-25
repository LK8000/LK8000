/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Logger.cpp,v 8.4 2010/12/11 23:59:33 root Exp root $
*/

#include "externs.h"
#include "Logger.h"
#include "InputEvents.h"
#include "Dialogs.h"
#include <ctype.h>
#include "utils/stl_utils.h"
#include "utils/stringext.h"
#include "utils/printf.h"
#include "utils/array_back_insert_iterator.h"
#include "OS/Memory.h"
#include "Util/Clamp.hpp"
#include <time.h>
#include "igc_file_writer.h"
#include <memory>
#include <deque>
#include "Baro.h"
#ifdef ANDROID
  #include "Android/AndroidFileUtils.h"
#endif

// #define DEBUG_LOGGER	1

#define A_RECORD                "A%s%c%c%c\r\n"

#define HFPLTPILOT              "HFPLTPILOT:%s\r\n"
#define HFGTYGLIDERTYPE         "HFGTYGLIDERTYPE:%s\r\n"
#define HFGIDGLIDERID           "HFGIDGLIDERID:%s\r\n"
#define HFCCLCOMPETITIONCLASS   "HFCCLCOMPETITIONCLASS:%s\r\n"
#define HFCIDCOMPETITIONID      "HFCIDCOMPETITIONID:%s\r\n"
#define HFREMARK                "HFREMARK:%s\r\n"

#define LOGGER_MANUFACTURER	"XLK"

static TCHAR NumToIGCChar(int n) {
  if (n<10) {
    return _T('1') + (n-1);
  } else {
    return _T('A') + (n-10);
  }
}


static int IGCCharToNum(TCHAR c) {
  if ((c >= _T('1')) && (c<= _T('9'))) {
    return c- _T('1') + 1;
  } else if ((c >= _T('A')) && (c<= _T('Z'))) {
    return c- _T('A') + 10;
  } else {
    return 0; // Error!
  }
}

namespace {

  struct LoggerBuffer_t {
    double latitude;
    double longitude;
    int gps_altitude; // 5 digit [0 : 99999] m
    int qnh_altitude; // 5 digit [-9999 : 99999] m
    int ground_speed; // 5 digit [0 : 99999] cm/h
    int track; // 3 digit [0 : 360]°
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
  };

  constexpr size_t max_buffer = 60;
  std::deque<LoggerBuffer_t> LoggerBuffer;

  // singleton instance of igc file writer
  //  created by StartLogger
  //  deleted by StopLogger
  std::unique_ptr<igc_file_writer> igc_writer_ptr;

  using asset_id_t = std::array<char, 3>;

  template<size_t size>
  bool IGCWriteRecord(const char (&szIn)[size]) {
    return igc_writer_ptr && igc_writer_ptr->append(szIn);
  }

}


void StartLogger() {
  if (TaskModified) {
    SaveDefaultTask();
  }
  LoggerActive = true;
}

void StopLogger() {
  LoggerActive = false;
}

static void LogPointToBuffer(const LoggerBuffer_t& point) {
  if (LoggerBuffer.size() >= max_buffer) {
    LoggerBuffer.pop_front();
  }
  LoggerBuffer.push_back(point);
}


static void LogPointToFile(const LoggerBuffer_t& point) {

  char NoS = (point.latitude > 0) ? 'N' : 'S';
  double latitude = std::abs(point.latitude);
  int DegLat = latitude;
  int MinLat = (latitude - DegLat) * 60. * 10000.;

  char EoW = (point.longitude > 0) ? 'E' : 'W';
  double longitude = std::abs(point.longitude);
  int DegLon = longitude;
  int MinLon = (longitude - DegLon) * 60. * 10000.;

  char szBRecord[128];

  /*
   * NoS/EoW and 6th digit of lat/lon was swaped after printf
   */
  sprintf(szBRecord, 
            "B%02d%02d%02d" 
            "%02d%06d"
            "%03d%06d"
            "A%05d%05d"
            "%c%c"
            "%05d%03d\r\n",
            point.hour, point.minute, point.second,
            DegLat, MinLat,
            DegLon, MinLon,
            point.qnh_altitude, point.gps_altitude,
            NoS, EoW,
            point.ground_speed, point.track);

  // move LAD and N/S to right place
  std::swap(szBRecord[35], szBRecord[14]);
  // move LOD and E/W to right place
  std::swap(szBRecord[36], szBRecord[23]);

  IGCWriteRecord(szBRecord);
}


static bool IsAlphaNum (TCHAR c) {
  return ( ((c >= _T('A')) && (c <= _T('Z')))
        || ((c >= _T('a')) && (c <= _T('z')))
        || ((c >= _T('0')) && (c <= _T('9'))));
}

static void GetShortFileName(TCHAR* Filename, size_t size, const LoggerBuffer_t& point, const asset_id_t& asset, unsigned idx) {
  /*
   * Short file name style: YMDCXXXF.IGC
   *    Y = Year; value 0 to 9, cycling every 10 years
   *    M = Month; value 1 to 9 then A for 10, B=11, C=12.
   *    D = Day; value 1 to 9 then A=10 through to V=31.
   *    C = manufacturer's IGC code letter (see table below)
   *    XXX = unique FR Serial Number (S/N); 3 alphanumeric characters
   *    F = Flight number of the day; 1 to 9 then, if needed, A=10 through to Z=35
   */

  auto out = array_back_inserter(Filename, size);

  out = NumToIGCChar(point.year % 10);
  out = NumToIGCChar(point.month);
  out = NumToIGCChar(point.day);
  out = _T('X');
  for (size_t i = 0; i < 3; ++i) {
    out = asset[i];
  }
  out = NumToIGCChar(idx);
  for (TCHAR c : _T(".IGC")) {
    out = c;
  }
}


static void GetLongFileName(TCHAR* Filename, size_t size, const LoggerBuffer_t& point, const asset_id_t& asset, unsigned idx) {
  /* Long file name style.
    *    This uses a full set of characters in each field, a hyphen separating each field,
    *    the field order being the same as in the short file name. For instance, if the
    *    short name for a file from manufacturer X is 36HXABC2.IGC, the equivalent long
    *    file name is 2003-06-17-XXX-ABC-02.IGC. Long file names may be generated by software
    *    that is compatible with long file names, although the DOS versions of the DATA, CONV
    *    and VALI programs must continue to generate and use short file names. (AL3)
    */
  _sntprintf(Filename, size,
             _T("%04d-%02d-%02d-" LOGGER_MANUFACTURER "-%c%c%c-%02d.IGC"),
             point.year, point.month, point.day, asset[0], asset[1], asset[2], idx);
}


static asset_id_t GetAssetCode() {
  asset_id_t asset_id;

  if (_tcslen(PilotName_Config) > 2) {
    asset_id[0] = IsAlphaNum(PilotName_Config[0]) ? _totupper(PilotName_Config[0]) : 'A';
    asset_id[1] = IsAlphaNum(PilotName_Config[1]) ? _totupper(PilotName_Config[1]) : 'A';
  } else {
    asset_id[0] = 'D';
    asset_id[1] = 'U';
  }
  if (_tcslen(AircraftType_Config) > 0) {
    asset_id[2] = IsAlphaNum(AircraftType_Config[0]) ? _totupper(AircraftType_Config[0]) : 'A';
  } else {
    asset_id[2] = 'M';
  }
  return asset_id;
}

//
// Paolo+Durval: feed external headers to LK for PNAdump software
//
#define MAXHLINE 100
#define EXTHFILE	"COMPE.CNF"
//#define DEBUGHFILE	1
static void AdditionalHeaders() {

    TCHAR pathfilename[MAX_PATH + 1];
    LocalPath(pathfilename, _T(LKD_LOGS), _T(EXTHFILE));

    if (!lk::filesystem::exist(pathfilename)) {
#if DEBUGHFILE
        StartupStore(_T("... No additional headers file <%s>\n"), pathfilename);
#endif
        return;
    }

#if DEBUGHFILE
    StartupStore(_T("... HFILE <%s> FOUND\n"), pathfilename);
#endif

    FILE* stream = _tfopen(pathfilename, _T("rb"));
    if (!stream) {
        StartupStore(_T("... ERROR, extHFILE <%s> not found!%s"), pathfilename, NEWLINE);
        return;
    }


    char tmpString[MAXHLINE + 1];
    char tmps[MAXHLINE + 1 + std::size(HFREMARK)];
    tmpString[MAXHLINE] = '\0';

    while(size_t nbRead = fread(tmpString, sizeof(tmpString[0]), std::size(tmpString) - 1U, stream)) {
        tmpString[nbRead] = '\0';
        char* pTmp = strpbrk(tmpString, "\r\n");
        while(pTmp && (((*pTmp) == '\r') || ((*pTmp) == '\n'))) {
            (*pTmp++) = '\0';
        }
        fseek(stream, -1 * (&tmpString[nbRead] - pTmp) ,SEEK_CUR);

        size_t len = strlen(tmpString);
        if ((len < 2) || (tmpString[0] != '$')) {
            continue;
        }

        lk::snprintf(tmps, HFREMARK, &tmpString[1]);
        IGCWriteRecord(tmps);
    }
    fclose(stream);
}

static void LoggerHeader(const LoggerBuffer_t& point, const asset_id_t& asset) {
  char temp[300];

  // Flight recorder ID number MUST go first..
  sprintf(temp, A_RECORD, LOGGER_MANUFACTURER, asset[0], asset[1], asset[2]);
  IGCWriteRecord(temp);

  sprintf(temp,"HFDTE%02d%02d%02d\r\n", point.day, point.month, point.year % 100);
  IGCWriteRecord(temp);

  char ascii_tmp[100];

  // Example: Hanna.Reitsch
  to_usascii(PilotName_Config, ascii_tmp);
  sprintf(temp,HFPLTPILOT, ascii_tmp);
  IGCWriteRecord(temp);

  // Example: DG-300
  to_usascii(AircraftType_Config, ascii_tmp);
  sprintf(temp,HFGTYGLIDERTYPE, ascii_tmp);
  IGCWriteRecord(temp);

  // Example: D-7176
  to_usascii(AircraftRego_Config, ascii_tmp);
  sprintf(temp,HFGIDGLIDERID, ascii_tmp);
  IGCWriteRecord(temp);

  // 110117 TOCHECK: maybe a 8 char limit is needed.
  to_usascii(CompetitionClass_Config, ascii_tmp);
  sprintf(temp,HFCCLCOMPETITIONCLASS, ascii_tmp);
  IGCWriteRecord(temp);

  to_usascii(CompetitionID_Config, ascii_tmp);
  sprintf(temp,HFCIDCOMPETITIONID, ascii_tmp);
  IGCWriteRecord(temp);

#ifdef LKCOMPETITION
  constexpr const char* competition_indicator = "C";
#else
  constexpr const char* competition_indicator = "";
#endif

#if defined(PPC2002)
  constexpr const char* model_name = " PPC2002";
#elif defined(PPC2003)
  constexpr const char* model_name = " PPC2003";
#elif defined(PNA)
  char model_name[200];
  to_usascii(GlobalModelName, ascii_tmp);
  sprintf(model_name," PNA %s", ascii_tmp);
#elif defined(ANDROID)
  // TODO : get real model name : native_view->GetProduct()
  constexpr const char* model_name = " ANDROID";
#elif defined(KOBO)
  constexpr const char* model_name = " KOBO";
#elif defined(WINDOWSPC)
  constexpr const char* model_name = " WINDOWSPC";
#elif defined(__linux__)
  constexpr const char* model_name = " LINUX";
#else
  constexpr const char* model_name = " UNKNOWN";
#endif

  sprintf(temp, "HFFTYFRTYPE:" LKFORK "%s%s\r\n", competition_indicator, model_name);
  IGCWriteRecord(temp);

#ifdef LKCOMPETITION
  IGCWriteRecord("HFRFWFIRMWAREVERSION:" LKVERSION "." LKRELEASE "." COMPETITION "\r\n");
#else
  IGCWriteRecord("HFRFWFIRMWAREVERSION:" LKVERSION "." LKRELEASE "\r\n");
#endif

  IGCWriteRecord("HFALGALTGPS:GEO\r\n");
  if (BaroAltitudeAvailable(GPS_INFO)) {
    IGCWriteRecord("HFALPALTPRESSURE:ISA\r\n");
  }

  IGCWriteRecord("HFDTM100GPSDATUM:WGS-84\r\n");

  if (GPSAltitudeOffset != 0) {
     lk::snprintf(temp,"HFGPSALTITUDEOFFSET:%+.0f\r\n", GPSAltitudeOffset);
     IGCWriteRecord(temp);
  }

  AdditionalHeaders();


  // I Record
  // LAD : The last places of decimal minutes of latitude, where latitude is recorded to a greater precision than the
  //       three decimal minutes that are in the main body of the B record. The fourth and any further decimal places of minutes
  //       are recorded as an extension to the B record, their position in each B record line being specified in the I record. (AL8)

  // LOD : The last places of decimal minutes of longitude, where longitude is recorded to a greater precision than the 
  //       three decimal minutes that are in the main body of the Brecord. The fourth and any further decimal places of minutes 
  //       are recorded as an extension to the B record, their position in each B record line being specified in the I record.(AL8)

  // GSP : Groundspeed, five numbers in centimetres per hour(AL8)

  // TRT : Track True. Three numbers based on degrees clockwise from 000 for north(AL8)

  // TODO : add if available :
  // VAR : Uncompensated variometer (non-total energy) vertical speed in metres and decimal metres. If negative, use negative 
  //       sign before the numbers.(AL8)

  IGCWriteRecord("I" "04"
                 "3636" "LAD"
                 "3737" "LOD"
                 "3842" "GSP"
                 "4345" "TRT"
                 "\r\n");
}

static void AddDeclaration(double Latitude, double Longitude, const TCHAR *ID) {

  char IDString[MAX_PATH];
  to_usascii(ID, IDString);

  char NoS = (Latitude > 0) ? 'N' : 'S';
  double latitude = std::abs(Latitude);
  int DegLat = latitude;
  int MinLat = (latitude - DegLat) * 60. * 1000.;

  char EoW = (Longitude > 0) ? 'E' : 'W';
  double longitude = std::abs(Longitude);
  int DegLon = longitude;
  int MinLon = (longitude - DegLon) * 60. * 1000.;

  char szCRecord[500];
  sprintf(szCRecord,"C%02d%05d%c%03d%05d%c%s\r\n",
	  DegLat, MinLat, NoS, DegLon, MinLon, EoW, IDString);

  IGCWriteRecord(szCRecord);
}

static void StartDeclaration(int ntp) {

  char temp[100];

  const auto& FirstPoint = LoggerBuffer.front();

  // JMW added task start declaration line

  // LGCSTKF013945TAKEOFF DETECTED

  // IGC GNSS specification 3.6.1
  sprintf(temp,
      "C%02d%02d%02d%02d%02d%02d0000000000%02d\r\n",
      // DD  MM  YY  HH  MM  SS  DD  MM  YY IIII TT
      FirstPoint.day,
      FirstPoint.month,
      FirstPoint.year % 100,
      FirstPoint.hour,
      FirstPoint.minute,
      FirstPoint.second,
      ntp-2);

  IGCWriteRecord(temp);
  // takeoff line
  // IGC GNSS specification 3.6.3

  // Use homewaypoint as default takeoff and landing position. Better than an empty field!
  if (ValidWayPoint(HomeWaypoint)) {
    const WAYPOINT& wp = WayPointList[HomeWaypoint];
    AddDeclaration(wp.Latitude, wp.Longitude, wp.Name);
  } else {
    // TODO bug: this is causing problems with some analysis software
    // maybe it's because the date and location fields are bogus
    IGCWriteRecord("C0000000N00000000ETAKEOFF\r\n");
  }
}

static void EndDeclaration() {
  // Use homewaypoint as default takeoff and landing position. Better than an empty field!
  if (ValidWayPoint(HomeWaypoint)) {
    const WAYPOINT& wp = WayPointList[HomeWaypoint];
    AddDeclaration(wp.Latitude, wp.Longitude, wp.Name);
  } else {
    // TODO bug: this is causing problems with some analysis software
    // maybe it's because the date and location fields are bogus
    IGCWriteRecord("C0000000N00000000ELANDING\r\n");
  }
}

static void LoggerTask() {
  LockTaskData();

  int ntp=0;
  for ( ; ValidTaskPointFast(ntp); ++ntp);

  StartDeclaration(ntp);
  for (unsigned i = 0; ValidTaskPointFast(i); ++i) {
    const WAYPOINT& wp = WayPointList[Task[i].Index];
    AddDeclaration(wp.Latitude, wp.Longitude, wp.Name);
  }
  EndDeclaration();

  UnlockTaskData();
}

static void internal_StartLogger() {
  const asset_id_t asset_id = GetAssetCode();
  const LoggerBuffer_t& first_point = LoggerBuffer.front();

  TCHAR szLoggerFilePath[MAX_PATH + 1];
  LocalPath(szLoggerFilePath, TEXT(LKD_LOGS));

  TCHAR* filename = szLoggerFilePath + _tcslen(szLoggerFilePath);
  *(filename++) = (*DIRSEP);
  size_t filename_size = std::distance(filename, std::end(szLoggerFilePath));

  for(int i = 1; i < 99; i++) {
    if (LoggerShortName) {
      GetShortFileName(filename, filename_size, first_point, asset_id, i);
    } else {
      GetLongFileName(filename, filename_size, first_point, asset_id, i);
    }

    if(!lk::filesystem::exist(szLoggerFilePath)) {
      break;
    }
  }

  igc_writer_ptr = std::make_unique<igc_file_writer>(szLoggerFilePath, LoggerGActive());

  LoggerHeader(first_point, asset_id);
  LoggerTask();

  for (const auto & point : LoggerBuffer) {
    LogPointToFile(point);
  }
  LoggerBuffer = std::deque<LoggerBuffer_t>(); //used instead of clear to deallocate.

#ifdef ANDROID
  // required to make file available to Android StorageManager without reboot device.
  AndroidFileUtils::UpdateMediaStore(szLoggerFilePath);
#endif

  StartupStore(_T(". Logger Started %s  File <%s>"), WhatTimeIsIt(), szLoggerFilePath);
}

static void internal_StopLogger() {
  igc_writer_ptr = nullptr;
  LoggerBuffer.clear();
}

void LogPoint(const NMEA_INFO& info) {

  if (info.NAVWarning) {
    // don't log invalid fix
    return;
  }

  // pending rounding error from millisecond timefix in RMC sentence?
  if (info.Second >= 60 || info.Second < 0) {
    TestLog(_T("... WRONG TIMEFIX FOR LOGGER, seconds=%d, fix skipped"), info.Second);
    return;
  }

  if(LoggerActive && !igc_writer_ptr) {
    // start Logger requested
    if(!LoggerBuffer.empty()) {
      // only start logger after first valid fix
      internal_StartLogger();
    }
  }

  if(!LoggerActive && igc_writer_ptr) {
    // stop Logger requested
    internal_StopLogger();
  }

  // BaroAltitude in this case is a QNE altitude (aka pressure altitude)
  // Some few instruments are sending only a cooked QNH altitude, without the relative QNH.
  // (If we had QNH in that case, we would save real QNE altitude in GPS_INFO.BaroAltitude)
  // There is nothing we can do about it, in these few cases: we shall log a QNH altitude instead
  // of QNE altitude, which is what we have been doing up to v4 in any case. It cant be worst.
  // In all other cases, the pressure altitude will be saved, and out IGC logger replay is converting it
  // to the desired QNH altitude back.

  LoggerBuffer_t point = {
    info.Latitude, 
    info.Longitude,
    Clamp<int>(Units::To(unMeter, (GPSAltitudeOffset == 0) ? info.Altitude : 0), 0, 99999),
    Clamp<int>(Units::To(unMeter, BaroAltitudeAvailable(info) ? QNHAltitudeToQNEAltitude(info.BaroAltitude) : 0) , -9999, 99999),
    Clamp<int>(Units::To(unKiloMeterPerHour, info.Speed) * 100, 0, 99999),
    static_cast<int>(AngleLimit360(info.TrackBearing)),
    info.Year, info.Month, info.Day,
    info.Hour, info.Minute, info.Second
  };

  if (igc_writer_ptr) {
    LogPointToFile(point);
  } else {
    LogPointToBuffer(point);
  }
}

bool DeclaredToDevice = false;


static bool LoggerDeclare(DeviceDescriptor_t* dev, const Declaration_t *decl)
{
  if (!devIsLogger(*dev))
  	return FALSE;

  // If a Flarm is reset while we are here, then it will come up with isFlarm set to false,
  // and task declaration will fail. The solution is to let devices have a flag for "HaveFlarm".
  LKDoNotResetComms=true;

  // LKTOKEN  _@M221_ = "Declare Task?"
  if (MessageBoxX(MsgToken<221>(),
	dev->Name, mbYesNo) == IdYes) {

	const unsigned ERROR_BUFFER_LEN = 64;
	TCHAR errorBuffer[ERROR_BUFFER_LEN] = { '\0' };

	if (devDeclare(dev, decl, ERROR_BUFFER_LEN, errorBuffer)) {
		// LKTOKEN  _@M686_ = "Task Declared!"
		MessageBoxX(MsgToken<686>(),
			dev->Name, mbOk);

		DeclaredToDevice = true;

	} else {

		TCHAR buffer[2*ERROR_BUFFER_LEN];

		if(errorBuffer[0] == '\0') {
			// LKTOKEN  _@M1410_ = "Unknown error"
			lk::snprintf(errorBuffer, _T("%s"), MsgToken<1410>());
		} else {
			// do it just to be sure
			errorBuffer[ERROR_BUFFER_LEN - 1] = '\0';
		}
    StartupStore(_T("Error! Task NOT declared : %s"), errorBuffer);

		// LKTOKEN  _@M265_ = "Error! Task NOT declared!"
		lk::snprintf(buffer, _T("%s\n%s"), MsgToken<265>(), errorBuffer);
		MessageBoxX(buffer, dev->Name, mbOk);

		DeclaredToDevice = false;
	}
  }
  LKDoNotResetComms=false;
  return TRUE;
}


void LoggerDeviceDeclare() {
  bool found_logger = false;
  Declaration_t Decl;
  int i;

  #if 0
  if (CALCULATED_INFO.Flying) {
    // LKTOKEN  _@M1423_ = "Forbidden during flight!"
    MessageBoxX(MsgToken<1423>(), _T(""), mbOk);
    return;
  }
  #endif

  _tcscpy(Decl.PilotName, PilotName_Config);		// max 64
  _tcscpy(Decl.AircraftType,AircraftType_Config);	// max 32
  _tcscpy(Decl.AircraftRego,AircraftRego_Config);	// max 32
  _tcscpy(Decl.CompetitionClass,CompetitionClass_Config);   //
  _tcscpy(Decl.CompetitionID,CompetitionID_Config);	// max 32

  for (i = 0; i < MAXTASKPOINTS; i++) {
    if (Task[i].Index == -1)
      break;
    Decl.waypoint[i] = &WayPointList[Task[i].Index];
  }
  Decl.num_waypoints = i;

  DeclaredToDevice = false;

  for(auto& dev : DeviceList) {
    if (LoggerDeclare(&dev, &Decl)) {
      found_logger = true;
    }  
  }

  if (!found_logger) {
	// LKTOKEN  _@M474_ = "No logger connected" 
    MessageBoxX(MsgToken<474>(), _T(""), mbOk);
    DeclaredToDevice = true; // testing only
  }

}


bool CheckDeclaration(void) {
  if (!DeclaredToDevice) {
    return true;
  } else {
    if(MessageBoxX(
	// LKTOKEN  _@M492_ = "OK to invalidate declaration?"
		   MsgToken<492>(),
	// LKTOKEN  _@M694_ = "Task declared"
		   MsgToken<694>(),
		   mbYesNo) == IdYes){
      DeclaredToDevice = false;
      return true;
    } else {
      return false;
    }
  }
}


static time_t LogFileDate(const TCHAR* filename) {
  TCHAR asset[MAX_PATH+1];
  struct tm st ={};
  unsigned short year, month, day, num;
  int matches;
  // scan for long filename
  matches = _stscanf(filename,
                    TEXT("%hu-%hu-%hu-%7s-%hu.IGC"),
                    &year,
                    &month,
                    &day,
                    asset,
                    &num);
  if (matches==5) {
    st.tm_year = year - 1900;
    st.tm_mon = month;
    st.tm_mday = day;
    st.tm_hour = num;
    st.tm_min = 0;
    st.tm_sec = 0;
    return mktime(&st);
  }

  TCHAR cyear, cmonth, cday, cflight;
  // scan for short filename
  matches = _stscanf(filename,
		     TEXT("%c%c%c%4s%c.IGC"),
		     &cyear,
		     &cmonth,
		     &cday,
		     asset,
		     &cflight);
  if (matches==5) {
    int iyear = (int)GPS_INFO.Year;
    int syear = iyear % 10;
    int yearzero = iyear - syear;
    int yearthis = IGCCharToNum(cyear) + yearzero;
    if (yearthis > iyear) {
      yearthis -= 10;
    }
    st.tm_year = yearthis - 1900;
    st.tm_mon = IGCCharToNum(cmonth);
    st.tm_mday = IGCCharToNum(cday);
    st.tm_hour = IGCCharToNum(cflight);
    st.tm_min = 0;
    st.tm_sec = 0;
    return mktime(&st);
    /*
      YMDCXXXF.IGC
      Y: Year, 0 to 9 cycling every 10 years
      M: Month, 1 to 9 then A for 10, B=11, C=12
      D: Day, 1 to 9 then A for 10, B=....
      C: Manuf. code = X
      XXX: Logger ID Alphanum
      F: Flight of day, 1 to 9 then A through Z
    */
  }
  return 0;
}


static bool LogFileIsOlder(const TCHAR *oldestname, const TCHAR *thisname) {
  time_t ftold = LogFileDate(oldestname);
  time_t ftnew = LogFileDate(thisname);
  return ftold > ftnew;
}


static bool DeleteOldIGCFile(TCHAR *pathname) {
  TCHAR searchpath[MAX_PATH+1];
  TCHAR fullname[MAX_PATH+1];
  lk::snprintf(searchpath, TEXT("%s*.igc"),pathname);
    tstring oldestname;
    lk::filesystem::directory_iterator It(searchpath);
    if (It) {
        oldestname = It.getName();
        while (++It) {
            if (LogFileIsOlder(oldestname.c_str(), It.getName())) {
                // we have a new oldest name
                oldestname = It.getName();
            }
        }
    }

  // now, delete the file...
  lk::snprintf(fullname, TEXT("%s%s"),pathname,oldestname.c_str());
  TestLog(_T("... DeleteOldIGCFile <%s> ..."), fullname);
  lk::filesystem::deleteFile(fullname);
  TestLog(_T("... done DeleteOldIGCFile"));
  return true; // did delete one
}


#define LOGGER_MINFREESTORAGE (250+MINFREESTORAGE)
// JMW note: we want to clear up enough space to save the persistent
// data (85 kb approx) and a new log file

#ifdef DEBUG_IGCFILENAME
TCHAR testtext1[] = TEXT("2007-11-05-XXX-AAA-01.IGC");
TCHAR testtext2[] = TEXT("2007-11-05-XXX-AAA-02.IGC");
TCHAR testtext3[] = TEXT("3BOA1VX2.IGC");
TCHAR testtext4[] = TEXT("5BDX7B31.IGC");
TCHAR testtext5[] = TEXT("3BOA1VX2.IGC");
TCHAR testtext6[] = TEXT("9BDX7B31.IGC");
TCHAR testtext7[] = TEXT("2008-01-05-XXX-AAA-01.IGC");
#endif

bool LoggerClearFreeSpace(void) {
  bool found = true;
  size_t kbfree=0;
  TCHAR pathname[MAX_PATH+1];
  TCHAR subpathname[MAX_PATH+1];
  int numtries = 0;

  LocalPath(pathname);
  LocalPath(subpathname,TEXT(LKD_LOGS));

#ifdef DEBUG_IGCFILENAME
  bool retval;
  retval = LogFileIsOlder(testtext1,
                          testtext2);
  retval = LogFileIsOlder(testtext1,
                          testtext3);
  retval = LogFileIsOlder(testtext4,
                          testtext5);
  retval = LogFileIsOlder(testtext6,
                          testtext7);
#endif

  while (found && ((kbfree = FindFreeSpace(pathname))<LOGGER_MINFREESTORAGE)
	 && (numtries++ <100)) {
    /* JMW asking for deleting old files is disabled now --- system
       automatically deletes old files as required
    */

    // search for IGC files, and delete the oldest one
    found = DeleteOldIGCFile(pathname);
    if (!found) {
      found = DeleteOldIGCFile(subpathname);
    }
  }
  if (kbfree>=LOGGER_MINFREESTORAGE) {
    TestLog(_T("... LoggerFreeSpace returned: true"));
    return true;
  } else {
    TestLog(_T("--- LoggerFreeSpace returned: false"));
    return false;
  }
}

bool LoggerGActive() {
#if defined(YDEBUG) || defined(DEBUG_LOGGER)
    return true;
#else
    return (!SIMMODE);
#endif
}
