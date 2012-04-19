/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "FlightDataRec.h"
#include "DoInits.h"

#define NO_ENTRYS 22

FILE *FlightDataRec=NULL;

static unsigned int iLogDelay =0;	// if 0 we skip any logging activity
bool FlightDataRecorderConfiguration=false; // if false, we skip any FlightRecorder activity, including warnings and logging

sFlightDataRec FDR[NO_ENTRYS];

//if(DoInit[MDI_FLIGHTRECORDER])


//
// InitFDR
// THIS FUNCTION MUST BE CALLED ONLY BY ONE UNIQUE THREAD
//
void InitFDR(void)
{

  TCHAR path[MAX_PATH+1];
  TCHAR szBatLogFileName[MAX_PATH+1];
  char szTmp[255];
  FILE* DataRecConfigFile = NULL;

  int i;

  LocalPath(path,TEXT(LKD_CONF));
  wsprintf(szBatLogFileName, TEXT("%s\\FlightRec.CFG"), path);
  DataRecConfigFile = _tfopen(szBatLogFileName, TEXT("r"));

  if(DataRecConfigFile == NULL)
  {
	// We shall no more check for dataconfig, unless we reset DoInit
	// So now the FlightDataRecorder is disabled at all, and we shall only
	// check for FlightDataRecorderConfiguration to be true 
	return;
  }

  FlightDataRecorderConfiguration=true;

  for(i=0 ; i < NO_ENTRYS; i++)
  {
	FDR[i].abLog = 0 ;FDR[i].fMin =0.0 ; FDR[i].fMax = 0.0;FDR[i].aiCheckInterval = 0; FDR[i].aiWarningCnt= 0;
  }


  i=0;  iLogDelay=0;
  do
  {
	i++;
	fscanf(DataRecConfigFile, "%120[^\n]",szTmp);
	iLogDelay = atoi(szTmp);

  } while ((iLogDelay == 0) && (i< 50));

  for(i= 0 ; i < NO_ENTRYS; i++)
    fscanf(DataRecConfigFile, "%d %f %f %i %i %120[^\n]", &FDR[i].abLog ,&FDR[i].fMin , &FDR[i].fMax , &FDR[i].aiCheckInterval, &FDR[i].aiWarningCnt, szTmp );
  fclose(DataRecConfigFile);
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



  if(  iLogDelay >0)  /* logging enabled? */
  {
  char sbuf[30];

  // WE TRY TO OPEN THE LOGFILE, AND IF WE CANT WE PERMANENTLY DISABLE LOGGING
    LocalPath(path,TEXT(LKD_LOGS));
    wsprintf(szBatLogFileName, TEXT("%s\\Flightrecorder.TXT"), path);
    FlightDataRec = _tfopen(szBatLogFileName, TEXT("w"));

  if (!FlightDataRec) {
	StartupStore(_T("... FDR Failure, cannot open <%s>%s"),szBatLogFileName,NEWLINE);
	iLogDelay=0;	 // logging disabled permanently
  	return;
  } 

  // FROM NOW ON, we can write on the file and on LK exit we must close the file.

  fprintf(FlightDataRec,"Recording interval:%ds \r",iLogDelay);

  for( i = 0;  i < NO_ENTRYS; i++)
  {
	unicode2utf(FDR[i].szName, sbuf, sizeof(sbuf));
	if(FDR[i].abLog > 0)
		fprintf(FlightDataRec,"%30s recording enabled\r",sbuf);
  }
  fprintf(FlightDataRec,"\r");

  for( i = 0;  i < NO_ENTRYS; i++)
  {
	unicode2utf(FDR[i].szName, sbuf, sizeof(sbuf));
	if(FDR[i].aiCheckInterval > 0)
	{
		if( FDR[i].aiWarningCnt > 0)
		{
			fprintf(FlightDataRec,"%30s range (%4.2f .. %4.2f) check every %is: max. %i warnings\r", 
				sbuf,FDR[i].fMin,  FDR[i].fMax,FDR[i].aiCheckInterval,  FDR[i].aiWarningCnt );
		}
		else
		{
			fprintf(FlightDataRec,"%30s range (%4.2f .. %4.2f) check every %is: unlimited warnings!\r", 
				sbuf,FDR[i].fMin,  FDR[i].fMax,FDR[i].aiCheckInterval);
		}
	}
  }

  fprintf(FlightDataRec,"\r");
  fprintf(FlightDataRec,"hh:mm:ss ");
    int idx=0;
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec," BAT1 " );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec,"  BAT2 " );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec," IntV " );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec," BAT%% " );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec," OAT  " );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec," lat       " );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec," lon       " );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec," Alt "  );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec," AltB "  );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec,"  AGL "  );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec,"  AS "  );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec," IAS "  );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec," BRG " );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec," VAR "  );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec,"  NET "  );

    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec,"  TAS "  );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec,"  AcX "  );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec,"  AcY "  );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec,"  AcZ "  );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec,"  BAL " );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec,"  BUG "  );
    if(FDR[idx++].abLog > 0) fprintf(FlightDataRec,"  MC  "  );

  fprintf(FlightDataRec,"\r"  );
  }
}




