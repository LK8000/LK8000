/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "FlightDataRec.h"
#include "DoInits.h"

#define NO_ENTRYS 20

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
	FDR[i].abLog = 0 ;FDR[i].fMin =0.0 ; FDR[i].fMax = 0.0;FDR[i].aiWarningCnt = 0; FDR[i].aiWarningCnt= 0;
  }


  i=0;  iLogDelay=0;
  do
  {
	i++;
	fscanf(DataRecConfigFile, "%120[^\n]",szTmp);
	iLogDelay = atoi(szTmp);

  } while ((iLogDelay == 0) && (i< 50));

  for(i= 0 ; i < 13; i++) {
	fscanf(DataRecConfigFile, "%d %f %f %i %i %120[^\n]", 
		&FDR[i].abLog ,&FDR[i].fMin , &FDR[i].fMax , &FDR[i].aiWarningCnt, &FDR[i].aiWarningCnt, szTmp );
  }

  fclose(DataRecConfigFile);

  if (iLogDelay == 0 ) return; // There is no logging needed

  // CAREFUL no check on size, buffer overrun szName is sized 30
  _tcscpy( FDR[0].szName , _T("External Batt 1"));
  _tcscpy( FDR[1].szName , _T("External Batt 2"));
  _tcscpy( FDR[2].szName , _T("supply  voltage"));
  _tcscpy( FDR[3].szName , _T("Batt %"));
  _tcscpy( FDR[4].szName , _T("Outside Air Temperature"));
  _tcscpy( FDR[5].szName , _T("Longitude"));
  _tcscpy( FDR[6].szName , _T("Latitude"));
  _tcscpy( FDR[7].szName , _T("Altitude"));
  _tcscpy( FDR[8].szName , _T("Baro Altitude"));
  _tcscpy( FDR[9].szName , _T("Speed"));
  _tcscpy( FDR[10].szName , _T("Indcated Airspeed"));
  _tcscpy( FDR[11].szName , _T("TrackBearing"));
  _tcscpy( FDR[12].szName , _T("Vario"));
  _tcscpy( FDR[13].szName , _T("NettoVario"));

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

  fprintf(FlightDataRec,"Recording interval:%ds \r\n\r\n",iLogDelay);

  for( i = 0;  i < NO_ENTRYS; i++)
  {
	unicode2utf(FDR[i].szName, sbuf, sizeof(sbuf));
	if(FDR[i].abLog > 0)
		fprintf(FlightDataRec,"%30s recording enabled\r\n",sbuf);
  }
  fprintf(FlightDataRec,"\r");

  for( i = 0;  i < NO_ENTRYS; i++)
  {
	unicode2utf(FDR[i].szName, sbuf, sizeof(sbuf));
	if(FDR[i].aiWarningCnt > 0)
	{
		if( FDR[i].aiWarningCnt > 0)
		{
			fprintf(FlightDataRec,"%30s range (%4.2f .. %4.2f) check every %is: max. %i warnings\r", 
				sbuf,FDR[i].fMin,  FDR[i].fMax,FDR[i].aiWarningCnt,  FDR[i].aiWarningCnt );
		}
		else
		{
			fprintf(FlightDataRec,"%30s range (%4.2f .. %4.2f) check every %is: unlimited warnings!\r", 
				sbuf,FDR[i].fMin,  FDR[i].fMax,FDR[i].aiWarningCnt);
		}
	}
  }

  fprintf(FlightDataRec,"\r");
  fprintf(FlightDataRec,"hh:mm:ss ");
  if(FDR[0].abLog  > 0) fprintf(FlightDataRec," BAT1 " );
  if(FDR[1].abLog  > 0) fprintf(FlightDataRec,"  BAT2 " );
  if(FDR[2].abLog  > 0) fprintf(FlightDataRec," IntV " );
  if(FDR[3].abLog  > 0) fprintf(FlightDataRec,"  %%  " );
  if(FDR[4].abLog  > 0) fprintf(FlightDataRec," OAT  " );
  if(FDR[5].abLog  > 0) fprintf(FlightDataRec," lat       " );
  if(FDR[6].abLog  > 0) fprintf(FlightDataRec," lon       " );
  if(FDR[7].abLog  > 0) fprintf(FlightDataRec," Alt "  );
  if(FDR[8].abLog  > 0) fprintf(FlightDataRec," AltB "  );
  if(FDR[9].abLog  > 0) fprintf(FlightDataRec,"  AS "  );
  if(FDR[10].abLog > 0) fprintf(FlightDataRec," IAS "  );
  if(FDR[11].abLog > 0) fprintf(FlightDataRec," BRG " );
  if(FDR[12].abLog > 0) fprintf(FlightDataRec,"  VAR "  );
  if(FDR[13].abLog > 0) fprintf(FlightDataRec,"  NET "  );

  fprintf(FlightDataRec,"\r"  );

}




/********************************************************
 *
 *  UpdateFlightRecorder
 *
 *******************************************************/
