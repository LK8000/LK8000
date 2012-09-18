/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Logger.cpp,v 8.4 2010/12/11 23:59:33 root Exp root $
*/

#include "externs.h"
#include "Logger.h"
#include "InputEvents.h"
#include "LKProfiles.h"


// Once done with all validations we can switch to XK8 code
// #define USEXK8	1

#if USEXK8
 #define LOGGER_MANUFACTURER	"XK8"
#else
 #define LOGGER_MANUFACTURER	"XCS"
#endif


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
static double FRecordLastTime = 0;
static char szLastFRecord[MAX_IGC_BUFF];

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
  int SatelliteIDs[MAXSATELLITES];
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

    #if TESTBENCH
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
		DeleteFile(szLoggerFileName);	// remove old LOGGER_TMP
		_tcscpy(sztmplogfile,szSLoggerFileName); // use LOGGER_SIG, signed
	}

	} // logger active

      int imCount=0;
      const int imMax=3;
      for (imCount=0; imCount < imMax; imCount++) {
        // MoveFile() nonzero==Success
        if (0 != MoveFile( sztmplogfile, szFLoggerFileName)) {
          iLoggerError=0;
          break; // success
        }
        Sleep(750); // wait for file system cache to fix itself?
      }
      if (imCount == imMax) { // MoveFile() failed all attempts

        if (0 == MoveFile( sztmplogfile, szFLoggerFileNameRoot)) { // try rename it and leave in root
          iLoggerError=1; //Fail.  NoMoveNoRename
        }
        else {
          iLoggerError=2; //NoMoveYesRename
        }
      }

    } // logger clearfreespace
    else { // Insufficient disk space.  // MoveFile() nonzero==Success
      if (0 == MoveFile( sztmplogfile, szFLoggerFileNameRoot)) { // try rename it and leave in root
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

// BaroAltitude should NOT be compensated for QNH. It should be a QNE altitude.
// Untils those people at IGC will not write more clearly their loggers specs, this confusion will persist.
// In our case, we should save BaroAltitude incoming, referred to 1013.25mb , here, BEFORE we change it to QNH.
void LogPointToBuffer(double Latitude, double Longitude, double Altitude,
                      double BaroAltitude, short Hour, short Minute, short Second,
                      int SatelliteIDs[]) {

  if (NumLoggerBuffered== MAX_LOGGER_BUFFER) {
    for (int i= 0; i< NumLoggerBuffered-1; i++) {
      LoggerBuffer[i]= LoggerBuffer[i+1];
    }
  } else {
    NumLoggerBuffered++;
  }
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

  for (int iSat=0; iSat < MAXSATELLITES; iSat++)
    LoggerBuffer[NumLoggerBuffered-1].SatelliteIDs[iSat]=SatelliteIDs[iSat];

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

  if ((Altitude<0) && (BaroAltitude<0)) return;
  Altitude = max(0.0,Altitude);
  BaroAltitude = max(0.0,BaroAltitude);

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
      LogPointToBuffer(Latitude, Longitude, Altitude, BaroAltitude,
                       GPS_INFO.Hour, GPS_INFO.Minute, GPS_INFO.Second,
                       GPS_INFO.SatelliteIDs);
    }
  } else if (NumLoggerBuffered && !GPS_INFO.NAVWarning) { 

    LogFRecordToFile(LoggerBuffer[0].SatelliteIDs,  // write FRec before cached BRecs
                   LoggerBuffer[0].Hour,
                   LoggerBuffer[0].Minute,
                   LoggerBuffer[0].Second,
                   true);

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

bool LogFRecordToFile(int SatelliteIDs[], short Hour, short Minute, short Second, bool bAlways)
{ // bAlways forces write when completing header for restart
  // only writes record if constallation has changed unless bAlways set
  #if TESTBENCH
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
  HANDLE hFile;
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
  strAssetNumber[0]= towupper(strAssetNumber[0]);
  strAssetNumber[1]= towupper(strAssetNumber[1]);
  strAssetNumber[2]= towupper(strAssetNumber[2]);
  strAssetNumber[3]= _T('\0');

  for (i=0; i < 3; i++) { // chars must be legal in file names
    cAsset[i] = IsAlphaNum(strAssetNumber[i]) ? strAssetNumber[i] : _T('A');
  }

  LocalPath(path,TEXT(LKD_LOGS));

  if (TaskModified) {
    SaveDefaultTask();
  }
  wsprintf(szLoggerFileName, TEXT("%s\\LOGGER_TMP.IGC"), path);

  wsprintf(szSLoggerFileName, TEXT("%s\\LOGGER_SIG.IGC"), path);
  TCHAR newfile[MAX_PATH+20];
  if (GetFileAttributes(szLoggerFileName) != 0xffffffff) {
	StartupStore(_T("---- Logger recovery: Existing LOGGER_TMP.IGC found, renamed to LOST%s"),NEWLINE);
	wsprintf(newfile, TEXT("%s\\LOST_%02d%02d%02d.IGC"), path, GPS_INFO.Hour, GPS_INFO.Minute, GPS_INFO.Second);
	CopyFile(szLoggerFileName,newfile,TRUE);
	DeleteFile(szLoggerFileName);
  }
  if (GetFileAttributes(szSLoggerFileName) != 0xffffffff) {
	StartupStore(_T("---- Logger recovery (G): Existing LOGGER_SIG.IGC found, renamed to LOSTG%s"),NEWLINE);
	wsprintf(newfile, TEXT("%s\\LOSTG_%02d%02d%02d.IGC"), path, GPS_INFO.Hour, GPS_INFO.Minute, GPS_INFO.Second);
	CopyFile(szSLoggerFileName,newfile,TRUE);
	DeleteFile(szSLoggerFileName);
  }

  
  for(i=1;i<99;i++)
    {
      // 2003-12-31-XXX-987-01.IGC
      // long filename form of IGC file.
      // XXX represents manufacturer code

      if (!LoggerShortName) {
        // Long file name
        wsprintf(szFLoggerFileName,
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
 
        wsprintf(szFLoggerFileNameRoot,
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
        wsprintf(szFLoggerFileName,
                 TEXT("%s\\%c%c%cX%c%c%c%c.IGC"),
                 path,
                 cyear,
                 cmonth,
                 cday,
                 cAsset[0],
                 cAsset[1],
                 cAsset[2],
                 cflight);

        wsprintf(szFLoggerFileNameRoot,
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

      hFile = CreateFile(szFLoggerFileName, GENERIC_WRITE,
			 FILE_SHARE_WRITE, NULL, CREATE_NEW,
			 FILE_ATTRIBUTE_NORMAL, 0);
      if(hFile!=INVALID_HANDLE_VALUE )
	{
          // file already exists
      CloseHandle(hFile);
      DeleteFile(szFLoggerFileName);
      break;
	}
  } // end while

  StartupStore(_T(". Logger Started %s  File <%s>%s"),
	WhatTimeIsIt(), szFLoggerFileName,NEWLINE);


  return;
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
  sprintf(temp,"HFPLTPILOT:%S\r\n", PilotName_Config);
  IGCWriteRecord(temp);

  // Example: DG-300
  sprintf(temp,"HFGTYGLIDERTYPE:%S\r\n", AircraftType_Config);
  IGCWriteRecord(temp);

  // Example: D-7176
  sprintf(temp,"HFGIDGLIDERID:%S\r\n", AircraftRego_Config);
  IGCWriteRecord(temp);

  // 110117 TOCHECK: maybe a 8 char limit is needed. 
  sprintf(temp,"HFCCLCOMPETITIONCLASS:%S\r\n", CompetitionClass_Config);
  IGCWriteRecord(temp);

  sprintf(temp,"HFCIDCOMPETITIONID:%S\r\n", CompetitionID_Config);
  IGCWriteRecord(temp);

  // until LK is using xcsoar G signature, we keep XCSOAR as main logger type
  // The comma is currently declared invalid as a igc character, which is probably wrong

  #if USEXK8
    #ifndef LKCOMPETITION
  sprintf(temp,"HFFTYFRTYPE:%s\r\n", LKFORK); // default
    #else
  sprintf(temp,"HFFTYFRTYPE:%sC\r\n", LKFORK); // default
    #endif
  #else
    #ifndef LKCOMPETITION
  sprintf(temp,"HFFTYFRTYPE:XCSOAR %s\r\n", LKFORK); // default
    #else
  sprintf(temp,"HFFTYFRTYPE:XCSOAR %sC\r\n", LKFORK); // default
    #endif
  #endif

  // PNAs are also PPC2003, so careful
  #ifdef PNA
  char pnamodel[MAX_PATH+1];
  ConvertTToC(pnamodel,GlobalModelName);
  pnamodel[_tcslen(GlobalModelName)]='\0';
    #if USEXK8
    	#ifndef LKCOMPETITION
    sprintf(temp,"HFFTYFRTYPE:%s PNA %s\r\n", LKFORK,pnamodel);
	#else
    sprintf(temp,"HFFTYFRTYPE:%sC PNA %s\r\n", LKFORK,pnamodel);
	#endif
    #else
    	#ifndef LKCOMPETITION
    sprintf(temp,"HFFTYFRTYPE:XCSOAR %s PNA %s\r\n", LKFORK,pnamodel);
	#else
    sprintf(temp,"HFFTYFRTYPE:XCSOAR %sC PNA %s\r\n", LKFORK,pnamodel);
	#endif
    #endif
  #else

  #ifdef PPC2002 
    #if USEXK8
    	#ifndef LKCOMPETITION
    sprintf(temp,"HFFTYFRTYPE:%s PPC2002\r\n", LKFORK);
	#else
    sprintf(temp,"HFFTYFRTYPE:%sC PPC2002\r\n", LKFORK);
	#endif
    #else
    	#ifndef LKCOMPETITION
    sprintf(temp,"HFFTYFRTYPE:XCSOAR %s PPC2002\r\n", LKFORK);
	#else
    sprintf(temp,"HFFTYFRTYPE:XCSOAR %sC PPC2002\r\n", LKFORK);
	#endif
    #endif
  #endif
  // PNA is also PPC2003..
  #ifdef PPC2003 
    #if USEXK8
    	#ifndef LKCOMPETITION
    sprintf(temp,"HFFTYFRTYPE:%s PPC2003\r\n", LKFORK);
	#else
    sprintf(temp,"HFFTYFRTYPE:%sC PPC2003\r\n", LKFORK);
	#endif
    #else
    	#ifndef LKCOMPETITION
    sprintf(temp,"HFFTYFRTYPE:XCSOAR %s PPC2003\r\n", LKFORK);
	#else
    sprintf(temp,"HFFTYFRTYPE:XCSOAR %sC PPC2003\r\n", LKFORK);
	#endif
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
	wsprintf(wname,_T("%s"),WayPointList[HomeWaypoint].Name);
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
	wsprintf(wname,_T("%s"),WayPointList[HomeWaypoint].Name);
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
  _tcsupr(tmpstring);
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


bool ReplayLogger::ReadLine(TCHAR *buffer) {
  static FILE *fp = NULL;
  if (!buffer) {
    if (fp) {
      fclose(fp);
      fp= NULL;
    }
    return false;
  }
  if (!fp) {
    if (_tcslen(FileName)>0) {
      fp = _tfopen(FileName, TEXT("rt"));
    }
  }
  if (fp==NULL) {
    return false;
  }

  if (fgetws(buffer, 200, fp)==NULL) {
    _tcscat(buffer,TEXT("\0"));
    return false;
  }
  return true;
}


bool ReplayLogger::ScanBuffer(TCHAR *buffer, double *Time, double *Latitude,
			      double *Longitude, double *Altitude)
{
  int DegLat, DegLon;
  int MinLat, MinLon;
  TCHAR NoS, EoW;
  int iAltitude;
  int bAltitude;
  int Hour=0;
  int Minute=0;
  int Second=0;

  int lfound=0;
  int found=0;

  if ((lfound =
       swscanf(buffer,
	       TEXT("B%02d%02d%02d%02d%05d%c%03d%05d%cA%05d%05d"),
	       &Hour, &Minute, &Second,
	       &DegLat, &MinLat, &NoS, &DegLon, &MinLon,
	       &EoW, &iAltitude, &bAltitude
	       )) != EOF) {

    if (lfound==11) {
      *Latitude = DegLat+MinLat/60000.0;
      if (NoS==_T('S')) {
	*Latitude *= -1;
      }

      *Longitude = DegLon+MinLon/60000.0;
      if (EoW==_T('W')) {
	*Longitude *= -1;
      }
      if (bAltitude>0) *Altitude = bAltitude;
	else *Altitude = iAltitude;
      *Time = Hour*3600+Minute*60+Second;
    }
  }

  TCHAR event[200];
  TCHAR misc[200];

  found = _stscanf(buffer,
		   TEXT("LPLT event=%[^ ] %[A-Za-z0-9 \\/().,]"),
		   event,misc);
  if (found>0) {
    pt2Event fevent = InputEvents::findEvent(event);
    if (fevent) {
      if (found==2) {
	TCHAR *mmisc = StringMallocParse(misc);
	if (mmisc!=NULL) {
		fevent(mmisc);
		free(mmisc);
	}
      } else {
	fevent(TEXT("\0"));
      }
    }

  }
  return (lfound==11 && (*Latitude!=0 && *Longitude!=0) );
}


bool ReplayLogger::ReadPoint(double *Time,
			     double *Latitude,
			     double *Longitude,
			     double *Altitude)
{
  TCHAR buffer[200];
/*
  // This is creating problems with the interpolator and calculations based on differential times
  // such as variometer derived from altitude differences. Probably due to vario lowpass filters.
  while(ReadLine(buffer)) {
    if(ScanBuffer(buffer, Time, Latitude, Longitude, Altitude))
      return true;
  }
  return false;
*/
  bool found=false;
  while(ReadLine(buffer) &&!found) {
    if(ScanBuffer(buffer, Time, Latitude, Longitude, Altitude))
      found=true;
  }
  return found;
}



TCHAR ReplayLogger::FileName[MAX_PATH+1];
bool ReplayLogger::Enabled = false;
double ReplayLogger::TimeScale = 1.0;

bool ReplayLogger::IsEnabled(void) {
  return Enabled;
}


typedef struct _LOGGER_INTERP_POINT
{
  double lat;
  double lon;
  double alt;
  double t;
} LOGGER_INTERP_POINT;

/*
  ps = (1 u u^2 u^3)[0  1 0 0] p0
  [-t 0 t 0] p1
  [2t t-3 3-2t -t] p2
  [-t 2-t t-2 t] p3

*/

class CatmullRomInterpolator {
public:
  CatmullRomInterpolator() {
    Reset();
  }
  void Reset() {
    num=0;
    for (int i=0; i<4; i++) {
      p[i].t= 0;
    }
  }

  LOGGER_INTERP_POINT p[4];

  bool Update(double t, double lon, double lat, double alt) {
    if (num<4) { num++; }
	if ( p[3].t >t ) { 
		for (int i=0; i<4; i++) {
			p[i].lat = lat;
			p[i].lon = lon;
			p[i].alt = alt;
			p[i].t   = t;
		}
		return false;
	}

    for (int i=0; i<3; i++) {
      p[i].lat = p[i+1].lat;
      p[i].lon = p[i+1].lon;
      p[i].alt = p[i+1].alt;
      p[i].t   = p[i+1].t;
    }
    p[3].lat = lat;
    p[3].lon = lon;
    p[3].alt = alt;
    p[3].t   = t;

	return true; // 100827
  }

  bool Ready() {
    return (num==4);
  }
  double GetSpeed(double time) {
    if (p[1].t == p[0].t ) return 0.0;
    if (p[2].t == p[1].t ) return 0.0;
    if (Ready()) {
      double u= (time-p[1].t)/(p[2].t-p[1].t);
      double s0;
      DistanceBearing(p[0].lat, p[0].lon,
                      p[1].lat, p[1].lon, &s0, NULL);

      s0/= (p[1].t-p[0].t);
      double s1;
      DistanceBearing(p[1].lat, p[1].lon,
                      p[2].lat, p[2].lon, &s1, NULL);
      s1/= (p[2].t-p[1].t);
      u = max(0.0,min(1.0,u));
      return s1*u+s0*(1.0-u);
    } else {
      return 0.0;
    }
  }
  void Interpolate(double time, double *lon, double *lat, double *alt) {
    if (!Ready()) {
      *lon = p[num].lon;
      *lat = p[num].lat;
      *alt = p[num].alt;
      return;
    }
    double t=0.98;
    LKASSERT( p[2].t != p[1].t );
    double u= (time-p[1].t)/(p[2].t-p[1].t);

    if (u<0.0) {
      *lat = p[1].lat;
      *lon = p[1].lon;
      *alt = p[1].alt;
      return;
    }
    if (u>1.0) {
      *lat = p[2].lat;
      *lon = p[2].lon;
      *alt = p[2].alt;
      return;
    }

    double u2 = u*u;
    double u3 = u2*u;
    double c[4]= {-t*u3+2*t*u2-t*u,
                  (2-t)*u3+(t-3)*u2+1,
                  (t-2)*u3+(3-2*t)*u2+t*u,
                  t*u3-t*u2};
    /*
      double c[4] = {-t*u+2*t*u2-t*u3,
      1+(t-3)*u2+(2-t)*u3,
      t*u+(3-2*t)*u2+(t-2)*u3,
      -t*u2+t*u3};
    */

    *lat = (p[0].lat*c[0] + p[1].lat*c[1] + p[2].lat*c[2] + p[3].lat*c[3]);
    *lon = (p[0].lon*c[0] + p[1].lon*c[1] + p[2].lon*c[2] + p[3].lon*c[3]);
    *alt = (p[0].alt*c[0] + p[1].alt*c[1] + p[2].alt*c[2] + p[3].alt*c[3]);

  }
  double GetMinTime(void) {
    return p[0].t;
  }
  double GetMaxTime(void) {
    return max(0.0,max(p[0].t, max(p[1].t, max(p[2].t, p[3].t))));
  }
  double GetAverageTime(void) {
    double tav= 0;
    if (num>0) {
      for (int i=0; i<num; i++) {
        tav += p[i].t/num;
      }
    }
    return tav;
  }
  bool NeedData(double tthis) {
    return (!Ready())||(p[2].t<=tthis+0.1); 
  }
private:
  int num;
  double tzero;
};




bool ReplayLogger::UpdateInternal(void) {
  static bool init=true;

  if (!Enabled) {
    init = true;
    ReadLine(NULL); // close file
    Enabled = true;
  }

  static CatmullRomInterpolator cli;

  SYSTEMTIME st;
  GetLocalTime(&st);
  static double time_lstart = 0;

  if (init) {
    time_lstart = 0;
  }
  static double time=0;
  double deltatimereal;
  static double tthis=0;

  bool finished = false;

  double timelast = time;
  time = (st.wHour*3600+st.wMinute*60+st.wSecond-time_lstart);
  deltatimereal = time-timelast;

  if (init) {
    time_lstart = time;
    time = 0;
    deltatimereal = 0;
    tthis = 0;
    cli.Reset();
  }

  tthis += TimeScale*deltatimereal;

  double mintime = cli.GetMinTime(); // li_lat.GetMinTime();
  if (tthis<mintime) { tthis = mintime; }

  // if need a new point
  while (cli.NeedData(tthis)&&(!finished)) {

    double t1, Lat1, Lon1, Alt1;
    finished = !ReadPoint(&t1,&Lat1,&Lon1,&Alt1);

    if (!finished && (t1>0)) {
	if (!cli.Update(t1,Lon1,Lat1,Alt1)) { 
		break;
	}
    }
  }

  if (!finished) {

    double LatX, LonX, AltX, SpeedX, BearingX;
    double LatX1, LonX1, AltX1;

    cli.Interpolate(tthis, &LonX, &LatX, &AltX);
    cli.Interpolate(tthis+0.1, &LonX1, &LatX1, &AltX1);

    SpeedX = cli.GetSpeed(tthis);
    DistanceBearing(LatX, LonX, LatX1, LonX1, NULL, &BearingX);

    if ((SpeedX>0) && (LatX != LatX1) && (LonX != LonX1)) {

      LockFlightData();
      if (init) {
        flightstats.Reset();
      }
      GPS_INFO.Latitude = LatX;
      GPS_INFO.Longitude = LonX;
      GPS_INFO.Speed = SpeedX;
      GPS_INFO.TrackBearing = BearingX;
      GPS_INFO.Altitude = AltX;
      GPS_INFO.BaroAltitude = AltitudeToQNHAltitude(AltX);
      GPS_INFO.Time = tthis;
      UnlockFlightData();
    } else {
      // This is required in case the integrator fails,
      // which can occur due to parsing faults
      tthis = cli.GetMaxTime();
    }
  }

  // quit if finished.
  if (finished) {
    Stop();
  }
  init = false;

  return !finished;
}


void ReplayLogger::Stop(void) {
  ReadLine(NULL); // close the file
  if (Enabled) {
    LockFlightData();
    GPS_INFO.Speed = 0;
    //    GPS_INFO.Time = 0;
    NumLoggerBuffered = 0;
    UnlockFlightData();
  }
  Enabled = false;
}


void ReplayLogger::Start(void) {
  if (Enabled) {
    Stop();
  }
  NumLoggerBuffered = 0;
  flightstats.Reset();
  if (!UpdateInternal()) {
    MessageBoxX(hWndMapWindow,
	// LKTOKEN  _@M201_ = "Could not open IGC file!" 
		gettext(TEXT("_@M201_")),
	// LKTOKEN  _@M305_ = "Flight replay" 
		gettext(TEXT("_@M305_")),
		MB_OK| MB_ICONINFORMATION);
  }
}


TCHAR* ReplayLogger::GetFilename(void) {
  return FileName;
}


void ReplayLogger::SetFilename(TCHAR *name) {
  if (!name) {
    return;
  }
  if (_tcscmp(FileName,name)!=0) {
    _tcscpy(FileName,name);
  }
}

bool ReplayLogger::Update(void) {
  if (!Enabled)
    return false;

  Enabled = UpdateInternal();
  return Enabled;
}


FILETIME LogFileDate(TCHAR* filename) {
  FILETIME ft;
  ft.dwLowDateTime = 0;
  ft.dwHighDateTime = 0;

  TCHAR asset[MAX_PATH+1];
  SYSTEMTIME st;
  unsigned short year, month, day, num;
  int matches;
  // scan for long filename
  matches = swscanf(filename,
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


bool LogFileIsOlder(TCHAR *oldestname, TCHAR *thisname) {
  FILETIME ftold = LogFileDate(oldestname);
  FILETIME ftnew = LogFileDate(thisname);
  return (CompareFileTime(&ftold, &ftnew)>0);
}


bool DeleteOldIGCFile(TCHAR *pathname) {
  HANDLE hFind;  // file handle
  WIN32_FIND_DATA FindFileData;
  TCHAR oldestname[MAX_PATH+1];
  TCHAR searchpath[MAX_PATH+1];
  TCHAR fullname[MAX_PATH+1];
  _stprintf(searchpath, TEXT("%s*"),pathname);

  hFind = FindFirstFile(searchpath, &FindFileData); // find the first file
  if(hFind == INVALID_HANDLE_VALUE) {
    return false;
  }
  if(!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
    if (MatchesExtension(FindFileData.cFileName, TEXT(".igc")) ||
	MatchesExtension(FindFileData.cFileName, TEXT(".IGC"))) {
      // do something...
      _tcscpy(oldestname, FindFileData.cFileName);
    } else {
      return false;
    }
  }
  bool bSearch = true;
  while(bSearch) { // until we finds an entry
    if(FindNextFile(hFind,&FindFileData)) {
      if(!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
	 (MatchesExtension(FindFileData.cFileName, TEXT(".igc")) ||
	  (MatchesExtension(FindFileData.cFileName, TEXT(".IGC"))))) {
	if (LogFileIsOlder(oldestname,FindFileData.cFileName)) {
	  _tcscpy(oldestname, FindFileData.cFileName);
	  // we have a new oldest name
	}
      }
    } else {
      bSearch = false;
    }
  }
  FindClose(hFind);  // closing file handle

  // now, delete the file...
  _stprintf(fullname, TEXT("%s%s"),pathname,oldestname);
  DeleteFile(fullname);
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

bool IGCWriteRecord(char *szIn) 
{
  HANDLE hFile;
  DWORD dwBytesRead;
  char charbuffer[MAX_IGC_BUFF];
  bool bRetVal = false;

  static BOOL bWriting = false;

  // THIS IS NOT A SOLUTION. DATA LOSS GRANTED.
  if ( !bWriting )
    {
      bWriting = true;

      hFile = CreateFile(szLoggerFileName, GENERIC_WRITE, FILE_SHARE_WRITE,
			 NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
      SetFilePointer(hFile, 0, NULL, FILE_END);
    
      strcpy(charbuffer, CleanIGCRecord(szIn));

      WriteFile(hFile, charbuffer, strlen(charbuffer), &dwBytesRead,
		(OVERLAPPED *)NULL);


      FlushFileBuffers(hFile);
      CloseHandle(hFile);
      bWriting = false;
      bRetVal = true;
    }

  return bRetVal;

}


bool LoggerGActive()
{
  #if (WINDOWSPC>0)
    #if TESTBENCH
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

  DWORD retval=99;

  TCHAR homedir[MAX_PATH];
  TCHAR path[MAX_PATH];

  LocalPath(path,_T(LKD_LOGS));
  #if (WINDOWSPC>0)
  _tcscat(path,_T("\\LKRECORD_PC.LK8"));
  #endif

  // CAREFUL!!! PNA is ALSO PPC2003!!
  #ifdef PNA
  _tcscat(path,_T("\\LKRECORD_PNA.LK8"));
  #else
    #ifdef PPC2002
    _tcscat(path,_T("\\LKRECORD_2002.LK8"));
    #endif
    #ifdef PPC2003
    _tcscat(path,_T("\\LKRECORD_2003.LK8"));
    #endif
  #endif 

  LocalPath(homedir,TEXT(LKD_LOGS));

  #if TESTBENCH
  StartupStore(_T(".... RunSignature: homedir <%s>%s"),homedir,NEWLINE);
  #endif


  PROCESS_INFORMATION pi;

  #if (WINDOWSPC>0)
  // Sadly, some parameters cannot be passed in the CE version
  STARTUPINFO si;
  ZeroMemory(&si,sizeof(STARTUPINFO));
  si.cb=sizeof(STARTUPINFO);
  si.wShowWindow= SW_SHOWNORMAL;
  si.dwFlags = STARTF_USESHOWWINDOW;

  if (!::CreateProcess(path,homedir, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
  #else
  if (!::CreateProcess(path,homedir, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, NULL, &pi)) {
  #endif
	DWORD lasterr=GetLastError();

	if (lasterr!=2) {
		// External executable failure, bad !
		StartupStore(_T(".... RunSignature exec <%s> FAILED, error code=%d"),path,lasterr,NEWLINE);
		#if TESTBENCH
		StartupStore(_T(".... Trying with DoSignature\n"));
		#endif
	}

	#if TESTBENCH
	else 
		StartupStore(_T(".... no executable found, proceeding with DoSignature\n"));
	#endif

	extern int DoSignature(TCHAR *hpath);
	retval=DoSignature(homedir);

	return retval;
  }
  ::WaitForSingleObject(pi.hProcess, 30000); // 30s
  GetExitCodeProcess(pi.hProcess,&retval);
  // STILL_ACTIVE = 259, this retval should be checked for

  #if TESTBENCH
  StartupStore(_T(".... RunSignature exec terminated, retval=%d%s"),retval,NEWLINE);
  #endif


  return retval;

}

