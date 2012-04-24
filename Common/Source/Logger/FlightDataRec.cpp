/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "FlightDataRec.h"

#define NO_ENTRYS 22

FILE *FlightDataRecorderFile=NULL;

static unsigned int iLogDelay =0;	// if 0 we skip any logging activity
bool FlightDataRecorderActive=false;	// if false, we skip any FlightRecorder activity, including warnings and logging

sFlightDataRec FDR[NO_ENTRYS];


//
// Called once at startup by WinMain from lk8000.cpp 
// This means that we can also give a message about FDR operative even in the splash screens
// In case of re-run, it is self-checking for previous successful run
//
void InitFlightDataRecorder(void)
{

  // Did we already found a config? If so, dont do anything else.
  if (FlightDataRecorderActive) {
	#if TESTBENCH
	StartupStore(_T("... InitFDR error: already initialised!\n"));
	#endif
	return;
  }

  TCHAR path[MAX_PATH+1];
  TCHAR szBatLogFileName[MAX_PATH+1];
  char  szTmp[255];
  FILE* fpDataRecConfigFile = NULL;

  int i;

  LocalPath(path,TEXT(LKD_CONF));
  wsprintf(szBatLogFileName, TEXT("%s\\FlightRecorder.CFG"), path);
  fpDataRecConfigFile = _tfopen(szBatLogFileName, TEXT("r"));

  if(fpDataRecConfigFile == NULL)
  {
	// We shall no more check for dataconfig, unless we reset DoInit
	// So now the FlightDataRecorder is disabled at all, and we shall only
	// check for FlightDataRecorderActive to be true 
	#if TESTBENCH
	StartupStore(_T("... InitFDR: no configuration, FDR disabled\n"));
	#endif
	return;
  }

  StartupStore(_T(". Flight Data Recorder is in use%s"),NEWLINE);

  FlightDataRecorderActive=true;

  // Reset configuration
  for(i=0 ; i < NO_ENTRYS; i++)
  {
	FDR[i].abLog = 0 ;FDR[i].fMin =0.0 ; FDR[i].fMax = 0.0;FDR[i].aiCheckInterval = 0; FDR[i].aiWarningCnt= 0;
  }

  // Load new configuration
  i=0;  iLogDelay=0;
  do
  {
	i++;
	fscanf(fpDataRecConfigFile, "%120[^\n]",szTmp);
	iLogDelay = atoi(szTmp);

  } while ((iLogDelay == 0) && (i< 50));

  for(i= 0 ; i < NO_ENTRYS; i++) {
	fscanf(fpDataRecConfigFile, "%d %f %f %i %i %120[^\n]", 
		&FDR[i].abLog ,&FDR[i].fMin , &FDR[i].fMax , &FDR[i].aiCheckInterval, &FDR[i].aiWarningCnt, szTmp );
  }

  fclose(fpDataRecConfigFile);


  i=0;
  _tcscpy( FDR[i++].szName , _T("External Batt 1"));
  _tcscpy( FDR[i++].szName , _T("External Batt 2"));
  _tcscpy( FDR[i++].szName , _T("supply  voltage"));
  _tcscpy( FDR[i++].szName , _T("Batt %"));
  _tcscpy( FDR[i++].szName , _T("Outside Air Temperature"));
  _tcscpy( FDR[i++].szName , _T("Longitude"));
  _tcscpy( FDR[i++].szName , _T("Latitude"));
  _tcscpy( FDR[i++].szName , _T("Altitude"));
  _tcscpy( FDR[i++].szName , _T("Baro Altitude"));
  _tcscpy( FDR[i++].szName , _T("Alt AGL"));
  _tcscpy( FDR[i++].szName , _T("Speed"));
  _tcscpy( FDR[i++].szName , _T("Indcated Airspeed"));
  _tcscpy( FDR[i++].szName , _T("TrackBearing"));
  _tcscpy( FDR[i++].szName , _T("Vario"));
  _tcscpy( FDR[i++].szName , _T("NettoVario"));

  _tcscpy( FDR[i++].szName , _T("TrueAirspeed"));
  _tcscpy( FDR[i++].szName , _T("Acceleration X"));
  _tcscpy( FDR[i++].szName , _T("Acceleration Y"));
  _tcscpy( FDR[i++].szName , _T("Acceleration Z"));
  _tcscpy( FDR[i++].szName , _T("Ballast"));
  _tcscpy( FDR[i++].szName , _T("Bugs"));
  _tcscpy( FDR[i++].szName , _T("MacReady"));


  if(iLogDelay == 0) return;  // Nothing to do

  char sbuf[30];

  // WE TRY TO OPEN THE LOGFILE, AND IF WE CANT WE PERMANENTLY DISABLE LOGGING
  LocalPath(path,TEXT(LKD_LOGS));
  wsprintf(szBatLogFileName, TEXT("%s\\FlightRecorder.TXT"), path);
  FlightDataRecorderFile = _tfopen(szBatLogFileName, TEXT("w"));

  if (FlightDataRecorderFile==NULL) {
	StartupStore(_T("... InitFDR failure: cannot open <%s>%s"),szBatLogFileName,NEWLINE);
	iLogDelay=0;	 // logging disabled permanently
  	return;
  } 

  // FROM NOW ON, we can write on the file and on LK exit we must close the file.

  fprintf(FlightDataRecorderFile,"Recording interval:%ds \r",iLogDelay);

  for( i = 0;  i < NO_ENTRYS; i++)
  {
	unicode2utf(FDR[i].szName, sbuf, sizeof(sbuf));
	if(FDR[i].abLog > 0)
		fprintf(FlightDataRecorderFile,"%30s recording enabled\r",sbuf);
  }
  fprintf(FlightDataRecorderFile,"\r");

  for( i = 0;  i < NO_ENTRYS; i++)
  {
	unicode2utf(FDR[i].szName, sbuf, sizeof(sbuf));
	if(FDR[i].aiCheckInterval > 0)
	{
		if( FDR[i].aiWarningCnt > 0)
		{
			fprintf(FlightDataRecorderFile,"%30s range (%4.2f .. %4.2f) check every %is: max. %i warnings\r", 
				sbuf,FDR[i].fMin,  FDR[i].fMax,FDR[i].aiCheckInterval,  FDR[i].aiWarningCnt );
		}
		else
		{
			fprintf(FlightDataRecorderFile,"%30s range (%4.2f .. %4.2f) check every %is: unlimited warnings!\r", 
				sbuf,FDR[i].fMin,  FDR[i].fMax,FDR[i].aiCheckInterval);
		}
	}
  }

  fprintf(FlightDataRecorderFile,"\r");
  fprintf(FlightDataRecorderFile,"hh:mm:ss ");

  int idx=0;
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," BAT1 " );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  BAT2 " );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," IntV " );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," BAT%% " );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," OAT  " );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," lat       " );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," lon       " );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," Alt "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," AltB "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  AGL "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  AS "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," IAS "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," BRG " );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," VAR "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  NET "  );

  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  TAS "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  AcX "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  AcY "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  AcZ "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  BAL " );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  BUG "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  MC  "  );

  fprintf(FlightDataRecorderFile,"\r"  );

  // Sync file, only for init
  fflush(FlightDataRecorderFile);
  
}