void UpdateFlightRecorder(void) {
  static unsigned int iCallCnt = 0;

  LKASSERT(iLogDelay<32767);

  if ((iLogDelay==0) || (++iCallCnt<iLogDelay)) return;
  iCallCnt=0;

  float fValue[NO_ENTRYS];

  LockFlightData();
  	fValue[0]  = GPS_INFO.ExtBatt1_Voltage;
  	fValue[1]  = GPS_INFO.ExtBatt2_Voltage;
  	fValue[2]  = GPS_INFO.SupplyBatteryVoltage;
  	fValue[3]  = PDABatteryPercent;
  	fValue[4]  = GPS_INFO.OutsideAirTemperature;
  	fValue[5]  = GPS_INFO.Latitude;
  	fValue[6]  = GPS_INFO.Longitude;
  	fValue[7]  = GPS_INFO.Altitude;
  	fValue[8]  = GPS_INFO.BaroAltitude;
  	fValue[9]  = GPS_INFO.Speed;
  	fValue[10] = GPS_INFO.IndicatedAirspeed;
  	fValue[11] = GPS_INFO.TrackBearing;
  	fValue[12] = GPS_INFO.Vario;
  	fValue[13] = GPS_INFO.NettoVario;
  	int Hour   = GPS_INFO.Hour;
  	int Min    = GPS_INFO.Minute;
  	int Sec    = GPS_INFO.Second;
  UnlockFlightData();

  if (FlightDataRec==NULL) return;

    fprintf(FlightDataRec,"%02d:%02d:%02d ", Hour,  Min,  Sec  );
    if(FDR[0].abLog  > 0) fprintf(FlightDataRec," %4.2f ",  fValue[0]  );// GPS_INFO.ExtBatt1_Voltage;
    if(FDR[1].abLog  > 0) fprintf(FlightDataRec," %4.2f ",  fValue[1]  );// GPS_INFO.ExtBatt2_Voltage;
    if(FDR[2].abLog  > 0) fprintf(FlightDataRec," %4.2f ",  fValue[2]  );// GPS_INFO.SupplyBatteryVoltage;
    if(FDR[3].abLog  > 0) fprintf(FlightDataRec," %03d " , (int) fValue[3]  );// GPS_INFO.PDABatteryPercent;
    if(FDR[4].abLog  > 0) fprintf(FlightDataRec," %4.2f ",  fValue[4]  );// GPS_INFO.OutsideAirTemperature;
    if(FDR[5].abLog  > 0) fprintf(FlightDataRec," %f ",     fValue[5]  );// GPS_INFO.Latitude;
    if(FDR[6].abLog  > 0) fprintf(FlightDataRec," %f ",     fValue[6]  );// GPS_INFO.Longitude;
    if(FDR[7].abLog  > 0) fprintf(FlightDataRec," %4.0f ",  fValue[7]  );// GPS_INFO.Altitude;
    if(FDR[8].abLog  > 0) fprintf(FlightDataRec," %4.0f ",  fValue[8]  );// GPS_INFO.BaroAltitude;
    if(FDR[9].abLog  > 0) fprintf(FlightDataRec," %3.0f ",  fValue[9]  );// GPS_INFO.Speed;
    if(FDR[10].abLog > 0) fprintf(FlightDataRec," %3.0f ",  fValue[10] );// GPS_INFO.IndicatedAirspeed;
    if(FDR[11].abLog > 0) fprintf(FlightDataRec," %3.0f ",  fValue[11] );// GPS_INFO.TrackBearing;
    if(FDR[12].abLog > 0) fprintf(FlightDataRec," %4.2f ",  fValue[12] );// GPS_INFO.Vario;
    if(FDR[13].abLog > 0) fprintf(FlightDataRec," %4.2f ",  fValue[13] );// GPS_INFO.NettoVario;

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
	fValue[0]  = GPS_INFO.ExtBatt1_Voltage;
	fValue[1]  = GPS_INFO.ExtBatt2_Voltage;
	fValue[2]  = GPS_INFO.SupplyBatteryVoltage;
	fValue[3]  = PDABatteryPercent;
	fValue[4]  = GPS_INFO.OutsideAirTemperature;
	fValue[5]  = GPS_INFO.Latitude;
	fValue[6]  = GPS_INFO.Longitude;
	fValue[7]  = GPS_INFO.Altitude;
	fValue[8]  = GPS_INFO.BaroAltitude;
	fValue[9]  = GPS_INFO.Speed;
	fValue[10] = GPS_INFO.IndicatedAirspeed;
	fValue[11] = GPS_INFO.TrackBearing;
	fValue[12] = GPS_INFO.Vario;
	fValue[13] = GPS_INFO.NettoVario;
  UnlockFlightData();


  if(iCallCnt > 60) // first warning after 60s
  {
	for(i=0 ; i < NO_ENTRYS; i++)
	{
		if(FDR[i].aiWarningCnt > 0)  // check enabled ? 
		   if((iCallCnt%FDR[i].aiWarningCnt)==0)	// check every ? sec 
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
  } else
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
