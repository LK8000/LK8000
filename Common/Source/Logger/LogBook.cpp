/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Process.h"
#include "Logger.h"
#include "utils/heapcheck.h"

// fwrite adds a linefeed to each CR. We should not use \r\n with fwrite
#define WNEWLINE	"\n"

//
// Called by Calculations at landing detection (not flying anymore)
// and by WndProc on shutdown, in case we are still flying
//
void UpdateLogBook(bool welandedforsure) {

  #if TESTBENCH
  #else

  #if (WINDOWSPC>0)
  #else
  // Only in SIMMODE on pna/ppc we dont log 
  if (SIMMODE) return;
  #endif

  #endif // !testbench

  #if TESTBENCH
  StartupStore(_T("... UpdateLogBook start\n"));
  #endif

  // If we are called by WndProc then we might be still flying and we
  // must log. Otherwise if we are not flying, it means that we already
  // logged at landing detection, so we can return.
  if (!welandedforsure ) {
	if (!CALCULATED_INFO.Flying) {
		#if TESTBENCH
		StartupStore(_T("... Not flying, no reason to do logbook on exit\n"));
		#endif
		return;
	}
	#if TESTBENCH
	else
		StartupStore(_T("... Still flying! Do LogBook on exit!\n"));
	#endif
  }

  if (CALCULATED_INFO.FlightTime<=0) {
	#if TESTBENCH
	StartupStore(_T("... UpdateLogBook: flight-time is zero, no log!\n"));
	#endif
	return;
  }
  UpdateLogBookTXT(welandedforsure);
  UpdateLogBookCSV(welandedforsure);
  UpdateLogBookLST(welandedforsure);

}


// Returns false if something went wrong
bool UpdateLogBookTXT(bool welandedforsure) {

  FILE *stream;
  TCHAR filename[MAX_PATH];
  TCHAR Temp[300];
  char  line[300];
  int ivalue;

  wsprintf(filename,_T("%s\\%S\\%S"), LKGetLocalPath(), LKD_LOGS,LKF_LOGBOOKTXT);

  #if TESTBENCH
  StartupStore(_T("... UpdateLogBookTXT <%s>\n"),filename);
  #endif

  stream = _wfopen(filename,TEXT("a+"));
  if (stream == NULL) {
	StartupStore(_T(".... ERROR updating LogBookTXT, file open failure!%s"),NEWLINE);
	return false;
  }

  //
  // Header line for new note
  //
  sprintf(line,"[%04d-%02d-%02d  @%02d:%02d]%s",
	GPS_INFO.Year,
	GPS_INFO.Month,
	GPS_INFO.Day,
	GPS_INFO.Hour,
	GPS_INFO.Minute,WNEWLINE);
  fwrite(line,strlen(line),1,stream);

  if (SIMMODE) {
	sprintf(line,"%S%s",gettext(_T("_@M1211_")),WNEWLINE); // simulaion
	fwrite(line,strlen(line),1,stream);
  }

  //
  // D-1234 (Ka6-CR)
  //
  sprintf(line,"%S (%S)%s%s", AircraftRego_Config,AircraftType_Config,WNEWLINE,WNEWLINE);
  fwrite(line,strlen(line),1,stream);

  //
  // Takeoff time
  //
  Units::TimeToTextS(Temp,(int)TimeLocal((long)CALCULATED_INFO.TakeOffTime));
  sprintf(line,"%S: %S%s",gettext(_T("_@M680_")),Temp,WNEWLINE);
  fwrite(line,strlen(line),1,stream);

  //
  // Landing time
  //
  if (!CALCULATED_INFO.Flying || welandedforsure ) {
	Units::TimeToTextS(Temp,(int)TimeLocal((long)(CALCULATED_INFO.TakeOffTime+CALCULATED_INFO.FlightTime)));
	sprintf(line,"%S: %S%s",gettext(_T("_@M386_")),Temp,WNEWLINE);
  } else {
  	#if TESTBENCH
	StartupStore(_T(".... LogBookTXT, logging but still flying!%s"),NEWLINE);
	#endif
	sprintf(line,"%S: ??:??:??%s",gettext(_T("_@M386_")),WNEWLINE);
  }
  fwrite(line,strlen(line),1,stream);

  //
  // Flight time
  //
  Units::TimeToTextS(Temp, (int)CALCULATED_INFO.FlightTime);
  sprintf(line,"%S: %S%s%s",gettext(_T("_@M306_")),Temp,WNEWLINE,WNEWLINE);
  fwrite(line,strlen(line),1,stream);

  if (ISGLIDER || ISPARAGLIDER) {

	//
	// FREE FLIGHT DETECTED
	//
	if ( CALCULATED_INFO.FreeFlightStartTime>0 ) {
		Units::TimeToTextS(Temp, (int)CALCULATED_INFO.FreeFlightStartTime);
		sprintf(line,"%S: %S%s",gettext(_T("_@M1452_")),Temp,WNEWLINE);
		fwrite(line,strlen(line),1,stream);
	}

	//
	// OLC Classic Dist
	//
	ivalue=CContestMgr::TYPE_OLC_CLASSIC;
	if (OlcResults[ivalue].Type()!=CContestMgr::TYPE_INVALID) {
		_stprintf(Temp, TEXT("%5.0f"),DISTANCEMODIFY*OlcResults[ivalue].Distance());
		sprintf(line,"%S: %S %S%s",gettext(_T("_@M1455_")),Temp,(Units::GetDistanceName()),WNEWLINE);
		fwrite(line,strlen(line),1,stream);
	}

	//
	// OLC FAI Dist
	//
	ivalue=CContestMgr::TYPE_OLC_FAI;
	if (OlcResults[ivalue].Type()!=CContestMgr::TYPE_INVALID) {
		_stprintf(Temp, TEXT("%5.0f"),DISTANCEMODIFY*OlcResults[ivalue].Distance());
		sprintf(line,"%S: %S %S%s",gettext(_T("_@M1457_")),Temp,(Units::GetDistanceName()),WNEWLINE);
		fwrite(line,strlen(line),1,stream);
	}

	//
	// Max Altitude gained
	//
	sprintf(line,"%S: %.0f %S%s",gettext(_T("_@M1769_")),ALTITUDEMODIFY*CALCULATED_INFO.MaxHeightGain,(Units::GetAltitudeName()),WNEWLINE);
	fwrite(line,strlen(line),1,stream);
  }

  //
  // Max Altitude reached
  //
  sprintf(line,"%S: %.0f %S%s",gettext(_T("_@M1767_")),ALTITUDEMODIFY*CALCULATED_INFO.MaxAltitude,(Units::GetAltitudeName()),WNEWLINE);
  fwrite(line,strlen(line),1,stream);

  //
  // Odometer, add a spare CR LF to separate next logfield
  //
  sprintf(line,"%S: %.0f %S%s%s",gettext(_T("_@M1167_")),DISTANCEMODIFY*CALCULATED_INFO.Odometer,(Units::GetDistanceName()),WNEWLINE,WNEWLINE);
  fwrite(line,strlen(line),1,stream);


  fclose(stream);

  return true;

}