/********************************************************
 *
 *  UpdateFlightRecorder
 *
 *******************************************************/
void UpdateFlightRecorder(void) {

  static unsigned int iCallCnt = 0;

  int idx=0;
  LKASSERT(iLogDelay<32767);

  if ((iLogDelay==0) || (++iCallCnt<iLogDelay)) return;
  iCallCnt=0;

  float fValue[NO_ENTRYS];
    LockFlightData();

  	fValue[idx++] = GPS_INFO.ExtBatt1_Voltage;
  	fValue[idx++] = GPS_INFO.ExtBatt2_Voltage;
  	fValue[idx++] = GPS_INFO.SupplyBatteryVoltage;
  	fValue[idx++] = PDABatteryPercent;
  	fValue[idx++] = GPS_INFO.OutsideAirTemperature;
  	fValue[idx++] = GPS_INFO.Latitude;
  	fValue[idx++] = GPS_INFO.Longitude;
  	fValue[idx++] = GPS_INFO.Altitude;
  	fValue[idx++] = GPS_INFO.BaroAltitude;
  	fValue[idx++] = CALCULATED_INFO.AltitudeAGL;
  	fValue[idx++] = GPS_INFO.Speed;
  	fValue[idx++] = GPS_INFO.IndicatedAirspeed;
  	fValue[idx++] = GPS_INFO.TrackBearing;
  	fValue[idx++] = GPS_INFO.Vario;
  	fValue[idx++] = GPS_INFO.NettoVario;

  	fValue[idx++] = GPS_INFO.TrueAirspeed ;
  	fValue[idx++] = GPS_INFO.AccelX ;
  	fValue[idx++] = GPS_INFO.AccelY ;
  	fValue[idx++] = GPS_INFO.AccelZ ;
  	fValue[idx++] = GPS_INFO.Ballast ;
  	fValue[idx++] = GPS_INFO.Bugs ;
  	fValue[idx++] = GPS_INFO.MacReady ;


//  	fValue[20] = NMEA_INFO.SpeedToFly();
  	int Hour   = GPS_INFO.Hour;
  	int Min    = GPS_INFO.Minute;
  	int Sec    = GPS_INFO.Second;
  UnlockFlightData();

  if (FlightDataRec==NULL) return;

    fprintf(FlightDataRec,"%02d:%02d:%02d ", Hour,  Min,  Sec  );
    idx=-1;
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %4.2f ",  fValue[idx]  );// GPS_INFO.ExtBatt1_Voltage;
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %4.2f ",  fValue[idx]  );// GPS_INFO.ExtBatt2_Voltage;
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %4.2f ",  fValue[idx]  );// GPS_INFO.SupplyBatteryVoltage;
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %03d " , (int) fValue[idx]  );// GPS_INFO.PDABatteryPercent;
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %4.2f ",  fValue[idx]  );// GPS_INFO.OutsideAirTemperature;
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %f ",     fValue[idx]  );// GPS_INFO.Latitude;
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %f ",     fValue[idx]  );// GPS_INFO.Longitude;
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %4.0f ",  fValue[idx]  );// GPS_INFO.Altitude;
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %4.0f ",  fValue[idx]  );// GPS_INFO.BaroAltitude;
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %4.0f ",  fValue[idx]  );// CALCULATED_INFO.AltitudeAGL;
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %3.0f ",  fValue[idx]  );// GPS_INFO.Speed;
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %3.0f ",  fValue[idx] );// GPS_INFO.IndicatedAirspeed;
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %3.0f ",  fValue[idx] );// GPS_INFO.TrackBearing;
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %4.2f ",  fValue[idx] );// GPS_INFO.Vario;
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %4.2f ",  fValue[idx] );// GPS_INFO.NettoVario;

    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %4.0f ", fValue[idx]  );// GPS_INFO.TrueAirspeed
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %4.2f ", fValue[idx]  );// GPS_INFO.AccelX
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %4.2f ", fValue[idx]  );// GPS_INFO.AccelY
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %4.2f ", fValue[idx]  );// GPS_INFO.AccelZ
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %4.0f ", fValue[idx]  );// GPS_INFO.Ballast
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %4.0f ", fValue[idx]  );// GPS_INFO.Bugs
    if(FDR[++idx].abLog > 0) fprintf(FlightDataRec," %4.2f ", fValue[idx]  );// GPS_INFO.MacReady
    fprintf(FlightDataRec,"\r"); /* next line */

}


