/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Logger.cpp,v 8.4 2010/12/11 23:59:33 root Exp root $
*/

#include "externs.h"
#include "Logger.h"
#include "InputEvents.h"
#include "dlgTools.h"
#include "TraceThread.h"
#include <ctype.h>
#include <utils/stl_utils.h>

// #define DEBUG_LOGGER	1

#ifdef _UNICODE
    #define HFPLTPILOT              "HFPLTPILOT:%S\r\n"
    #define HFGTYGLIDERTYPE         "HFGTYGLIDERTYPE:%S\r\n"
    #define HFGIDGLIDERID           "HFGIDGLIDERID:%S\r\n"
    #define HFCCLCOMPETITIONCLASS   "HFCCLCOMPETITIONCLASS:%S\r\n"
    #define HFCIDCOMPETITIONID      "HFCIDCOMPETITIONID:%S\r\n"
#else
    #define HFPLTPILOT              "HFPLTPILOT:%s\r\n"
    #define HFGTYGLIDERTYPE         "HFGTYGLIDERTYPE:%s\r\n"
    #define HFGIDGLIDERID           "HFGIDGLIDERID:%s\r\n"
    #define HFCCLCOMPETITIONCLASS   "HFCCLCOMPETITIONCLASS:%s\r\n"
    #define HFCIDCOMPETITIONID      "HFCIDCOMPETITIONID:%s\r\n"
#endif
#define HFREMARK                "HFREMARK:%s\r\n"

#define LOGGER_MANUFACTURER	"XLK"

extern NMEA_INFO GPS_INFO;

TCHAR NumToIGCChar(int n) {
  if (n<10) {
    return _T('1') + (n-1);
  } else {
    return _T('A') + (n-10);
  }
}


int IGCCharToNum(TCHAR c) {
  if ((c >= _T('1')) && (c<= _T('9'))) {
    return c- _T('1') + 1;
  } else if ((c >= _T('A')) && (c<= _T('Z'))) {
    return c- _T('A') + 10;
  } else {
    return 0; // Error!
  }
}



static TCHAR szLoggerFileName[MAX_PATH+1] = TEXT("\0");	 // LOGGER_TMP.IGC
static TCHAR szFLoggerFileName[MAX_PATH+1] = TEXT("\0"); // final IGC name
static TCHAR szSLoggerFileName[MAX_PATH+1] = TEXT("\0"); // LOGGER_SIG.IGC
static TCHAR szFLoggerFileNameRoot[MAX_PATH+1] = TEXT("\0");
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

LoggerBuffer_T FirstPoint;
LoggerBuffer_T LoggerBuffer[MAX_LOGGER_BUFFER];



