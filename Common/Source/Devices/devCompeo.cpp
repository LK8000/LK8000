/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Baro.h"
#include "devCompeo.h"

static BOOL VMVABD(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS);

static BOOL CompeoParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){

  (void)d;

  if (!NMEAParser::NMEAChecksum(String) || (pGPS == NULL)){
    return FALSE;
  }


  if(_tcsncmp(TEXT("$VMVABD"), String, 7)==0)
    {
      return VMVABD(d, &String[8], pGPS);
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

  StartupStore(_T(". FLYTEC/COMPEO device installed%s"),NEWLINE);

  _tcscpy(d->Name, TEXT("Brauniger/Compeo 5030"));
  d->ParseNMEA = CompeoParseNMEA;
  d->LinkTimeout = CompeoLinkTimeout;
  d->IsGPSSource = CompeoIsGPSSource;
  d->IsBaroSource = CompeoIsBaroSource;

  return(TRUE);

}


BOOL CompeoRegister(void){
  return(devRegister(
    TEXT("Brauniger/Compeo 5030"),
    (1l << dfGPS)
    | (1l << dfBaroAlt)
    | (1l << dfSpeed)
    | (1l << dfVario),
    CompeoInstall
  ));
}


static BOOL VMVABD(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS)
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
  pGPS->Altitude = StrToDouble(ctemp,NULL);

  NMEAParser::ExtractParameter(String,ctemp,2);
  double palt=StrToDouble(ctemp,NULL);

   UpdateBaroSource( pGPS, 0,d, QNEAltitudeToQNHAltitude(palt));

  NMEAParser::ExtractParameter(String,ctemp,4);
  pGPS->Vario = StrToDouble(ctemp,NULL);
  pGPS->VarioAvailable = TRUE;

  NMEAParser::ExtractParameter(String,ctemp,8);
  if (ctemp[0] != '\0') { // 100209
	// we store m/s  , so we convert it from kmh
	vias = StrToDouble(ctemp,NULL)/3.6;
	pGPS->IndicatedAirspeed = vias;
	// Check if zero?
	vtas = vias*AirDensityRatio(palt);
	pGPS->TrueAirspeed = vtas;

	if (pGPS->IndicatedAirspeed >0)
		pGPS->AirspeedAvailable = TRUE;
	else
		pGPS->AirspeedAvailable = FALSE;
  } else
		pGPS->AirspeedAvailable = FALSE;


  TriggerVarioUpdate();

  return TRUE;
}
