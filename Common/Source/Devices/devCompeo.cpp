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
    const double ias = Units::From(unKiloMeterPerHour, StrToDouble(ctemp, nullptr));
    if (ias > 0) {
      pGPS->IndicatedAirSpeed.update(*d, ias);
      pGPS->TrueAirSpeed.update(*d, TrueAirSpeed(ias, QneAltitude));
    }
  }
  TriggerVarioUpdate();

  return TRUE;
}
