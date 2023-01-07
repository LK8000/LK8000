/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "devZander.h"

static BOOL PZAN1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *apGPS);
static BOOL PZAN2(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *apGPS);
static BOOL PZAN3(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *apGPS);
static BOOL PZAN4(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *apGPS);

static BOOL ZanderParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *apGPS){
  (void)d;

  if (!NMEAParser::NMEAChecksum(String) || (apGPS == NULL)){
    return FALSE;
  }


  if(_tcsncmp(TEXT("$PZAN1"), String, 6)==0)
    {
      return PZAN1(d, &String[7], apGPS);
    }
  if(_tcsncmp(TEXT("$PZAN2"), String, 6)==0)
    {
      return PZAN2(d, &String[7], apGPS);
    }
  if(_tcsncmp(TEXT("$PZAN3"), String, 6)==0)
    {
      return PZAN3(d, &String[7], apGPS);
    }
  if(_tcsncmp(TEXT("$PZAN4"), String, 6)==0)
    {
      return PZAN4(d, &String[7], apGPS);
    }

  return FALSE;

}

void zanderInstall(PDeviceDescriptor_t d) {
  _tcscpy(d->Name, TEXT("Zander"));
  d->ParseNMEA = ZanderParseNMEA;
}

// *****************************************************************************
// local stuff


static BOOL PZAN1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *apGPS)
{
  double palt=0;
  TCHAR ctemp[80];
  NMEAParser::ExtractParameter(String,ctemp,0);
  palt=StrToDouble(ctemp,NULL);
  UpdateBaroSource(apGPS, d, QNEAltitudeToQNHAltitude(palt));
  return TRUE;
}


static BOOL PZAN2(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *apGPS)
{
  TCHAR ctemp[80];

  NMEAParser::ExtractParameter(String,ctemp,0);
  double vtas = StrToDouble(ctemp,NULL)/3.6;

  NMEAParser::ExtractParameter(String,ctemp,1);
  double wnet = (StrToDouble(ctemp,NULL)-10000)/100; // cm/s
  UpdateVarioSource(*apGPS, *d, wnet);

  double qnh_altitude = (BaroAltitudeAvailable(*apGPS)) ? apGPS->BaroAltitude : apGPS->Altitude;
  double vias = IndicatedAirSpeed(vtas, QNHAltitudeToQNEAltitude(qnh_altitude));

  apGPS->AirspeedAvailable = TRUE;
  apGPS->TrueAirspeed = vtas;
  apGPS->IndicatedAirspeed = vias;

  return TRUE;
}

static BOOL PZAN3(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *apGPS)
{
  //$PZAN3,+,026,A,321,035,V*cc
  //Windkomponente (+=R�ckenwind, -=Gegenwind)
  //A=active (Messung Windkomponente ok) / V=void (Messung nicht verwendbar)
  //Windrichtung (true, Wind aus dieser Richtung))
  //Windst�rke (km/h)
  //A=active (Windmessung ok) / V=void (Windmessung nicht verwendbar)
  //Windmessung im Geradeausflug: mit ZS1-Kompass A,A, ohne Kompass A,V
  //Windmessung im Kreisflug: V,A

  TCHAR ctemp[80];
  double wspeed, wfrom;
  char wind_usable;

  NMEAParser::ExtractParameter(String,ctemp,3);
  wfrom=StrToDouble(ctemp,NULL);

  NMEAParser::ExtractParameter(String,ctemp,4);
  wspeed=StrToDouble(ctemp,NULL);

  NMEAParser::ExtractParameter(String,ctemp,5);
  wind_usable=ctemp[0];


  if (wind_usable == 'A') {

	wspeed/=3.6;

	#if 1 // 120424 fix correct wind setting

	apGPS->ExternalWindAvailable = TRUE;
	apGPS->ExternalWindSpeed = wspeed;
	apGPS->ExternalWindDirection = wfrom;

	#else

	// do not update if it has not changed
	if ( (wspeed!=CALCULATED_INFO.WindSpeed) || (wfrom != CALCULATED_INFO.WindBearing) ) {

		SetWindEstimate(wspeed, wfrom,9);
		CALCULATED_INFO.WindSpeed=wspeed;
		CALCULATED_INFO.WindBearing=wfrom;

	}
	#endif
  }

  return true;
}

static BOOL PZAN4(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *apGPS)
{
  //$PZAN4,1.5,+,20,39,45*cc
  //Einstellungen am ZS1:
  //MacCready (m/s)
  //windcomponent (km/h)
  //wing loading (kp/m2)
  //best glide ratio

  TCHAR ctemp[80];

  NMEAParser::ExtractParameter(String,ctemp,0);
  CheckSetMACCREADY(StrToDouble(ctemp, nullptr));


  return true;
}
