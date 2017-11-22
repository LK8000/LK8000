/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "devCompeo.h"

static BOOL VMVABD(DeviceDescriptor_t* d, const char *String, NMEA_INFO *pGPS);

static BOOL CompeoParseNMEA(DeviceDescriptor_t* d, const char *String, NMEA_INFO *pGPS){

  (void)d;

  if (!NMEAParser::NMEAChecksum(String) || (pGPS == NULL)){
    return FALSE;
  }

  if (strncmp("$VMVABD", String, 7)==0) {
    return VMVABD(d, &String[8], pGPS);
  }

  return FALSE;
}

void CompeoInstall(DeviceDescriptor_t* d) {

  StartupStore(_T(". FLYTEC/COMPEO device installed%s"),NEWLINE);

  _tcscpy(d->Name, TEXT("Brauniger/Compeo 5030"));
  d->ParseNMEA = CompeoParseNMEA;
}

static BOOL VMVABD(DeviceDescriptor_t* d, const char *String, NMEA_INFO *pGPS)
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

  char ctemp[80];

  NMEAParser::ExtractParameter(String,ctemp,0);
  pGPS->Altitude = StrToDouble(ctemp,NULL);

  NMEAParser::ExtractParameter(String,ctemp,2);
  double QneAltitude = StrToDouble(ctemp,NULL);

  UpdateBaroSource( pGPS, d, QNEAltitudeToQNHAltitude(QneAltitude));

  NMEAParser::ExtractParameter(String,ctemp,4);
  UpdateVarioSource(*pGPS, *d, StrToDouble(ctemp,NULL));

  NMEAParser::ExtractParameter(String,ctemp,8);
  if (ctemp[0] != '\0') { // 100209
    // we store m/s  , so we convert it from kmh
    pGPS->IndicatedAirspeed = Units::ToSys(unKiloMeterPerHour, StrToDouble(ctemp, nullptr));
    pGPS->TrueAirspeed = TrueAirSpeed(pGPS->IndicatedAirspeed, QneAltitude);
    pGPS->AirspeedAvailable = (pGPS->IndicatedAirspeed >0);
  } else {
    pGPS->AirspeedAvailable = FALSE;
  }
  TriggerVarioUpdate();

  return TRUE;
}
