/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Process.h"
#include "utils/heapcheck.h"

// Returns false if something went wrong
bool UpdateLogBookTXT(void) {

  FILE *stream;
  TCHAR filename[MAX_PATH];
  TCHAR Temp[300];
  char  line[300];
  int ivalue;

  wsprintf(filename,_T("%s\\%S\\%S"), LKGetLocalPath(), LKD_LOGS,LKF_LOGBOOKTXT);

  #if TESTBENCH
  StartupStore(_T("... UpdateLogBookTXT <%s>\n"),filename);
  #endif
  if (CALCULATED_INFO.FlightTime<=0) {
	#if TESTBENCH
	StartupStore(_T("... UpdateLogBookTXT: flight-time is zero!\n"),filename);
	#endif
	return true; // no problems, just a no-flight trigger
  }

  stream = _wfopen(filename,TEXT("a+"));
  if (stream == NULL) {
	StartupStore(_T(".... ERROR updating LogBookTXT, file open failure!%s"),NEWLINE);
	return false;
  }

  //
  // Header line for new note
  //
  sprintf(line,"\r\n[%04d-%02d-%02d  @%02d:%02d]\r\n",
	GPS_INFO.Year,
	GPS_INFO.Month,
	GPS_INFO.Day,
	GPS_INFO.Hour,
	GPS_INFO.Minute);
  fwrite(line,strlen(line),1,stream);

  //
  // D-1234 (Ka6-CR)
  //
  sprintf(line,"%S (%S)\r\n\r\n", AircraftRego_Config,AircraftType_Config);
  fwrite(line,strlen(line),1,stream);

  //
  // Takeoff time
  //
  Units::TimeToTextS(Temp,(int)TimeLocal((long)CALCULATED_INFO.TakeOffTime));
  sprintf(line,"%S: %S\r\n",gettext(_T("_@M680_")),Temp);
  fwrite(line,strlen(line),1,stream);

  //
  // Landing time
  //
  if (!CALCULATED_INFO.Flying) {
	Units::TimeToTextS(Temp,(int)TimeLocal((long)(CALCULATED_INFO.TakeOffTime+CALCULATED_INFO.FlightTime)));
	sprintf(line,"%S: %S\r\n",gettext(_T("_@M386_")),Temp);
  } else {
  	#if TESTBENCH
	StartupStore(_T(".... LogBookTXT, logging but still flying!%s"),NEWLINE);
	#endif
	sprintf(line,"%S: -------\r\n",gettext(_T("_@M386_")));
  }
  fwrite(line,strlen(line),1,stream);

  //
  // Flight time
  //
  Units::TimeToTextS(Temp, (int)CALCULATED_INFO.FlightTime);
  sprintf(line,"%S: %S\r\n\r\n",gettext(_T("_@M306_")),Temp);
  fwrite(line,strlen(line),1,stream);

  if (ISGLIDER || ISPARAGLIDER) {

	//
	// FREE FLIGHT DETECTED
	//
	if ( CALCULATED_INFO.FreeFlightStartTime>0 ) {
		Units::TimeToTextS(Temp, (int)CALCULATED_INFO.FreeFlightStartTime);
		sprintf(line,"%S: %S\r\n",gettext(_T("_@M1452_")),Temp);
		fwrite(line,strlen(line),1,stream);
	}

	//
	// OLC Classic Dist
	//
	ivalue=CContestMgr::TYPE_OLC_CLASSIC;
	if (OlcResults[ivalue].Type()!=CContestMgr::TYPE_INVALID) {
		_stprintf(Temp, TEXT("%5.0f"),DISTANCEMODIFY*OlcResults[ivalue].Distance());
		sprintf(line,"%S: %S %S\r\n",gettext(_T("_@M1455_")),Temp,(Units::GetDistanceName()));
		fwrite(line,strlen(line),1,stream);
	}

	//
	// OLC FAI Dist
	//
	ivalue=CContestMgr::TYPE_OLC_FAI;
	if (OlcResults[ivalue].Type()!=CContestMgr::TYPE_INVALID) {
		_stprintf(Temp, TEXT("%5.0f"),DISTANCEMODIFY*OlcResults[ivalue].Distance());
		sprintf(line,"%S: %S %S\r\n",gettext(_T("_@M1457_")),Temp,(Units::GetDistanceName()));
		fwrite(line,strlen(line),1,stream);
	}

	//
	// Max Altitude gained
	//
	sprintf(line,"%S: %.0f %S\r\n",gettext(_T("_@M1769_")),ALTITUDEMODIFY*CALCULATED_INFO.MaxHeightGain,(Units::GetAltitudeName()));
	fwrite(line,strlen(line),1,stream);
  }

  //
  // Max Altitude reached
  //
  sprintf(line,"%S: %.0f %S\r\n",gettext(_T("_@M1767_")),ALTITUDEMODIFY*CALCULATED_INFO.MaxAltitude,(Units::GetAltitudeName()));
  fwrite(line,strlen(line),1,stream);

  //
  // Odometer
  //
  sprintf(line,"%S: %.0f %S\r\n",gettext(_T("_@M1167_")),DISTANCEMODIFY*CALCULATED_INFO.Odometer,(Units::GetDistanceName()));
  fwrite(line,strlen(line),1,stream);


  fclose(stream);

  return true;

}



