/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "FlightDataRec.h"
#include "utils/stringext.h"

#define NO_ENTRYS 26

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
	FDR[i].abLog = 0 ;FDR[i].fMin =0.0 ; FDR[i].fMax = 0.0;FDR[i].aiCheckInterval = 0; FDR[i].aiMaxWarnings= 0; FDR[i].iWarningDelay=0;
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
		&FDR[i].abLog ,&FDR[i].fMin , &FDR[i].fMax , &FDR[i].aiCheckInterval, &FDR[i].aiMaxWarnings, szTmp );
  }

  fclose(fpDataRecConfigFile);


  i=0;
  _tcscpy( FDR[i++].szName , _T("External Batt. 1"));
  _tcscpy( FDR[i++].szName , _T("External Batt. 2"));
  _tcscpy( FDR[i++].szName , _T("supply  voltage"));
  _tcscpy( FDR[i++].szName , _T("PDA Batt. %"));
  _tcscpy( FDR[i++].szName , _T("Outside Air Temperature"));
  _tcscpy( FDR[i++].szName , _T("Longitude"));
  _tcscpy( FDR[i++].szName , _T("Latitude"));
  _tcscpy( FDR[i++].szName , _T("Altitude"));
  _tcscpy( FDR[i++].szName , _T("Baro Altitude"));
  _tcscpy( FDR[i++].szName , _T("Alt AGL"));
  _tcscpy( FDR[i++].szName , _T("Indcated Airspeed"));
  _tcscpy( FDR[i++].szName , _T("True Airspeed"));
  _tcscpy( FDR[i++].szName , _T("Ground Speed"));
  _tcscpy( FDR[i++].szName , _T("TrackBearing"));
  _tcscpy( FDR[i++].szName , _T("Vario"));
  _tcscpy( FDR[i++].szName , _T("NettoVario"));


  _tcscpy( FDR[i++].szName , _T("Acceleration X"));
  _tcscpy( FDR[i++].szName , _T("Acceleration Y"));
  _tcscpy( FDR[i++].szName , _T("Acceleration Z"));
  _tcscpy( FDR[i++].szName , _T("Ballast"));
  _tcscpy( FDR[i++].szName , _T("Bugs"));
  _tcscpy( FDR[i++].szName , _T("MacReady"));
  _tcscpy( FDR[i++].szName , _T("Wind speed"));
  _tcscpy( FDR[i++].szName , _T("Wind direction"));

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

  SYSTEMTIME pda_time;
  GetSystemTime(&pda_time);
  // FROM NOW ON, we can write on the file and on LK exit we must close the file.
  fprintf(FlightDataRecorderFile,"******************************************************************\r");
  fprintf(FlightDataRecorderFile,"* LK8000 Tactical Flight Computer -  WWW.LK8000.IT\r");
  fprintf(FlightDataRecorderFile,"*\r");
  fprintf(FlightDataRecorderFile,"* Flight Data Recorder Output\r");
  fprintf(FlightDataRecorderFile,"* GNU 2012 by Ulrich Heynen / Paolo Ventafridda\r");
  fprintf(FlightDataRecorderFile,"*\r");
  fprintf(FlightDataRecorderFile,"* flight recorded on: %02d:%02d:%04d starting at %02d:%02d:%02d UTC\r", pda_time.wDay,pda_time.wMonth,pda_time.wYear , pda_time.wHour,  pda_time.wMinute,  pda_time.wSecond  );
  fprintf(FlightDataRecorderFile,"*\r");
  fprintf(FlightDataRecorderFile,"******************************************************************\r\r");

  fprintf(FlightDataRecorderFile,"Recording interval:%us \r\r",iLogDelay);

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
		if( FDR[i].aiMaxWarnings > 0)
		{
		  fprintf(FlightDataRecorderFile,"%30s range (%4.2f .. %4.2f) warning every %is, max. %i warnings\r",
				sbuf,FDR[i].fMin,  FDR[i].fMax,FDR[i].aiCheckInterval,  FDR[i].aiMaxWarnings );
		}
		else
		{
		  fprintf(FlightDataRecorderFile,"%30s range (%4.2f .. %4.2f) check every %is, unlimited warnings!\r",
				sbuf,FDR[i].fMin,  FDR[i].fMax,FDR[i].aiMaxWarnings);
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
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," IAS "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  TAS "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  GS "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," BRG " );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  VAR  "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  NET  "  );

  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  AcX "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  AcY "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  AcZ "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  BAL " );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  BUG "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile,"  MC  "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," eWnd "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," eWdir"  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," cWnd "  );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," cWdir"  );

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

  static double nextHB=0;
  if (LKHearthBeats < nextHB) return;
  nextHB=LKHearthBeats+2;       // 2hz to 1hz

  SYSTEMTIME pda_time;
  GetSystemTime(&pda_time);

  int idx=0;
  LKASSERT(iLogDelay<32767);

  if ((iLogDelay==0) || (++iCallCnt<iLogDelay)) return;
  iCallCnt=0;

  if (FlightDataRecorderFile==NULL) return;

  // Shutdown will set LogDelay to zero before closing the file descriptor

  if (iLogDelay!=0) fprintf(FlightDataRecorderFile,"%02d:%02d:%02d ", pda_time.wHour,  pda_time.wMinute,  pda_time.wSecond  );

  idx=0;
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %5.2f ",  Basic->ExtBatt1_Voltage     );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %5.2f ",  Basic->ExtBatt2_Voltage     );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %5.2f ",  Basic->SupplyBatteryVoltage );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %03d " , (int)  PDABatteryPercent     );
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %4.2f ",  Basic->OutsideAirTemperature);
  if(FDR[idx++].abLog > 0)
  {
	if (Basic->NAVWarning)
      fprintf(FlightDataRecorderFile," no fix    ");
	else
	  fprintf(FlightDataRecorderFile," %f ",     Basic->Latitude  );// GPS_INFO.Latitude;
  }
  if(FDR[idx++].abLog > 0)
  {
	if (Basic->NAVWarning)
	  fprintf(FlightDataRecorderFile," no fix    ");
	else
	  fprintf(FlightDataRecorderFile," %f ",     Basic->Longitude );// Longitude;

  }
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %4.0f ",  Basic->Altitude             );// GPS_INFO.Altitude;
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %4.0f ",  Basic->BaroAltitude         );// GPS_INFO.BaroAltitude;
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %4.0f ",  Calculated->AltitudeAGL );// CALCULATED_INFO.AltitudeAGL;
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %3.0f ",  Basic->IndicatedAirspeed *TOKPH  );// GPS_INFO.IndicatedAirspeed;
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %4.0f ",  Basic->TrueAirspeed *TOKPH  );// GPS_INFO.TrueAirspeed
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %3.0f ",  Basic->Speed *TOKPH         );// GPS_INFO.Speed;
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %3.0f ",  Basic->TrackBearing         );// GPS_INFO.TrackBearing;
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %5.2f ",  Basic->Vario                );// GPS_INFO.Vario;
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %5.2f ",  Basic->NettoVario           );// GPS_INFO.NettoVario;
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %4.1f ",  Basic->AccelX               );// GPS_INFO.AccelX
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %4.1f ",  Basic->AccelY               );// GPS_INFO.AccelY
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %4.1f ",  Basic->AccelZ               );// GPS_INFO.AccelZ
#define GLOBAL_MC
#ifdef GLOBAL_MC
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %4.0f ",  BALLAST*100.0               );// GPS_INFO.Ballast
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %4.0f ",  BUGS*100.0                  );// GPS_INFO.Bugs
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %4.2f ",  MACCREADY                   );// GPS_INFO.MacReady
#else
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %4.0f ",   Basic->Ballast *100.0      );// GPS_INFO.Ballast
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %4.0f ",   Basic->Bugs *100.0         );// GPS_INFO.Bugs
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %4.2f ",   Basic->MacReady            );// GPS_INFO.MacReady
#endif
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %4.0f ",  Basic->ExternalWindSpeed *TOKPH );// GPS_INFO.Bugs
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %4.0f ",  Basic->ExternalWindDirection);// GPS_INFO.MacReady
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %4.0f ",  Calculated->WindSpeed *TOKPH );// GPS_INFO.Bugs
  if(FDR[idx++].abLog > 0) fprintf(FlightDataRecorderFile," %4.0f ",  Calculated->WindBearing );// GPS_INFO.MacReady



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
// This function is called by DrawFunctions1HZ  once per second max.
//
void MapWindow::DrawFDRAlarms(HDC hDC, const RECT rc)
{

  int i;
  TCHAR szTmp[80];
  float fValue[NO_ENTRYS];

  // Do not give warnings during the first minute of life
  if(LKHearthBeats <= 150) return;

  // Alarms are working only with a valid GPS fix. No navigator, no alarms. 
  // if (GPS_INFO.NAVWarning) return;


  // No need to lock data, we are using DrawInfo and DerivedDrawInfo, already copied for MapWindow

  i=0;

  fValue[i++] = DrawInfo.ExtBatt1_Voltage;
  fValue[i++] = DrawInfo.ExtBatt2_Voltage;
  fValue[i++] = DrawInfo.SupplyBatteryVoltage;
  fValue[i++] = PDABatteryPercent;
  fValue[i++] = DrawInfo.OutsideAirTemperature;
  if (DrawInfo.NAVWarning) fValue[i++] = 0.0; else fValue[i++] = DrawInfo.Latitude;
  if (DrawInfo.NAVWarning) fValue[i++] = 0.0; else fValue[i++] = DrawInfo.Longitude;
  fValue[i++] = DrawInfo.Altitude;
  fValue[i++] = DrawInfo.BaroAltitude;
  fValue[i++] = DerivedDrawInfo.AltitudeAGL;
  fValue[i++] = DrawInfo.IndicatedAirspeed   ;
  fValue[i++] = DrawInfo.TrueAirspeed *TOKPH;
  fValue[i++] = DrawInfo.Speed  *TOKPH  ;

  fValue[i++] = DrawInfo.TrackBearing;
  fValue[i++] = DrawInfo.Vario;
  fValue[i++] = DrawInfo.NettoVario;
  fValue[i++] = DrawInfo.AccelX ;
  fValue[i++] = DrawInfo.AccelY ;
  fValue[i++] = DrawInfo.AccelZ ;

#ifdef GLOBAL_MC
  fValue[i++] = BALLAST*100.0 ;
  fValue[i++] = BUGS*100.0  ;
  fValue[i++] = MACCREADY ;
#else
  fValue[i++] = DrawInfo.Ballast*100.0;
  fValue[i++] = DrawInfo.Bugs*100.0;
  fValue[i++] = DrawInfo.MacReady;
#endif

  fValue[i++] = DrawInfo.ExternalWindSpeed * TOKPH;
  fValue[i++] = DrawInfo.ExternalWindDirection ;
  fValue[i++] = DerivedDrawInfo.WindSpeed *TOKPH ;
  fValue[i++] = DerivedDrawInfo.WindBearing ;

  //
  // WARNING> if a value is going up and down the threshold, we should avoid repeating the 
  // message to the pilot. See DrawLKAlarms for an example on how the altitude alarms work
  //
  // ANSWER ULLI:
  // we have a different approach, after the first warning, the next check will be after the user defined
  // delay time. I think this is the best approach since there are also values without "Noise"
#define UNDER_WARNED  -100
#define OVER_WARNED   -101
#define ARMED            0

  for(i=0 ; i < NO_ENTRYS; i++)
  {
	if(FDR[i].aiCheckInterval != 0)  // check enabled ?
	{
	  if( FDR[i].iWarningDelay > ARMED )
		FDR[i].iWarningDelay--;
	  else
	  {
	    if((FDR[i].aiWarningCnt < FDR[i].aiMaxWarnings) ||(FDR[i].aiMaxWarnings==0)) /* still warnings left? or all warnings*/
	    {

	      if (fValue[i] < FDR[i].fMin)
	      {
	    	if( FDR[i].iWarningDelay  == ARMED)
	    	{
	          _stprintf(szTmp,_T("%s: (%4.2f < %4.2f)"), FDR[i].szName,fValue[i] , FDR[i].fMin);
              DoStatusMessage(szTmp);
			  FDR[i].aiWarningCnt++;
			  if(FDR[i].aiCheckInterval >0)
			    FDR[i].iWarningDelay = FDR[i].aiCheckInterval-1;
			  else
			    FDR[i].iWarningDelay = UNDER_WARNED ;
	    	}
	    	if(FDR[i].iWarningDelay  == OVER_WARNED)
	    	  FDR[i].iWarningDelay = ARMED;
		  }

		  if (fValue[i] > FDR[i].fMax)
		  {
		    if( FDR[i].iWarningDelay  == ARMED)
		    {
			  _stprintf(szTmp,_T("%s: (%4.2f > %4.2f)"), FDR[i].szName,fValue[i] , FDR[i].fMax);
			  DoStatusMessage(szTmp);
			  FDR[i].aiWarningCnt++;
			  if(FDR[i].aiCheckInterval >0)
			    FDR[i].iWarningDelay = FDR[i].aiCheckInterval-1;
			  else
				FDR[i].iWarningDelay = OVER_WARNED ;
		    }
	    	if(FDR[i].iWarningDelay  == UNDER_WARNED)
	    	  FDR[i].iWarningDelay = ARMED;
		  }
	    }
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