void StopLogger(void) {
  TCHAR szMessage[(MAX_PATH*2)+1] = TEXT("\0");
  int iLoggerError=0;  // see switch statement for error handler
  TCHAR sztmplogfile[MAX_PATH+1] = TEXT("\0");
  int retval=0;

  _tcscpy(sztmplogfile,szLoggerFileName); // use LOGGER_TMP, unsigned

  if (LoggerActive) {
    LoggerActive = false;
    if (LoggerClearFreeSpace()) {

    #if (TESTBENCH && DEBUG_LOGGER)
    if (LoggerGActive())
    #else
    if (!SIMMODE && LoggerGActive())
    #endif
	{

	extern int RunSignature();
	retval = RunSignature();
	if (retval!=0) {
		StartupStore(_T(".... LOGGER SIGNATURE ERROR, CODE=%d%s"),retval,NEWLINE);
		switch(retval) {
			case -1:
				StartupStore(_T(".... (EXEQ DEBUG FAILURE)%s"),NEWLINE);
				break;
			case 1:
				StartupStore(_T(".... (SOURCE FILE DISAPPEARED)%s"),NEWLINE);
				break;
			case 3:
				StartupStore(_T(".... (EXEQ WITH WRONG ARGUMENTS)%s"),NEWLINE);
				break;
			case 4:
				StartupStore(_T(".... (BAD ENVIRONMENT)%s"),NEWLINE);
				break;
			case 11:
				StartupStore(_T(".... (LOGGER_TMP DISAPPEARED)%s"),NEWLINE);
				break;
			case 12:
				StartupStore(_T(".... (LOGGER_SIG ALREADY EXISTING)%s"),NEWLINE);
				break;
			case 21:
				StartupStore(_T(".... (MUTEX FAILURE=)%s"),NEWLINE);
				break;
			case 259:
				StartupStore(_T(".... (PROCESS DID NOT TERMINATE!)%s"),NEWLINE);
				break;
			default:
				break;
		}
		// we shall be moving LOGGER_TMP, and leave LOGGER_SIG untouched. In fact we do not know
		// if LOGGER_SIG is or will be available.
	} else {
		// RunSig ok, change logfile to new logger_sig
		StartupStore(_T(". Logger OK, IGC signed with G-Record%s"),NEWLINE);
		lk::filesystem::deleteFile(szLoggerFileName);	// remove old LOGGER_TMP
		_tcscpy(sztmplogfile,szSLoggerFileName); // use LOGGER_SIG, signed
	}

	} // logger active

      int imCount=0;
      const int imMax=3;
      for (imCount=0; imCount < imMax; imCount++) {
        // MoveFile() nonzero==Success
        if (lk::filesystem::moveFile(sztmplogfile, szFLoggerFileName)) {
          iLoggerError=0;
          break; // success
        }
        Sleep(750); // wait for file system cache to fix itself?
      }
      if (imCount == imMax) { // MoveFile() failed all attempts

        if (!lk::filesystem::moveFile(sztmplogfile, szFLoggerFileNameRoot)) { // try rename it and leave in root
          iLoggerError=1; //Fail.  NoMoveNoRename
        }
        else {
          iLoggerError=2; //NoMoveYesRename
        }
      }

    } // logger clearfreespace
    else { // Insufficient disk space.  // MoveFile() nonzero==Success
      if (!lk::filesystem::moveFile(sztmplogfile, szFLoggerFileNameRoot)) { // try rename it and leave in root
        iLoggerError=3; //Fail.  Insufficient Disk Space, NoRename
      }
      else {
        iLoggerError=4; //Success.  Insufficient Disk Space, YesRename
      }
    }

    switch (iLoggerError) { //0=Success 1=NoMoveNoRename 2=NoMoveYesRename 3=NoSpaceNoRename 4=NoSpaceYesRename
    case 0:
      StartupStore(TEXT(". Logger: File saved %s%s"),WhatTimeIsIt(),NEWLINE);
      break;

    case 1: // NoMoveNoRename
      LK_tcsncpy(szMessage,TEXT("--- Logger file not copied.  It is in the root folder of your device and called "),MAX_PATH);
      _tcsncat(szMessage,sztmplogfile,MAX_PATH);

      MessageBoxX(hWndMapWindow,
		gettext(szMessage),
	// LKTOKEN  _@M404_ = "Logger Error" 
		gettext(TEXT("_@M404_")), MB_OK| MB_ICONERROR);
      _tcsncat(szMessage,TEXT(SNEWLINE),MAX_PATH);
      StartupStore(szMessage);
      break;

    case 2: // NoMoveYesRename
      LK_tcsncpy(szMessage,TEXT("--- Logger file not copied.  It is in the root folder of your device"),MAX_PATH);

      MessageBoxX(hWndMapWindow,
		gettext(szMessage),
	// LKTOKEN  _@M404_ = "Logger Error" 
		gettext(TEXT("_@M404_")), MB_OK| MB_ICONERROR);
      _tcsncat(szMessage,TEXT(SNEWLINE),MAX_PATH);
      StartupStore(szMessage);
      break;

    case 3: // Insufficient Storage.  NoRename
      LK_tcsncpy(szMessage,TEXT("++++++ Insuff. storage. Logger file in device's root folder, called "),MAX_PATH);
      _tcsncat(szMessage,sztmplogfile,MAX_PATH);

      MessageBoxX(hWndMapWindow,
		gettext(szMessage),
	// LKTOKEN  _@M404_ = "Logger Error" 
		gettext(TEXT("_@M404_")), MB_OK| MB_ICONERROR);
      _tcsncat(szMessage,TEXT(SNEWLINE),MAX_PATH);
      StartupStore(szMessage);
      break;

    case 4: // Insufficient Storage.  YesRename
      LK_tcsncpy(szMessage,TEXT("++++++ Insufficient storage.  Logger file is in the root folder of your device"),MAX_PATH);

      MessageBoxX(hWndMapWindow,
		gettext(szMessage),
	// LKTOKEN  _@M404_ = "Logger Error" 
		gettext(TEXT("_@M404_")), MB_OK| MB_ICONERROR);
      _tcsncat(szMessage,TEXT(SNEWLINE),MAX_PATH);
      StartupStore(szMessage);
      break;
} // error handler

    NumLoggerBuffered = 0;
  }
}

