/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Logger.cpp,v 8.4 2010/12/11 23:59:33 root Exp root $
*/

#include "externs.h"
#include "Logger.h"
#include "InputEvents.h"
#include "Dialogs.h"
#include "TraceThread.h"
#include <ctype.h>
#include "utils/stl_utils.h"
#include "utils/stringext.h"
#include "OS/Memory.h"
#include "Util/Clamp.hpp"
#include <time.h>
#include "igc_file_writer.h"
#include <memory>
// #define DEBUG_LOGGER	1

//
// v6 NOTICE: WE DO NOT USE LOGFRECORD AT ALL
//

#ifdef _UNICODE
    #define A_RECORD                "A%s%C%C%C\r\n"
    #define HFPLTPILOT              "HFPLTPILOT:%S\r\n"
    #define HFGTYGLIDERTYPE         "HFGTYGLIDERTYPE:%S\r\n"
    #define HFGIDGLIDERID           "HFGIDGLIDERID:%S\r\n"
    #define HFCCLCOMPETITIONCLASS   "HFCCLCOMPETITIONCLASS:%S\r\n"
    #define HFCIDCOMPETITIONID      "HFCIDCOMPETITIONID:%S\r\n"
#else
    #define A_RECORD                "A%s%c%c%c\r\n"
    #define HFPLTPILOT              "HFPLTPILOT:%s\r\n"
    #define HFGTYGLIDERTYPE         "HFGTYGLIDERTYPE:%s\r\n"
    #define HFGIDGLIDERID           "HFGIDGLIDERID:%s\r\n"
    #define HFCCLCOMPETITIONCLASS   "HFCCLCOMPETITIONCLASS:%s\r\n"
    #define HFCIDCOMPETITIONID      "HFCIDCOMPETITIONID:%s\r\n"
#endif
#define HFREMARK                "HFREMARK:%s\r\n"

#define LOGGER_MANUFACTURER	"XLK"

extern NMEA_INFO GPS_INFO;

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

static TCHAR szFLoggerFileName[MAX_PATH+1] = TEXT("\0"); // final IGC name

#if LOGFRECORD
static double FRecordLastTime = 0;
static char szLastFRecord[MAX_IGC_BUFF];
#endif

#if LOGFRECORD
void SetFRecordLastTime(double dTime)
{ FRecordLastTime=dTime; }

double GetFRecordLastTime(void)
{ return FRecordLastTime; }

void ResetFRecord_Internal(void)
{
    for (int iFirst = 0; iFirst < MAX_IGC_BUFF; iFirst++)
      szLastFRecord[iFirst]=0;
}
void ResetFRecord(void)
{
  SetFRecordLastTime(0);
  ResetFRecord_Internal();
}
#endif

int EW_count = 0;
int NumLoggerBuffered = 0;

#define MAX_LOGGER_BUFFER 60

typedef struct LoggerBuffer {
  double Latitude;
  double Longitude;
  double Altitude;
  double BaroAltitude;
  short Day;
  short Month;
  short Year;
  short Hour;
  short Minute;
  short Second;
  #if LOGFRECORD
  int SatelliteIDs[MAXSATELLITES];
  #endif
} LoggerBuffer_T;

static LoggerBuffer_T FirstPoint;
static LoggerBuffer_T LoggerBuffer[MAX_LOGGER_BUFFER];


namespace {

  // singleton instance of igc file writer
  //  created by StartLogger
  //  deleted by StopLogger
  std::unique_ptr<igc_file_writer> igc_writer_ptr;

  template<size_t size>
  bool IGCWriteRecord(const char (&szIn)[size]) {
    return igc_writer_ptr && igc_writer_ptr->append(szIn);
  }

}

void StopLogger(void) {
    igc_writer_ptr.reset();
    LoggerActive = false;
    NumLoggerBuffered = 0;
}