//
// Reset will apply only to TXT and LST, not to the CSV
//
void ResetLogBook(void) {

  TCHAR filename[MAX_PATH];

  #if TESTBENCH
  StartupStore(_T("... ResetLogBook <%s>\n"),filename);
  #endif

  wsprintf(filename,_T("%s\\%S\\%S"), LKGetLocalPath(), LKD_LOGS,LKF_LOGBOOKTXT);
  DeleteFile(filename);

  wsprintf(filename,_T("%s\\%S\\%S"), LKGetLocalPath(), LKD_LOGS,LKF_LOGBOOKLST);
  DeleteFile(filename);


  return;
}

//
// This is the comma separated value logbook, ready for excel and spreadsheets
//
bool UpdateLogBookCSV(bool welandedforsure) {

  FILE *stream;
  TCHAR filename[MAX_PATH];
  TCHAR Temp[300];
  char  line[300];
  int ivalue;
  bool dofirstline=false;
  char stakeoff[20],slanding[20],sflighttime[20], solcdist[20];

  wsprintf(filename,_T("%s\\%S\\%S"), LKGetLocalPath(), LKD_LOGS,LKF_LOGBOOKCSV);

  #if TESTBENCH
  StartupStore(_T("... UpdateLogBookCSV <%s>\n"),filename);
  #endif

  stream = _wfopen(filename,TEXT("r"));
  if (stream == NULL)
        dofirstline=true;
  else
        fclose(stream);

  stream = _wfopen(filename,TEXT("a+"));
  if (stream == NULL) {
	StartupStore(_T(".... ERROR updating LogBookCSV, file open failure!%s"),NEWLINE);
	return false;
  }

  if (dofirstline) {
	sprintf(line,"Year,Month,Day,AircraftRego,AircraftType,Takeoff,Landing,FlyTime,Odometer,OLCdist,DistUnits%s",WNEWLINE);
	fwrite(line,strlen(line),1,stream);
  }


  Units::TimeToTextS(Temp,(int)TimeLocal((long)CALCULATED_INFO.TakeOffTime));
  sprintf(stakeoff,"%S",Temp);

  if (!CALCULATED_INFO.Flying || welandedforsure) {
	Units::TimeToTextS(Temp,(int)TimeLocal((long)(CALCULATED_INFO.TakeOffTime+CALCULATED_INFO.FlightTime)));
	sprintf(slanding,"%S",Temp);
  } else {
	#if TESTBENCH
	StartupStore(_T(".... LogBookCSV, logging but still flying!%s"),NEWLINE);
	#endif
	sprintf(slanding,"???");
  }

  ivalue=CContestMgr::TYPE_OLC_CLASSIC;
  if (OlcResults[ivalue].Type()!=CContestMgr::TYPE_INVALID) {
	sprintf(solcdist, "%5.0f",DISTANCEMODIFY*OlcResults[ivalue].Distance());
  } else {
	sprintf(solcdist, "---");
  }

  Units::TimeToTextS(Temp, (int)CALCULATED_INFO.FlightTime);
  sprintf(sflighttime,"%S",Temp);

  char simmode[8];
  if (SIMMODE)
	strcpy(simmode,",SIM");
  else
	strcpy(simmode,"");

  sprintf(line,"%04d,%02d,%02d,%S,%S,%s,%s,%s,%d,%s,%S%s%s",
        GPS_INFO.Year,
        GPS_INFO.Month,
        GPS_INFO.Day,
	AircraftRego_Config,
	AircraftType_Config,
	stakeoff, slanding,sflighttime,
	(int)(DISTANCEMODIFY*CALCULATED_INFO.Odometer),
	solcdist,
	Units::GetDistanceName(),
	simmode, WNEWLINE
  );

  fwrite(line,strlen(line),1,stream);

  fclose(stream);
  return true;
}