// BaroAltitude in this case is a QNE altitude (aka pressure altitude)
// Some few instruments are sending only a cooked QNH altitude, without the relative QNH.
// (If we had QNH in that case, we would save real QNE altitude in GPS_INFO.BaroAltitude)
// There is nothing we can do about it, in these few cases: we shall log a QNH altitude instead
// of QNE altitude, which is what we have been doing up to v4 in any case. It cant be worst.
// In all other cases, the pressure altitude will be saved, and out IGC logger replay is converting it
// to the desired QNH altitude back. 
#if LOGFRECORD
void LogPointToBuffer(double Latitude, double Longitude, double Altitude,
                      double BaroAltitude, short Hour, short Minute, short Second,
                      int SatelliteIDs[]) {
#else
void LogPointToBuffer(double Latitude, double Longitude, double Altitude,
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


void LogPointToFile(double Latitude, double Longitude, double Altitude,
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
          (int)BaroAltitude,(int)Altitude);

  IGCWriteRecord(szBRecord);
}


void LogPoint(double Latitude, double Longitude, double Altitude,
              double BaroAltitude) {
  if (!LoggerActive) {
    if (!GPS_INFO.NAVWarning) {
      #if LOGFRECORD
      LogPointToBuffer(Latitude, Longitude, Altitude, BaroAltitude,
                       GPS_INFO.Hour, GPS_INFO.Minute, GPS_INFO.Second,
                       GPS_INFO.SatelliteIDs);
      #else
      LogPointToBuffer(Latitude, Longitude, Altitude, BaroAltitude, GPS_INFO.Hour, GPS_INFO.Minute, GPS_INFO.Second);
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
    LogPointToFile(Latitude, Longitude, Altitude, BaroAltitude,
                   GPS_INFO.Hour, GPS_INFO.Minute, GPS_INFO.Second);
  }
}


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

bool IsAlphaNum (TCHAR c) {
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
  _stprintf(szLoggerFileName, TEXT("%s\\LOGGER_TMP.IGC"), path);

  _stprintf(szSLoggerFileName, TEXT("%s\\LOGGER_SIG.IGC"), path);
  TCHAR newfile[MAX_PATH+20];
  if (lk::filesystem::exist(szLoggerFileName)) {
	StartupStore(_T("---- Logger recovery: Existing LOGGER_TMP.IGC found, renamed to LOST%s"),NEWLINE);
	_stprintf(newfile, TEXT("%s\\LOST_%02d%02d%02d.IGC"), path, GPS_INFO.Hour, GPS_INFO.Minute, GPS_INFO.Second);
	lk::filesystem::copyFile(szLoggerFileName,newfile,false);
	lk::filesystem::deleteFile(szLoggerFileName);
  }
  if (lk::filesystem::exist(szSLoggerFileName)) {
	StartupStore(_T("---- Logger recovery (G): Existing LOGGER_SIG.IGC found, renamed to LOSTG%s"),NEWLINE);
	_stprintf(newfile, TEXT("%s\\LOSTG_%02d%02d%02d.IGC"), path, GPS_INFO.Hour, GPS_INFO.Minute, GPS_INFO.Second);
	lk::filesystem::copyFile(szSLoggerFileName,newfile,false);
	lk::filesystem::deleteFile(szSLoggerFileName);
  }

  
  for(i=1;i<99;i++)
    {
      // 2003-12-31-XXX-987-01.IGC
      // long filename form of IGC file.
      // XXX represents manufacturer code

      if (!LoggerShortName) {
        // Long file name
        _stprintf(szFLoggerFileName,
                 TEXT("%s\\%04d-%02d-%02d-%s-%c%c%c-%02d.IGC"),
                 path,
                 GPS_INFO.Year,
                 GPS_INFO.Month,
                 GPS_INFO.Day,
		 _T(LOGGER_MANUFACTURER),
                 cAsset[0],
                 cAsset[1],
                 cAsset[2],
                 i);
 
        _stprintf(szFLoggerFileNameRoot,
                 TEXT("%s\\%04d-%02d-%02d-%s-%c%c%c-%02d.IGC"),
                 TEXT(""), // this creates it in root if MoveFile() fails
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
        _stprintf(szFLoggerFileName,
                 TEXT("%s\\%c%c%cX%c%c%c%c.IGC"),
                 path,
                 cyear,
                 cmonth,
                 cday,
                 cAsset[0],
                 cAsset[1],
                 cAsset[2],
                 cflight);

        _stprintf(szFLoggerFileNameRoot,
                 TEXT("%s\\%c%c%cX%c%c%c%c.IGC"),
                 TEXT(""), // this creates it in root if MoveFile() fails
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

  StartupStore(_T(". Logger Started %s  File <%s>%s"),
	WhatTimeIsIt(), szFLoggerFileName,NEWLINE);
}


//
// This is called by Calc/Task thread, after calling StartLogger
//
void LoggerHeader(void)
{
  char datum[]= "HFDTM100GPSDATUM:WGS-84\r\n";
  char temp[100];
  
  // Flight recorder ID number MUST go first..

  // Do one more check on %C because if one is 0 the string will not be closed by newline
  // resulting in a wrong header!
  strAssetNumber[0]= IsAlphaNum(strAssetNumber[0]) ? strAssetNumber[0] : _T('A');
  strAssetNumber[1]= IsAlphaNum(strAssetNumber[1]) ? strAssetNumber[1] : _T('A');
  strAssetNumber[2]= IsAlphaNum(strAssetNumber[0]) ? strAssetNumber[2] : _T('A');

  sprintf(temp,
	  "A%s%C%C%C\r\n",
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
  ConvertTToC(pnamodel,GlobalModelName);
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

  IGCWriteRecord(temp);

  #ifndef LKCOMPETITION
  sprintf(temp,"HFRFWFIRMWAREVERSION:%s.%s\r\n", LKVERSION, LKRELEASE);
  #else
  sprintf(temp,"HFRFWFIRMWAREVERSION:%s.%s.COMPETITION\r\n", LKVERSION, LKRELEASE);
  #endif
  IGCWriteRecord(temp);

  IGCWriteRecord(datum);

  extern void AdditionalHeaders(void);
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
  if (MessageBoxX(hWndMapWindow, gettext(TEXT("_@M221_")), 
	dev->Name, MB_YESNO| MB_ICONQUESTION) == IDYES) {

	const unsigned ERROR_BUFFER_LEN = 64;
	TCHAR errorBuffer[ERROR_BUFFER_LEN] = { '\0' };

	if (devDeclare(dev, decl, ERROR_BUFFER_LEN, errorBuffer)) {
		// LKTOKEN  _@M686_ = "Task Declared!" 
		MessageBoxX(hWndMapWindow, gettext(TEXT("_@M686_")),
			dev->Name, MB_OK| MB_ICONINFORMATION);

		DeclaredToDevice = true;

	} else {

		TCHAR buffer[2*ERROR_BUFFER_LEN];

		if(errorBuffer[0] == '\0') {
			// LKTOKEN  _@M1410_ = "Unknown error" 
			_sntprintf(errorBuffer, ERROR_BUFFER_LEN, gettext(_T("_@M1410_")));
		} else {
			// do it just to be sure
			errorBuffer[ERROR_BUFFER_LEN - 1] = '\0';
		}

		// LKTOKEN  _@M265_ = "Error! Task NOT declared!" 
		_sntprintf(buffer, 2*ERROR_BUFFER_LEN, _T("%s\n%s"), gettext(_T("_@M265_")), errorBuffer);
		MessageBoxX(hWndMapWindow, buffer, dev->Name, MB_OK| MB_ICONERROR);
      
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
    MessageBoxX(hWndMapWindow, gettext(TEXT("_@M1423_")), _T(""), MB_OK| MB_ICONINFORMATION);
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

  if (LoggerDeclare(devA(), &Decl))
    found_logger = true;

  if (LoggerDeclare(devB(), &Decl))
    found_logger = true;

  if (!found_logger) {
	// LKTOKEN  _@M474_ = "No logger connected" 
    MessageBoxX(hWndMapWindow, gettext(TEXT("_@M474_")),
		devB()->Name, MB_OK| MB_ICONINFORMATION);
    DeclaredToDevice = true; // testing only
  }

}


bool CheckDeclaration(void) {
  if (!DeclaredToDevice) {
    return true;
  } else {
    if(MessageBoxX(hWndMapWindow,
	// LKTOKEN  _@M492_ = "OK to invalidate declaration?" 
		   gettext(TEXT("_@M492_")),
	// LKTOKEN  _@M694_ = "Task declared" 
		   gettext(TEXT("_@M694_")),
		   MB_YESNO| MB_ICONQUESTION) == IDYES){
      DeclaredToDevice = false;
      return true;
    } else {
      return false;
    }
  }
}


FILETIME LogFileDate(const TCHAR* filename) {
  FILETIME ft;
  ft.dwLowDateTime = 0;
  ft.dwHighDateTime = 0;

  TCHAR asset[MAX_PATH+1];
  SYSTEMTIME st;
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
    st.wYear = year;
    st.wMonth = month;
    st.wDay = day;
    st.wHour = num;
    st.wMinute = 0;
    st.wSecond = 0;
    st.wMilliseconds = 0;
    SystemTimeToFileTime(&st,&ft);
    return ft;
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
    st.wYear = yearthis;
    st.wMonth = IGCCharToNum(cmonth);
    st.wDay = IGCCharToNum(cday);
    st.wHour = IGCCharToNum(cflight);
    st.wMinute = 0;
    st.wSecond = 0;
    st.wMilliseconds = 0;
    SystemTimeToFileTime(&st,&ft);
    return ft;
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
  return ft;
}


bool LogFileIsOlder(const TCHAR *oldestname, const TCHAR *thisname) {
  FILETIME ftold = LogFileDate(oldestname);
  FILETIME ftnew = LogFileDate(thisname);
  return (CompareFileTime(&ftold, &ftnew)>0);
}


bool DeleteOldIGCFile(TCHAR *pathname) {
  TCHAR searchpath[MAX_PATH+1];
  TCHAR fullname[MAX_PATH+1];
  _stprintf(searchpath, TEXT("%s*.igc"),pathname);
    std::tstring oldestname;
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
  unsigned long kbfree=0;
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

bool IsValidIGCChar(char c) //returns 1 if valid char for IGC files
{//                                 

  if ( c >=0x20  && c <= 0x7E &&
       c != 0x0D &&
       c != 0x0A &&
       c != 0x24 &&
       c != 0x2A &&
       c != 0x2C &&
       c != 0x21 &&
       c != 0x5C &&
       c != 0x5E &&
       c != 0x7E
       )
    return true;
  else
    return false;
}

char * CleanIGCRecord (char * szIn)
{  // replace invalid chars w/ 0x20

  int iLen = strlen(szIn);
  for (int i = 0; i < iLen -2; i++)  // don't clean terminating \r\n!
    if (!IsValidIGCChar(szIn[i]))
      szIn[i]=' ';

  return szIn;
}

bool IGCWriteRecord(const char *szIn) {
    char charbuffer[MAX_IGC_BUFF];
    bool bRetVal = false;

    static bool bWriting = false;

    // THIS IS NOT A SOLUTION. DATA LOSS GRANTED.
    if (!bWriting) {
        bWriting = true;

        FILE* stream = _tfopen(szLoggerFileName, _T("ab"));
        if (stream) {
            strncpy(charbuffer, szIn, array_size(charbuffer));
            charbuffer[array_size(charbuffer) - 1] = '\0';
            CleanIGCRecord(charbuffer);
            fwrite(charbuffer, sizeof(charbuffer[0]), strlen(charbuffer), stream);
            fflush(stream);
            fclose(stream);
            bRetVal = true;
        }
        bWriting = false;
    }
    return bRetVal;
}


bool LoggerGActive()
{
  #if (WINDOWSPC>0)
    #if (TESTBENCH && DEBUG_LOGGER)
    return true;	// THIS IS ONLY for checking Grecord new stuff under testbench
    #else
    return false;
    #endif
  #else
  return true;
  #endif
}


//
//	259	still active, very bad
//	0	is the only OK that we want!
//	other values, very bad
//
int RunSignature() {
    TCHAR homedir[MAX_PATH];
    LocalPath(homedir, TEXT(LKD_LOGS));
       
#ifndef NO_IGCPLUGIN
    DWORD retval = 99;

    TCHAR path[MAX_PATH];
    LocalPath(path, _T(LKD_LOGS));
#if (WINDOWSPC>0)
    _tcscat(path, _T("\\LKRECORD_PC.LK8"));
#endif
    // CAREFUL!!! PNA is ALSO PPC2003!!
    // 
    // ATTENTION: on PNA we are executing LKRECORD_PNA.LK8.EXE really
#ifdef PNA
    _tcscat(path, _T("\\LKRECORD_PNA.LK8"));
#else
#ifdef PPC2002
    _tcscat(path, _T("\\LKRECORD_2002.LK8"));
#endif
#ifdef PPC2003
    _tcscat(path, _T("\\LKRECORD_2003.LK8"));
#endif
#endif 

#if TESTBENCH
    StartupStore(_T(".... RunSignature: homedir <%s>%s"), homedir, NEWLINE);
#endif

    PROCESS_INFORMATION pi;

#if (WINDOWSPC>0)
    // Sadly, some parameters cannot be passed in the CE version
    STARTUPINFO si;
    ZeroMemory(&si, sizeof (STARTUPINFO));
    si.cb = sizeof (STARTUPINFO);
    si.wShowWindow = SW_SHOWNORMAL;
    si.dwFlags = STARTF_USESHOWWINDOW;

    if (::CreateProcess(path, homedir, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
#else
    if (::CreateProcess(path, homedir, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, NULL, &pi)) {
#endif

        ::WaitForSingleObject(pi.hProcess, 30000); // 30s
        GetExitCodeProcess(pi.hProcess, &retval);
        // STILL_ACTIVE = 259, this retval should be checked for

#if TESTBENCH
        StartupStore(_T(".... RunSignature exec <%s> terminated, retval=%lu%s"), path, retval, NEWLINE);
#endif

        return retval;
    } else {
        DWORD lasterr = GetLastError();

        if (lasterr != 2) {
            // External executable failure, bad !
            StartupStore(_T(".... RunSignature exec <%s> FAILED, error code=%lu %s"), path, lasterr, NEWLINE);
#if TESTBENCH
            StartupStore(_T(".... Trying with DoSignature\n"));
#endif
        }
#if TESTBENCH
        else
            StartupStore(_T(".... no executable <%s> found, proceeding with DoSignature\n"), path);
#endif      
    }
#endif

    extern int DoSignature(TCHAR * hpath);
    return DoSignature(homedir);
}


//
// Paolo+Durval: feed external headers to LK for PNAdump software
//
#define MAXHLINE 100  
#define EXTHFILE	"COMPE.CNF"
//#define DEBUGHFILE	1

void AdditionalHeaders(void) {

    TCHAR pathfilename[MAX_PATH + 1];
    _stprintf(pathfilename, TEXT("%s\\%s\\%s"), LKGetLocalPath(), TEXT(LKD_LOGS), _T(EXTHFILE));

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
    char tmps[MAXHLINE + 1];
    tmpString[MAXHLINE] = '\0';
    
    while(size_t nbRead = fread(tmpString, sizeof(tmpString[0]), array_size(tmpString) - 1U, stream)) {
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