// BaroAltitude in this case is a QNE altitude (aka pressure altitude)
// Some few instruments are sending only a cooked QNH altitude, without the relative QNH.
// (If we had QNH in that case, we would save real QNE altitude in GPS_INFO.BaroAltitude)
// There is nothing we can do about it, in these few cases: we shall log a QNH altitude instead
// of QNE altitude, which is what we have been doing up to v4 in any case. It cant be worst.
// In all other cases, the pressure altitude will be saved, and out IGC logger replay is converting it
// to the desired QNH altitude back.
#if LOGFRECORD
static void LogPointToBuffer(double Latitude, double Longitude, double Altitude,
                      double BaroAltitude, short Hour, short Minute, short Second,
                      int SatelliteIDs[]) {
#else
static void LogPointToBuffer(double Latitude, double Longitude, double Altitude,
                      double BaroAltitude, short Hour, short Minute, short Second) {
#endif

  LKASSERT(NumLoggerBuffered<=MAX_LOGGER_BUFFER);
  if (NumLoggerBuffered== MAX_LOGGER_BUFFER) {
    for (int i= 0; i< NumLoggerBuffered-1; i++) {
      LKASSERT((i+1)<MAX_LOGGER_BUFFER);
      LoggerBuffer[i]= LoggerBuffer[i+1];
    }
  } else {
    NumLoggerBuffered++;
  }
  LKASSERT((NumLoggerBuffered-1)>=0);
  LoggerBuffer[NumLoggerBuffered-1].Latitude = Latitude;
  LoggerBuffer[NumLoggerBuffered-1].Longitude = Longitude;
  LoggerBuffer[NumLoggerBuffered-1].Altitude = Altitude;
  LoggerBuffer[NumLoggerBuffered-1].BaroAltitude = BaroAltitude;
  LoggerBuffer[NumLoggerBuffered-1].Hour = Hour;
  LoggerBuffer[NumLoggerBuffered-1].Minute = Minute;
  LoggerBuffer[NumLoggerBuffered-1].Second = Second;
  LoggerBuffer[NumLoggerBuffered-1].Year = GPS_INFO.Year;
  LoggerBuffer[NumLoggerBuffered-1].Month = GPS_INFO.Month;
  LoggerBuffer[NumLoggerBuffered-1].Day = GPS_INFO.Day;

  #if LOGFRECORD
  for (int iSat=0; iSat < MAXSATELLITES; iSat++)
    LoggerBuffer[NumLoggerBuffered-1].SatelliteIDs[iSat]=SatelliteIDs[iSat];
  #endif

  // This is the first point that will be output to file.
  // Declaration must happen before this, so must save this time.
  FirstPoint = LoggerBuffer[0];
}


static void LogPointToFile(double Latitude, double Longitude, double Altitude,
                    double BaroAltitude, short Hour, short Minute, short Second)
{
  char szBRecord[500];

  int DegLat, DegLon;
  double MinLat, MinLon;
  char NoS, EoW;

  // pending rounding error from millisecond timefix in RMC sentence?
  if (Second>=60||Second<0) {
	#if TESTBENCH
	StartupStore(_T("... WRONG TIMEFIX FOR LOGGER, seconds=%d, fix skipped\n"),Second);
	#endif
	return;
  }

  // v5: very old bug since v2: Netherlands can have negative altitudes!
  //if ((Altitude<0) && (BaroAltitude<0)) return;
  //Altitude = max(0.0,Altitude);
  //BaroAltitude = max(0.0,BaroAltitude);

  DegLat = (int)Latitude;
  MinLat = Latitude - DegLat;
  NoS = 'N';
  if((MinLat<0) || ((MinLat==0) && (DegLat<0)))
    {
      NoS = 'S';
      DegLat *= -1; MinLat *= -1;
    }
  MinLat *= 60;
  MinLat *= 1000;

  DegLon = (int)Longitude ;
  MinLon = Longitude  - DegLon;
  EoW = 'E';
  if((MinLon<0) || ((MinLon==0) && (DegLon<0)))
    {
      EoW = 'W';
      DegLon *= -1; MinLon *= -1;
    }
  MinLon *=60;
  MinLon *= 1000;

  sprintf(szBRecord,"B%02d%02d%02d%02d%05.0f%c%03d%05.0f%cA%05d%05d\r\n",
          Hour, Minute, Second,
          DegLat, MinLat, NoS, DegLon, MinLon, EoW,
          (int)BaroAltitude,Clamp<int>(Altitude,0,99999));

  IGCWriteRecord(szBRecord);
}


void LogPoint(double Latitude, double Longitude, double Altitude, double BaroAltitude,
              int iHour, int iMin, int iSec) {
  if (!LoggerActive) {
    if (!GPS_INFO.NAVWarning) {
      #if LOGFRECORD
      LogPointToBuffer(Latitude, Longitude, Altitude, BaroAltitude,
                       iHour, iMin, iSec,
                       GPS_INFO.SatelliteIDs); // no problem for wrong sats in case
      #else
      LogPointToBuffer(Latitude, Longitude, GPSAltitudeOffset==0?Altitude:0, BaroAltitude, iHour, iMin, iSec);
      #endif
    }
  } else if (NumLoggerBuffered && !GPS_INFO.NAVWarning) {

    #if LOGFRECORD
    LogFRecordToFile(LoggerBuffer[0].SatelliteIDs,  // write FRec before cached BRecs
                   LoggerBuffer[0].Hour,
                   LoggerBuffer[0].Minute,
                   LoggerBuffer[0].Second,
                   true);
    #endif

    LKASSERT(NumLoggerBuffered<=MAX_LOGGER_BUFFER); // because we check i<
    for (int i=0; i<NumLoggerBuffered; i++) {
      LogPointToFile(LoggerBuffer[i].Latitude,
                     LoggerBuffer[i].Longitude,
                     LoggerBuffer[i].Altitude,
                     LoggerBuffer[i].BaroAltitude,
                     LoggerBuffer[i].Hour,
                     LoggerBuffer[i].Minute,
                     LoggerBuffer[i].Second);
    }
    NumLoggerBuffered = 0;
  }
  if (LoggerActive && !GPS_INFO.NAVWarning) {
    LogPointToFile(Latitude, Longitude, GPSAltitudeOffset==0?Altitude:0, BaroAltitude,
                   iHour, iMin, iSec);
  }
}

//
// v6 NOTICE: WE DO NOT USE LOGFRECORD AT ALL
//
#if LOGFRECORD
bool LogFRecordToFile(int SatelliteIDs[], short Hour, short Minute, short Second, bool bAlways)
{ // bAlways forces write when completing header for restart
  // only writes record if constallation has changed unless bAlways set
  #if (TESTBENCH && DEBUG_LOGGER)
  #else
  if (SIMMODE) return true;
  #endif
  char szFRecord[MAX_IGC_BUFF];
  static bool bFirst = true;
  int eof=0;
  int iNumberSatellites=0;
  bool bRetVal = false;

  if (bFirst)
  {
    bFirst = false;
    ResetFRecord_Internal();
  }


  sprintf(szFRecord,"F%02d%02d%02d", Hour, Minute, Second);
  eof=7;

  for (int i=0; i < MAXSATELLITES; i++)
  {
    if (SatelliteIDs[i] > 0)
    {
      sprintf(szFRecord+eof, "%02d",SatelliteIDs[i]);
      eof +=2;
      iNumberSatellites++;
    }
  }
  sprintf(szFRecord+ eof,"\r\n");

  // only write F Record if it has changed since last time
  // check every 4.5 minutes to see if it's changed.  Transient changes are not tracked.
  if (!bAlways
        && strcmp(szFRecord + 7, szLastFRecord + 7) == 0
        && strlen(szFRecord) == strlen(szLastFRecord) )
  { // constellation has not changed
      if (iNumberSatellites >=3)
        bRetVal=true;  // if the last FRecord had 3+ sats, then return true
                      //  and this causes 5-minute counter to reset
      else
        bRetVal=false;  // non-2d fix, don't reset 5-minute counter so
                        // we keep looking for changed constellations
  }
  else
  { // constellation has changed
    if (IGCWriteRecord(szFRecord))
    {
      strcpy(szLastFRecord, szFRecord);
      if (iNumberSatellites >=3)
        bRetVal=true;  // if we log an FRecord with a 3+ sats, then return true
                      //  and this causes 5-minute counter to reset
      else
        bRetVal=false;  // non-2d fix, log it, and don't reset 5-minute counter so
                        // we keep looking for changed constellations
    }
    else
    {  // IGCwrite failed
      bRetVal = false;
    }

  }
  return bRetVal;

}


bool LogFRecord(int SatelliteIDs[], bool bAlways )
{
  if (LoggerActive || bAlways)
    {
      return LogFRecordToFile(SatelliteIDs,
			      GPS_INFO.Hour, GPS_INFO.Minute, GPS_INFO.Second, bAlways);
    }
  else
    return false;  // track whether we succussfully write it, else we retry
}
#endif // LOGFRECORD

static bool IsAlphaNum (TCHAR c) {
  if (((c >= _T('A'))&&(c <= _T('Z')))
      ||((c >= _T('a'))&&(c <= _T('z')))
      ||((c >= _T('0'))&&(c <= _T('9')))) {
    return true;
  } else {
    return false;
  }
}

void StartLogger()
{

  SHOWTHREAD(_T("StartLogger"));

  int i;
  TCHAR path[MAX_PATH+1];
  TCHAR cAsset[3];

  // strAsset is initialized with DUM.
  if (_tcslen(PilotName_Config)>0) {
	strAssetNumber[0]= IsAlphaNum(PilotName_Config[0]) ? PilotName_Config[0] : _T('A');
	strAssetNumber[1]= IsAlphaNum(PilotName_Config[1]) ? PilotName_Config[1] : _T('A');
  } else {
	strAssetNumber[0]= _T('D');
	strAssetNumber[1]= _T('U');
  }
  if (_tcslen(AircraftType_Config)>0) {
	strAssetNumber[2]= IsAlphaNum(AircraftType_Config[0]) ? AircraftType_Config[0] : _T('A');
  } else {
	strAssetNumber[2]= _T('M');
  }
  strAssetNumber[0]= _totupper(strAssetNumber[0]);
  strAssetNumber[1]= _totupper(strAssetNumber[1]);
  strAssetNumber[2]= _totupper(strAssetNumber[2]);
  strAssetNumber[3]= _T('\0');

  for (i=0; i < 3; i++) { // chars must be legal in file names
    cAsset[i] = IsAlphaNum(strAssetNumber[i]) ? strAssetNumber[i] : _T('A');
  }

  LocalPath(path,TEXT(LKD_LOGS));

  if (TaskModified) {
    SaveDefaultTask();
  }

  for(i=1;i<99;i++)
    {
      // 2003-12-31-XXX-987-01.IGC
      // long filename form of IGC file.
      // XXX represents manufacturer code

      if (!LoggerShortName) {
        // Long file name
        _sntprintf(szFLoggerFileName, std::size(szFLoggerFileName),
                 TEXT("%s%s%04d-%02d-%02d-%s-%c%c%c-%02d.IGC"),
                 path, _T(DIRSEP),
                 GPS_INFO.Year,
                 GPS_INFO.Month,
                 GPS_INFO.Day,
                 _T(LOGGER_MANUFACTURER),
                 cAsset[0],
                 cAsset[1],
                 cAsset[2],
                 i);

      } else {
        // Short file name
        TCHAR cyear, cmonth, cday, cflight;
        cyear = NumToIGCChar((int)GPS_INFO.Year % 10);
        cmonth = NumToIGCChar(GPS_INFO.Month);
        cday = NumToIGCChar(GPS_INFO.Day);
        cflight = NumToIGCChar(i);
        _sntprintf(szFLoggerFileName, std::size(szFLoggerFileName),
                 TEXT("%s%s%c%c%cX%c%c%c%c.IGC"),
                 path, _T(DIRSEP),
                 cyear,
                 cmonth,
                 cday,
                 cAsset[0],
                 cAsset[1],
                 cAsset[2],
                 cflight);

      } // end if

      if(!lk::filesystem::exist(szFLoggerFileName)) {
        break;
      }
  } // end while

  igc_writer_ptr = std::make_unique<igc_file_writer>(szFLoggerFileName, LoggerGActive());

  StartupStore(_T(". Logger Started %s  File <%s>"), WhatTimeIsIt(), szFLoggerFileName);
}

//
// Paolo+Durval: feed external headers to LK for PNAdump software
//
#define MAXHLINE 100
#define EXTHFILE	"COMPE.CNF"
//#define DEBUGHFILE	1
static void AdditionalHeaders(void) {

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

        sprintf(tmps, HFREMARK, &tmpString[1]);
        IGCWriteRecord(tmps);
    }
    fclose(stream);
}