//
// Called by Thread_Calculation when FlighDataRecorderActive
// These values are thread copied inside Calc Thread. No need to lock.
//
void UpdateFlightDataRecorder(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  static unsigned int iCallCnt = 0;

  int idx=0;
  LKASSERT(iLogDelay<32767);

  if ((iLogDelay==0) || (++iCallCnt<iLogDelay)) return;
  iCallCnt=0;

  if (FlightDataRecorderFile==NULL) return;

  float fValue[NO_ENTRYS];

  fValue[idx++] = Basic->ExtBatt1_Voltage;
  fValue[idx++] = Basic->ExtBatt2_Voltage;
  fValue[idx++] = Basic->SupplyBatteryVoltage;
  fValue[idx++] = PDABatteryPercent;
  fValue[idx++] = Basic->OutsideAirTemperature;
  fValue[idx++] = Basic->Latitude;
  fValue[idx++] = Basic->Longitude;
  fValue[idx++] = Basic->Altitude;
  fValue[idx++] = Basic->BaroAltitude;
  fValue[idx++] = Calculated->AltitudeAGL;
  fValue[idx++] = Basic->Speed;
  fValue[idx++] = Basic->IndicatedAirspeed;
  fValue[idx++] = Basic->TrackBearing;
  fValue[idx++] = Basic->Vario;
  fValue[idx++] = Basic->NettoVario;

  fValue[idx++] = Basic->TrueAirspeed ;
  fValue[idx++] = Basic->AccelX ;
  fValue[idx++] = Basic->AccelY ;
  fValue[idx++] = Basic->AccelZ ;
  fValue[idx++] = Basic->Ballast ;
  fValue[idx++] = Basic->Bugs ;
  fValue[idx++] = Basic->MacReady ;

//	fValue[20] = NMEA_INFO.SpeedToFly();
  int Hour   = Basic->Hour;
  int Min    = Basic->Minute;
  int Sec    = Basic->Second;

  // xULLI: WHAT ABOUT USING
  // if(FDR[0].abLog > 0) fprintf(FlightDataRecorderFile," %4.2f ",  Basic->ExtBatt1_Voltage);
  // if(FDR[1].abLog > 0) fprintf(FlightDataRecorderFile," %4.2f ",  Basic->ExtBatt2_Voltage);
  // etc.etc.
  // 

  // Shutdown will set LogDelay to zero before closing the file descriptor
  if (iLogDelay!=0) fprintf(FlightDataRecorderFile,"%02d:%02d:%02d ", Hour,  Min,  Sec  );

  idx=-1;
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %4.2f ",  fValue[idx]  );// GPS_INFO.ExtBatt1_Voltage;
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %4.2f ",  fValue[idx]  );// GPS_INFO.ExtBatt2_Voltage;
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %4.2f ",  fValue[idx]  );// GPS_INFO.SupplyBatteryVoltage;
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %03d " , (int) fValue[idx]  );// GPS_INFO.PDABatteryPercent;
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %4.2f ",  fValue[idx]  );// GPS_INFO.OutsideAirTemperature;
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %f ",     fValue[idx]  );// GPS_INFO.Latitude;
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %f ",     fValue[idx]  );// GPS_INFO.Longitude;
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %4.0f ",  fValue[idx]  );// GPS_INFO.Altitude;
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %4.0f ",  fValue[idx]  );// GPS_INFO.BaroAltitude;
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %4.0f ",  fValue[idx]  );// CALCULATED_INFO.AltitudeAGL;
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %3.0f ",  fValue[idx]  );// GPS_INFO.Speed;
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %3.0f ",  fValue[idx] );// GPS_INFO.IndicatedAirspeed;
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %3.0f ",  fValue[idx] );// GPS_INFO.TrackBearing;
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %4.2f ",  fValue[idx] );// GPS_INFO.Vario;
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %4.2f ",  fValue[idx] );// GPS_INFO.NettoVario;

  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %4.0f ", fValue[idx]  );// GPS_INFO.TrueAirspeed
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %4.2f ", fValue[idx]  );// GPS_INFO.AccelX
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %4.2f ", fValue[idx]  );// GPS_INFO.AccelY
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %4.2f ", fValue[idx]  );// GPS_INFO.AccelZ
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %4.0f ", fValue[idx]  );// GPS_INFO.Ballast
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %4.0f ", fValue[idx]  );// GPS_INFO.Bugs
  if(FDR[++idx].abLog > 0) fprintf(FlightDataRecorderFile," %4.2f ", fValue[idx]  );// GPS_INFO.MacReady
  if (iLogDelay!=0) fprintf(FlightDataRecorderFile,"\r"); /* next line */

}


