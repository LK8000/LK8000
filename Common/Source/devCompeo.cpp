/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"

#include "externs.h"
#include "Utils.h"
#include "Parser.h"
#include "Port.h"

#include "devCompeo.h"

#include "utils/heapcheck.h"

static BOOL VMVABD(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO);

static BOOL CompeoParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO){

  (void)d;

  if(_tcsncmp(TEXT("$VMVABD"), String, 7)==0)
    {
      return VMVABD(d, &String[8], GPS_INFO);
    } 

  return FALSE;

}


static BOOL CompeoIsGPSSource(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE); 
}


static BOOL CompeoIsBaroSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}


static BOOL CompeoLinkTimeout(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


static BOOL CompeoInstall(PDeviceDescriptor_t d){

  // StartupStore(_T(". FLYTEC/COMPEO device installed%s"),NEWLINE);

  _tcscpy(d->Name, TEXT("Brauniger/Compeo 5030"));
  d->ParseNMEA = CompeoParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = CompeoLinkTimeout;
  d->Declare = NULL;
  d->IsGPSSource = CompeoIsGPSSource;
  d->IsBaroSource = CompeoIsBaroSource;

  return(TRUE);

}


BOOL CompeoRegister(void){
  // StartupStore(_T(". Flytec/Compeo device registered%s"),NEWLINE);
  return(devRegister(
    TEXT("Brauniger/Compeo 5030"),
    (1l << dfGPS)
    | (1l << dfBaroAlt)
    | (1l << dfSpeed)
    | (1l << dfVario),
    CompeoInstall
  ));
}


static BOOL VMVABD(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO)
{
/*
	$VMVABD,  
	0000.0 gps altitude,		0
	M,				1
	0000.0 bari altitude,		2
	M,				3
	-0.0 vario ms,,,		4
	MS,				7
	0.0 ias or tas,			8
	KH,				9
	22.4 temp,			10
	C*nn

	091129
	The Brauniger Compeo can send TAS if a pitot is connected (delta hang gliders) or IAS is rotary device for para.
	Since rotary is seldom used, we assume TAS is received. No indication from NMEA about IAS or TAS selected.
	100114 IAS or TAS???
*/

  TCHAR ctemp[80];
  double vtas, vias;

  NMEAParser::ExtractParameter(String,ctemp,0);
  GPS_INFO->Altitude = StrToDouble(ctemp,NULL);

  NMEAParser::ExtractParameter(String,ctemp,2);
  #if DUALBARO
  if (d == pDevPrimaryBaroSource) {
	  GPS_INFO->BaroAltitude = AltitudeToQNHAltitude(StrToDouble(ctemp,NULL));
	  GPS_INFO->BaroAltitudeAvailable = TRUE;
  }
  #else
  GPS_INFO->BaroAltitude = AltitudeToQNHAltitude(StrToDouble(ctemp,NULL));
  GPS_INFO->BaroAltitudeAvailable = TRUE;
  #endif

  NMEAParser::ExtractParameter(String,ctemp,4);
  GPS_INFO->Vario = StrToDouble(ctemp,NULL);
  GPS_INFO->VarioAvailable = TRUE;

  NMEAParser::ExtractParameter(String,ctemp,8);
  if (ctemp[0] != '\0') { // 100209
	// we store m/s  , so we convert it from kmh
	vias = StrToDouble(ctemp,NULL)/3.6;
	GPS_INFO->IndicatedAirspeed = vias;
	// Check if zero?
	vtas = vias*AirDensityRatio(GPS_INFO->BaroAltitude);
	GPS_INFO->TrueAirspeed = vtas;

	if (GPS_INFO->IndicatedAirspeed >0) 
		GPS_INFO->AirspeedAvailable = TRUE;
	else 
		GPS_INFO->AirspeedAvailable = FALSE;
  } else
		GPS_INFO->AirspeedAvailable = FALSE;


  TriggerVarioUpdate();

  return TRUE;
}