//
// This is called by Calc/Task thread, after calling StartLogger
//
void LoggerHeader(void)
{
  char datum[]= "HFDTM100GPSDATUM:WGS-84\r\n";
  char temp[300];

  // Flight recorder ID number MUST go first..

  // Do one more check on %C because if one is 0 the string will not be closed by newline
  // resulting in a wrong header!
  strAssetNumber[0]= IsAlphaNum(strAssetNumber[0]) ? strAssetNumber[0] : _T('A');
  strAssetNumber[1]= IsAlphaNum(strAssetNumber[1]) ? strAssetNumber[1] : _T('A');
  strAssetNumber[2]= IsAlphaNum(strAssetNumber[0]) ? strAssetNumber[2] : _T('A');

  sprintf(temp,
	  A_RECORD,
	  LOGGER_MANUFACTURER,
	  strAssetNumber[0],
	  strAssetNumber[1],
	  strAssetNumber[2]);
  IGCWriteRecord(temp);

  sprintf(temp,"HFDTE%02d%02d%02d\r\n",
	  GPS_INFO.Day,
	  GPS_INFO.Month,
	  GPS_INFO.Year % 100);
  IGCWriteRecord(temp);

  // Example: Hanna.Reitsch
  sprintf(temp,HFPLTPILOT, PilotName_Config);
  IGCWriteRecord(temp);

  // Example: DG-300
  sprintf(temp,HFGTYGLIDERTYPE, AircraftType_Config);
  IGCWriteRecord(temp);

  // Example: D-7176
  sprintf(temp,HFGIDGLIDERID, AircraftRego_Config);
  IGCWriteRecord(temp);

  // 110117 TOCHECK: maybe a 8 char limit is needed.
  sprintf(temp,HFCCLCOMPETITIONCLASS, CompetitionClass_Config);
  IGCWriteRecord(temp);

  sprintf(temp,HFCIDCOMPETITIONID, CompetitionID_Config);
  IGCWriteRecord(temp);

    #ifndef LKCOMPETITION
  sprintf(temp,"HFFTYFRTYPE:%s\r\n", LKFORK); // default
    #else
  sprintf(temp,"HFFTYFRTYPE:%sC\r\n", LKFORK); // default
    #endif

  // PNAs are also PPC2003, so careful
  #ifdef PNA
    char pnamodel[MAX_PATH+1];
    TCHAR2ascii(GlobalModelName, pnamodel, sizeof(pnamodel));
    pnamodel[_tcslen(GlobalModelName)]='\0';
    #ifndef LKCOMPETITION
    sprintf(temp,"HFFTYFRTYPE:%s PNA %s\r\n", LKFORK,pnamodel);
	#else
    sprintf(temp,"HFFTYFRTYPE:%sC PNA %s\r\n", LKFORK,pnamodel);
	#endif
  #else

  #ifdef PPC2002
	#ifndef LKCOMPETITION
    sprintf(temp,"HFFTYFRTYPE:%s PPC2002\r\n", LKFORK);
	#else
    sprintf(temp,"HFFTYFRTYPE:%sC PPC2002\r\n", LKFORK);
	#endif
  #endif
  // PNA is also PPC2003..
  #ifdef PPC2003
	#ifndef LKCOMPETITION
    sprintf(temp,"HFFTYFRTYPE:%s PPC2003\r\n", LKFORK);
	#else
    sprintf(temp,"HFFTYFRTYPE:%sC PPC2003\r\n", LKFORK);
	#endif
  #endif

  #endif

  #ifdef __linux__
	#ifndef LKCOMPETITION
    sprintf(temp,"HFFTYFRTYPE:%s LINUX\r\n", LKFORK);
	#else
    sprintf(temp,"HFFTYFRTYPE:%sC LINUX\r\n", LKFORK);
	#endif
  #endif
// Kobo & android AFTER __linux__
  #ifdef ANDROID
    #ifndef LKCOMPETITION
      sprintf(temp,"HFFTYFRTYPE:%s ANDROID\r\n", LKFORK);
    #else
      sprintf(temp,"HFFTYFRTYPE:%sC ANDROID\r\n", LKFORK);
    #endif
  #endif
  #ifdef KOBO
	#ifndef LKCOMPETITION
    sprintf(temp,"HFFTYFRTYPE:%s KOBO\r\n", LKFORK);
	#else
    sprintf(temp,"HFFTYFRTYPE:%sC KOBO\r\n", LKFORK);
	#endif
  #endif
  #ifdef WINDOWSPC
	#ifndef LKCOMPETITION
    sprintf(temp,"HFFTYFRTYPE:%s WINDOWSPC\r\n", LKFORK);
	#else
    sprintf(temp,"HFFTYFRTYPE:%sC WINDOWSPC\r\n", LKFORK);
	#endif
  #endif


  IGCWriteRecord(temp);

  #ifndef LKCOMPETITION
  sprintf(temp,"HFRFWFIRMWAREVERSION:%s.%s\r\n", LKVERSION, LKRELEASE);
  #else
  sprintf(temp,"HFRFWFIRMWAREVERSION:%s.%s.COMPETITION\r\n", LKVERSION, LKRELEASE);
  #endif
  IGCWriteRecord(temp);

  IGCWriteRecord("HFALGALTGPS:GEO\r\n");
  if(GPS_INFO.BaroAltitudeAvailable) {
    IGCWriteRecord("HFALPALTPRESSURE:ISA\r\n");
  }

  IGCWriteRecord(datum);

  if (GPSAltitudeOffset != 0) {
     sprintf(temp,"HFGPSALTITUDEOFFSET: %+.0f\r\n", GPSAltitudeOffset);
     IGCWriteRecord(temp);
  }


  AdditionalHeaders();

}