//
// Check ranges and generate warnings
//
// This is not really a fligh recorder function. We are only sharing flight recorder configuration.
// Since the scope of this function is to give only warnings, we call this function from Draw thread
// which is the place where we do print warnings.
// No need to lock, because we are using copied data, already thread safe.
// Function called by RenderMapWindow   in the Draw thread (twice, see RenderMapWindow)
//
// It is possible to draw directly on the screen, instead of using DoStatusMessage. See DrawLKAlarms.
//
void MapWindow::DrawFDRAlarms(HDC hDC, const RECT rc)
{
  static unsigned short iCallCnt=0;
  int i;
  TCHAR szTmp[80];
  float fValue[NO_ENTRYS];

  // Do not give warnings during the first minute of life
  if(iCallCnt++ <= 60) return;

  // Alarms are working only with a valid GPS fix. No navigator, no alarms. 
  // if (GPS_INFO.NAVWarning) return;


  // No need to lock data, we are using DrawInfo and DerivedDrawInfo, already copied for MapWindow

  i=0;

  fValue[i++] = DrawInfo.ExtBatt1_Voltage;
  fValue[i++] = DrawInfo.ExtBatt2_Voltage;
  fValue[i++] = DrawInfo.SupplyBatteryVoltage;
  fValue[i++] = PDABatteryPercent;
  fValue[i++] = DrawInfo.OutsideAirTemperature;
  fValue[i++] = DrawInfo.Latitude;
  fValue[i++] = DrawInfo.Longitude;
  fValue[i++] = DrawInfo.Altitude;
  fValue[i++] = DrawInfo.BaroAltitude;
  fValue[i++] = DerivedDrawInfo.AltitudeAGL;
  fValue[i++] = DrawInfo.Speed;
  fValue[i++] = DrawInfo.IndicatedAirspeed;
  fValue[i++] = DrawInfo.TrackBearing;
  fValue[i++] = DrawInfo.Vario;
  fValue[i++] = DrawInfo.NettoVario;

  fValue[i++] = DrawInfo.TrueAirspeed ;
  fValue[i++] = DrawInfo.AccelX ;
  fValue[i++] = DrawInfo.AccelY ;
  fValue[i++] = DrawInfo.AccelZ ;
  fValue[i++] = DrawInfo.Ballast ;
  fValue[i++] = DrawInfo.Bugs ;
  fValue[i++] = DrawInfo.MacReady ;

  
  //
  // WARNING> if a value is going up and down the threshold, we should avoid repeating the 
  // message to the pilot. See DrawLKAlarms for an example on how the altitude alarms work
  //
  for(i=0 ; i < NO_ENTRYS; i++)
  {
	if(FDR[i].aiWarningCnt > 0)  // check enabled ? 
	   if((iCallCnt%FDR[i].aiCheckInterval)==0)	// check every ? sec
	     if((FDR[i].aiWarningCnt < FDR[i].aiWarningCnt) ||(FDR[i].aiWarningCnt==0)) /* still warnings left? */
	     {
		if (fValue[i] < FDR[i].fMin)
		{
			_stprintf(szTmp,_T("%s: (%4.2f < %4.2f)"), FDR[i].szName,fValue[i] , FDR[i].fMin);
			DoStatusMessage(szTmp);
			FDR[i].aiWarningCnt++;
		}

		if (fValue[i] > FDR[i].fMax)
		{
			_stprintf(szTmp,_T("%s: (%4.2 > %4.2f)"), FDR[i].szName,fValue[i] , FDR[i].fMax);
			DoStatusMessage(szTmp);
			FDR[i].aiWarningCnt++;
		}
	    }
  }

}


//
// We call the Close from WndProc, in the Shutdown procedure.
// This will tell Draw thread and Calc thread to stop bothering about FDR
//
void CloseFlightDataRecorder(void)
{
  if (!FlightDataRecorderActive) return;

  FlightDataRecorderActive=false;
  #if TESTBENCH
  StartupStore(_T("... Closing Flight Data Recorder\n"));
  #endif
  iLogDelay=0;
  if (FlightDataRecorderFile) fclose(FlightDataRecorderFile);
}