void ResetLogBook(void) {

  FILE *stream;
  TCHAR filename[MAX_PATH];

  wsprintf(filename,_T("%s\\%S\\%S"), LKGetLocalPath(), LKD_LOGS,LKF_LOGBOOKTXT);

  #if TESTBENCH
  StartupStore(_T("... ResetLogBook <%s>\n"),filename);
  #endif

  stream = _wfopen(filename,TEXT("w+"));
  if (stream == NULL) {
	StartupStore(_T(".... ERROR resetting LogBook, file open failure!%s"),NEWLINE);
	return;
  }
  fclose(stream);
  return;
}

//
// This is the comma separated value logbook, ready for excel and spreadsheets
//
bool UpdateLogBookCSV(void) {

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
  if (CALCULATED_INFO.FlightTime<=0) {
	#if TESTBENCH
	StartupStore(_T("... UpdateLogBookCSV: flight-time is zero!\n"),filename);
	#endif
	return true; // no problems, just a no-flight trigger
  }

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
	sprintf(line,"Year,Month,Day,AircraftRego,AircraftType,Takeoff,Landing,FlyTime,Odometer,OLCdist,DistUnits\r\n");
	fwrite(line,strlen(line),1,stream);
  }


  Units::TimeToTextS(Temp,(int)TimeLocal((long)CALCULATED_INFO.TakeOffTime));
  sprintf(stakeoff,"%S",Temp);

  if (!CALCULATED_INFO.Flying) {
	Units::TimeToTextS(Temp,(int)TimeLocal((long)(CALCULATED_INFO.TakeOffTime+CALCULATED_INFO.FlightTime)));
	sprintf(slanding,"%S",Temp);
  } else {
	#if TESTBENCH
	StartupStore(_T(".... LogBookCSV, logging but still flying!%s"),NEWLINE);
	#endif
	sprintf(slanding,"---");
  }

  ivalue=CContestMgr::TYPE_OLC_CLASSIC;
  if (OlcResults[ivalue].Type()!=CContestMgr::TYPE_INVALID) {
	sprintf(solcdist, "%5.0f",DISTANCEMODIFY*OlcResults[ivalue].Distance());
  } else {
	sprintf(solcdist, "---");
  }

  Units::TimeToTextS(Temp, (int)CALCULATED_INFO.FlightTime);
  sprintf(sflighttime,"%S",Temp);

  sprintf(line,"%04d,%02d,%02d,%S,%S,%s,%s,%s,%d,%s,%S\r\n",
        GPS_INFO.Year,
        GPS_INFO.Month,
        GPS_INFO.Day,
	AircraftRego_Config,
	AircraftType_Config,
	stakeoff, slanding,sflighttime,
	(int)(DISTANCEMODIFY*CALCULATED_INFO.Odometer),
	solcdist,
	Units::GetDistanceName()
  );

  fwrite(line,strlen(line),1,stream);

  fclose(stream);
  return true;
}