//
// This is a simple text list of the logbook
//
bool UpdateLogBookLST(bool welandedforsure) {

  FILE *stream;
  TCHAR filename[MAX_PATH];
  TCHAR Temp[300];
  char  line[300];
  bool dofirstline=false;
  char stakeoff[20],slanding[20],sflighttime[20];

  wsprintf(filename,_T("%s\\%S\\%S"), LKGetLocalPath(), LKD_LOGS,LKF_LOGBOOKLST);

  #if TESTBENCH
  StartupStore(_T("... UpdateLogBookLST <%s>\n"),filename);
  #endif

  stream = _wfopen(filename,TEXT("r"));
  if (stream == NULL)
        dofirstline=true;
  else
        fclose(stream);

  stream = _wfopen(filename,TEXT("a+"));
  if (stream == NULL) {
	StartupStore(_T(".... ERROR updating LogBookLST, file open failure!%s"),NEWLINE);
	return false;
  }

  if (dofirstline) {
	sprintf(line,"[%S]%s",gettext(_T("_@M1753_")),WNEWLINE); // List of flights
	fwrite(line,strlen(line),1,stream);
	sprintf(line,"Date  Duration  (Takeoff  Landing)  Aircraft%s",WNEWLINE);
	fwrite(line,strlen(line),1,stream);
	sprintf(line,"__________________________________________________________________________________________________%s",WNEWLINE);
	fwrite(line,strlen(line),1,stream);
  }

  Units::TimeToTextS(Temp,(int)TimeLocal((long)CALCULATED_INFO.TakeOffTime));
  sprintf(stakeoff,"%S",Temp);

  if (!CALCULATED_INFO.Flying || welandedforsure) {
	Units::TimeToTextS(Temp,(int)TimeLocal((long)(CALCULATED_INFO.TakeOffTime+CALCULATED_INFO.FlightTime)));
	sprintf(slanding,"%S",Temp);
  } else {
	#if TESTBENCH
	StartupStore(_T(".... LogBookLST, logging but still flying!%s"),NEWLINE);
	#endif
	sprintf(slanding,"??:??:??");
  }

  Units::TimeToTextS(Temp, (int)CALCULATED_INFO.FlightTime);
  sprintf(sflighttime,"%S",Temp);

  char simmode[8];
  if (SIMMODE)
	strcpy(simmode,"  SIM!");
  else
	strcpy(simmode,"");

  sprintf(line,"%04d/%02d/%02d  %s (%s  %s)  %S%s%s",
        GPS_INFO.Year,
        GPS_INFO.Month,
        GPS_INFO.Day,
	sflighttime,stakeoff,slanding,
	AircraftRego_Config, simmode, WNEWLINE
  );

  fwrite(line,strlen(line),1,stream);

  fclose(stream);
  return true;
}