void StartDeclaration(int ntp)
{
  // TODO bug: this is causing problems with some analysis software
  // maybe it's because the date and location fields are bogus
  char start[] = "C0000000N00000000ETAKEOFF\r\n";
  char temp[100];

  if (NumLoggerBuffered==0) {
    FirstPoint.Year = GPS_INFO.Year;
    FirstPoint.Month = GPS_INFO.Month;
    FirstPoint.Day = GPS_INFO.Day;
    FirstPoint.Hour = GPS_INFO.Hour;
    FirstPoint.Minute = GPS_INFO.Minute;
    FirstPoint.Second = GPS_INFO.Second;
  }

  // JMW added task start declaration line

  // LGCSTKF013945TAKEOFF DETECTED

  // IGC GNSS specification 3.6.1
  sprintf(temp,
	  "C%02d%02d%02d%02d%02d%02d0000000000%02d\r\n",
	  // DD  MM  YY  HH  MM  SS  DD  MM  YY IIII TT
	  FirstPoint.Day,
	  FirstPoint.Month,
	  FirstPoint.Year % 100,
	  FirstPoint.Hour,
	  FirstPoint.Minute,
	  FirstPoint.Second,
	  ntp-2);

  IGCWriteRecord(temp);
  // takeoff line
  // IGC GNSS specification 3.6.3

  // Use homewaypoint as default takeoff and landing position. Better than an empty field!
  if (ValidWayPoint(HomeWaypoint)) {
	TCHAR wname[NAME_SIZE+1];
	_stprintf(wname,_T("%s"),WayPointList[HomeWaypoint].Name);
	wname[8]='\0';
	AddDeclaration(WayPointList[HomeWaypoint].Latitude, WayPointList[HomeWaypoint].Longitude, wname);
  } else
	IGCWriteRecord(start);

}


