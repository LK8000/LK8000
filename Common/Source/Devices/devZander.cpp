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
#include "Comm/ExternalWind.h"

static BOOL PZAN1(DeviceDescriptor_t* d, const char* String, NMEA_INFO *apGPS);
static BOOL PZAN2(DeviceDescriptor_t* d, const char* String, NMEA_INFO *apGPS);
static BOOL PZAN3(DeviceDescriptor_t* d, const char* String, NMEA_INFO *apGPS);
static BOOL PZAN4(DeviceDescriptor_t* d, const char* String, NMEA_INFO *apGPS);

static BOOL ZanderParseNMEA(DeviceDescriptor_t* d, const char* String, NMEA_INFO *apGPS){
  (void)d;

  if (!NMEAParser::NMEAChecksum(String) || (apGPS == NULL)){
    return FALSE;
  }


  if(strncmp("$PZAN1", String, 6) == 0)
    {
      return PZAN1(d, &String[7], apGPS);
    }
  if(strncmp("$PZAN2", String, 6) == 0)
    {
      return PZAN2(d, &String[7], apGPS);
    }
  if(strncmp("$PZAN3", String, 6) == 0)
    {
      return PZAN3(d, &String[7], apGPS);
    }
  if(strncmp("$PZAN4", String, 6) == 0)
    {
      return PZAN4(d, &String[7], apGPS);
    }

  return FALSE;

}

void zanderInstall(DeviceDescriptor_t* d) {
  d->ParseNMEA = ZanderParseNMEA;
}

// *****************************************************************************
// local stuff


static BOOL PZAN1(DeviceDescriptor_t* d, const char* String, NMEA_INFO *apGPS)
{
  double palt=0;
  char ctemp[80];
  NMEAParser::ExtractParameter(String,ctemp,0);
  palt=StrToDouble(ctemp,NULL);
  UpdateBaroSource(apGPS, d, QNEAltitudeToQNHAltitude(palt));
  return TRUE;
}


static BOOL PZAN2(DeviceDescriptor_t* d, const char* String, NMEA_INFO *apGPS)
{
  char ctemp[80];

  NMEAParser::ExtractParameter(String,ctemp,0);
  double vtas = StrToDouble(ctemp,NULL)/3.6;

  NMEAParser::ExtractParameter(String,ctemp,1);
  double wnet = (StrToDouble(ctemp,NULL)-10000)/100; // cm/s
  UpdateVarioSource(*apGPS, *d, wnet);

  double qnh_altitude = (BaroAltitudeAvailable(*apGPS)) ? apGPS->BaroAltitude : apGPS->Altitude;
  double vias = IndicatedAirSpeed(vtas, QNHAltitudeToQNEAltitude(qnh_altitude));

  apGPS->TrueAirSpeed.update(*d, vtas);
  apGPS->IndicatedAirSpeed.update(*d, vias);

  return TRUE;
}

static BOOL PZAN3(DeviceDescriptor_t* d, const char* String, NMEA_INFO *apGPS)
{
  //$PZAN3,+,026,A,321,035,V*cc
  //Windkomponente (+=R�ckenwind, -=Gegenwind)
  //A=active (Messung Windkomponente ok) / V=void (Messung nicht verwendbar)
  //Windrichtung (true, Wind aus dieser Richtung))
  //Windst�rke (km/h)
  //A=active (Windmessung ok) / V=void (Windmessung nicht verwendbar)
  //Windmessung im Geradeausflug: mit ZS1-Kompass A,A, ohne Kompass A,V
  //Windmessung im Kreisflug: V,A

  char ctemp[80];
  double wspeed, wfrom;
  char wind_usable;

  NMEAParser::ExtractParameter(String,ctemp,3);
  wfrom=StrToDouble(ctemp,NULL);

  NMEAParser::ExtractParameter(String,ctemp,4);
  wspeed=StrToDouble(ctemp,NULL);

  NMEAParser::ExtractParameter(String,ctemp,5);
  wind_usable=ctemp[0];


  if (wind_usable == 'A') {
    UpdateExternalWind(*apGPS, *d, Units::From(Units_t::unKiloMeterPerHour, wspeed), wfrom);
  }

  return true;
}

static BOOL PZAN4(DeviceDescriptor_t* d, const char* String, NMEA_INFO *apGPS)
{
  //$PZAN4,1.5,+,20,39,45*cc
  //Einstellungen am ZS1:
  //MacCready (m/s)
  //windcomponent (km/h)
  //wing loading (kp/m2)
  //best glide ratio

  char ctemp[80];

  NMEAParser::ExtractParameter(String,ctemp,0);
  d->RecvMacCready(StrToDouble(ctemp, nullptr));

  return true;
}