/******************************************************************************************/
/* check ranges and generate warnings
 *
 *
 *                                           */
/******************************************************************************************/
void OnCheckFDRRanges(void)
{
  static unsigned int iCallCnt=0;
  int i;
  TCHAR szTmp[80];
  float fValue[NO_ENTRYS];

  LockFlightData();
  i=0;
	fValue[i++] = GPS_INFO.ExtBatt1_Voltage;
	fValue[i++] = GPS_INFO.ExtBatt2_Voltage;
	fValue[i++] = GPS_INFO.SupplyBatteryVoltage;
	fValue[i++] = PDABatteryPercent;
	fValue[i++] = GPS_INFO.OutsideAirTemperature;
	fValue[i++] = GPS_INFO.Latitude;
	fValue[i++] = GPS_INFO.Longitude;
	fValue[i++] = GPS_INFO.Altitude;
	fValue[i++] = GPS_INFO.BaroAltitude;
  	fValue[i++] = CALCULATED_INFO.AltitudeAGL;
	fValue[i++] = GPS_INFO.Speed;
	fValue[i++] = GPS_INFO.IndicatedAirspeed;
	fValue[i++] = GPS_INFO.TrackBearing;
	fValue[i++] = GPS_INFO.Vario;
	fValue[i++] = GPS_INFO.NettoVario;

  	fValue[i++] = GPS_INFO.TrueAirspeed ;
  	fValue[i++] = GPS_INFO.AccelX ;
  	fValue[i++] = GPS_INFO.AccelY ;
  	fValue[i++] = GPS_INFO.AccelZ ;
  	fValue[i++] = GPS_INFO.Ballast ;
  	fValue[i++] = GPS_INFO.Bugs ;
  	fValue[i++] = GPS_INFO.MacReady ;

UnlockFlightData();

  if(iCallCnt > 60) // first warning after 60s
  {
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
  else
iCallCnt++;

}



/********************************************************
 *
 *  OnExitFlightRecorder
 *
 *******************************************************/
void OnExitFlightRecorder(void)
{
  iLogDelay=0;
  if (FlightDataRec) fclose(FlightDataRec);
}