void EndDeclaration(void)
{
  // TODO bug: this is causing problems with some analysis software
  // maybe it's because the date and location fields are bogus
  char start[] = "C0000000N00000000ELANDING\r\n";

  // Use homewaypoint as default takeoff and landing position. Better than an empty field!
  if (ValidWayPoint(HomeWaypoint)) {
	TCHAR wname[NAME_SIZE+1];
	_stprintf(wname,_T("%s"),WayPointList[HomeWaypoint].Name);
	wname[8]='\0';
	AddDeclaration(WayPointList[HomeWaypoint].Latitude, WayPointList[HomeWaypoint].Longitude, wname);
  } else
	IGCWriteRecord(start);
}

void AddDeclaration(double Latitude, double Longitude, TCHAR *ID)
{
  char szCRecord[500];

  char IDString[MAX_PATH];
  int i;

  int DegLat, DegLon;
  double MinLat, MinLon;
  char NoS, EoW;

  TCHAR tmpstring[MAX_PATH];
  _tcscpy(tmpstring, ID);
  CharUpper(tmpstring);
  for(i=0;i<(int)_tcslen(tmpstring);i++)
    {
      IDString[i] = (char)tmpstring[i];
    }
  IDString[i] = '\0';

  DegLat = (int)Latitude;
  MinLat = Latitude - DegLat;
  NoS = 'N';
  if((MinLat<0) || ((MinLat-DegLat==0) && (DegLat<0)))
    {
      NoS = 'S';
      DegLat *= -1; MinLat *= -1;
    }
  MinLat *= 60;
  MinLat *= 1000;

  DegLon = (int)Longitude ;
  MinLon = Longitude  - DegLon;
  EoW = 'E';
  if((MinLon<0) || ((MinLon-DegLon==0) && (DegLon<0)))
    {
      EoW = 'W';
      DegLon *= -1; MinLon *= -1;
    }
  MinLon *=60;
  MinLon *= 1000;

  sprintf(szCRecord,"C%02d%05.0f%c%03d%05.0f%c%s\r\n",
	  DegLat, MinLat, NoS, DegLon, MinLon, EoW, IDString);

  IGCWriteRecord(szCRecord);
}


