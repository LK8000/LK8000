/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "devCondor.h"
#include "Comm/ExternalWind.h"


static BOOL cLXWP0(DeviceDescriptor_t* d, const char* String, NMEA_INFO *pGPS);
static BOOL cLXWP1(DeviceDescriptor_t* d, const char* String, NMEA_INFO *pGPS);
static BOOL cLXWP2(DeviceDescriptor_t* d, const char* String, NMEA_INFO *pGPS);

static
BOOL CondorParseNMEA(DeviceDescriptor_t* d, const char *String, NMEA_INFO *pGPS){

  if (!NMEAParser::NMEAChecksum(String) || (pGPS == NULL)){
    return FALSE;
  }


  if(strncmp("$LXWP0", String, 6)==0)
    {
      return cLXWP0(d, &String[7], pGPS);
    }
  if(strncmp("$LXWP1", String, 6)==0)
    {
      return cLXWP1(d, &String[7], pGPS);
    }
  if(strncmp("$LXWP2", String, 6)==0)
    {
      return cLXWP2(d, &String[7], pGPS);
    }

  return FALSE;

}


void condorInstall(DeviceDescriptor_t* d) {
  d->ParseNMEA = CondorParseNMEA;
  DevIsCondor = true;
}

// *****************************************************************************
// local stuff


static
BOOL cLXWP1(DeviceDescriptor_t* d, const char* String, NMEA_INFO *pGPS)
{
  //  TCHAR ctemp[80];
  (void)pGPS;
  // do nothing!
  return TRUE;
}


static
BOOL cLXWP2(DeviceDescriptor_t* d, const char* String, NMEA_INFO *pGPS) {
  char ctemp[80];
  NMEAParser::ExtractParameter(String,ctemp,0);
  d->RecvMacCready(StrToDouble(ctemp, nullptr));
  return TRUE;
}


static
BOOL cLXWP0(DeviceDescriptor_t* d, const char* String, NMEA_INFO *pGPS) {
  char ctemp[80];

  /*
  $LXWP0,Y,222.3,1665.5,1.71,,,,,,239,174,10.1

   0 loger_stored (Y/N)
   1 IAS (kph) ----> Condor uses TAS!
   2 baroaltitude (m)
   3 vario (m/s)
   4-8 unknown
   9 heading of plane
  10 windcourse (deg)
  11 windspeed (kph)

  */

  DevIsCondor=true;

  NMEAParser::ExtractParameter(String,ctemp,1);
  double airspeed = Units::From(unKiloMeterPerHour, StrToDouble(ctemp,NULL));

  NMEAParser::ExtractParameter(String,ctemp,2);
  double QneAltitude = StrToDouble(ctemp,NULL);

  pGPS->IndicatedAirspeed = IndicatedAirSpeed(airspeed, QneAltitude);
  pGPS->TrueAirspeed = airspeed;
  pGPS->AirspeedAvailable = TRUE;

  UpdateBaroSource( pGPS, d,  QNEAltitudeToQNHAltitude(QneAltitude));

  NMEAParser::ExtractParameter(String,ctemp,3);
  double Vario = StrToDouble(ctemp,NULL);
  UpdateVarioSource(*pGPS, *d, Vario);


  // we don't use heading for wind calculation since... wind is already calculated in condor!!
  NMEAParser::ExtractParameter(String,ctemp,11);
  double wspeed = StrToDouble(ctemp,NULL);
  NMEAParser::ExtractParameter(String,ctemp,10);
  double wfrom = StrToDouble(ctemp,NULL) + 180;

  UpdateExternalWind(*pGPS, *d, Units::From(Units_t::unKiloMeterPerHour, wspeed), wfrom);

  return TRUE;
}