// This is bad, useless, and now removed
void LoggerNote(const TCHAR *text) {
  return;
  /* NOT THREAD SAFE
  if (LoggerActive) {

    char fulltext[500];
    sprintf(fulltext, "LPLT%S\r\n", text);
    IGCWriteRecord(fulltext);

  }
  */
}


bool DeclaredToDevice = false;


static bool LoggerDeclare(PDeviceDescriptor_t dev, Declaration_t *decl)
{
  if (!devIsLogger(dev))
	return FALSE;

  // If a Flarm is reset while we are here, then it will come up with isFlarm set to false,
  // and task declaration will fail. The solution is to let devices have a flag for "HaveFlarm".
  LKDoNotResetComms=true;

  // LKTOKEN  _@M221_ = "Declare Task?"
  if (MessageBoxX(MsgToken(221),
	dev->Name, mbYesNo) == IdYes) {

	const unsigned ERROR_BUFFER_LEN = 64;
	TCHAR errorBuffer[ERROR_BUFFER_LEN] = { '\0' };

	if (devDeclare(dev, decl, ERROR_BUFFER_LEN, errorBuffer)) {
		// LKTOKEN  _@M686_ = "Task Declared!"
		MessageBoxX(MsgToken(686),
			dev->Name, mbOk);

		DeclaredToDevice = true;

	} else {

		TCHAR buffer[2*ERROR_BUFFER_LEN];

		if(errorBuffer[0] == '\0') {
			// LKTOKEN  _@M1410_ = "Unknown error"
			_tcsncpy(errorBuffer, MsgToken(1410), ERROR_BUFFER_LEN);
		} else {
			// do it just to be sure
			errorBuffer[ERROR_BUFFER_LEN - 1] = '\0';
		}
        StartupStore(_T("Error! Task NOT declared : %s" NEWLINE), errorBuffer);

		// LKTOKEN  _@M265_ = "Error! Task NOT declared!"
		_sntprintf(buffer, 2*ERROR_BUFFER_LEN, _T("%s\n%s"), MsgToken(265), errorBuffer);
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
    MessageBoxX(MsgToken(1423), _T(""), mbOk);
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
    MessageBoxX(MsgToken(474), _T(""), mbOk);
    DeclaredToDevice = true; // testing only
  }

}


bool CheckDeclaration(void) {
  if (!DeclaredToDevice) {
    return true;
  } else {
    if(MessageBoxX(
	// LKTOKEN  _@M492_ = "OK to invalidate declaration?"
		   MsgToken(492),
	// LKTOKEN  _@M694_ = "Task declared"
		   MsgToken(694),
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
  _stprintf(searchpath, TEXT("%s*.igc"),pathname);
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
  _stprintf(fullname, TEXT("%s%s"),pathname,oldestname.c_str());
  #if TESTBENCH
  StartupStore(_T("... DeleteOldIGCFile <%s> ...\n"),fullname);
  #endif
  lk::filesystem::deleteFile(fullname);
  #if TESTBENCH
  StartupStore(_T("... done DeleteOldIGCFile\n"));
  #endif
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
    #if TESTBENCH
    StartupStore(TEXT("... LoggerFreeSpace returned: true%s"),NEWLINE);
    #endif
    return true;
  } else {
    StartupStore(TEXT("--- LoggerFreeSpace returned: false%s"),NEWLINE);
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

