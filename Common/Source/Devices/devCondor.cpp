/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "devCondor.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "Comm/ExternalWind.h"

namespace {

BOOL cLXWP2(DeviceDescriptor_t* d, const char* String, NMEA_INFO* pGPS) {
  char ctemp[MAX_NMEA_LEN];
  NMEAParser::ExtractParameter(String, ctemp, 0);
  d->RecvMacCready(StrToDouble(ctemp, nullptr));
  return TRUE;
}

template <unsigned Version>
BOOL cLXWP0(DeviceDescriptor_t* d, const char* String, NMEA_INFO* pGPS) {
  char ctemp[MAX_NMEA_LEN];

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

  DevIsCondor = true;

  NMEAParser::ExtractParameter(String, ctemp, 1);
  double airspeed = Units::From(unKiloMeterPerHour, StrToDouble(ctemp, nullptr));

  NMEAParser::ExtractParameter(String, ctemp, 2);
  double QneAltitude = StrToDouble(ctemp, nullptr);

  pGPS->IndicatedAirspeed = IndicatedAirSpeed(airspeed, QneAltitude);
  pGPS->TrueAirspeed = airspeed;
  pGPS->AirspeedAvailable = TRUE;

  UpdateBaroSource(pGPS, d, QNEAltitudeToQNHAltitude(QneAltitude));

  NMEAParser::ExtractParameter(String, ctemp, 3);
  double Vario = Units::From(unMeterPerSecond, StrToDouble(ctemp, nullptr));
  UpdateVarioSource(*pGPS, *d, Vario);

  // we don't use heading for wind calculation since... wind is already calculated in condor!!
  NMEAParser::ExtractParameter(String, ctemp, 11);
  double wspeed = Units::From(unKiloMeterPerHour, StrToDouble(ctemp, nullptr));
  NMEAParser::ExtractParameter(String, ctemp, 10);
  double wfrom = StrToDouble(ctemp, nullptr);
  if constexpr (Version < 3) {
    wfrom += 180.;
  }

  UpdateExternalWind(*pGPS, *d, wspeed, wfrom);

  return TRUE;
}

template <unsigned Version>
BOOL CondorParseNMEA(DeviceDescriptor_t* d, const char* String, NMEA_INFO* pGPS) {
  if (pGPS && NMEAParser::NMEAChecksum(String)) {

    if (strncmp("$LXWP0", String, 6) == 0) {
      return cLXWP0<Version>(d, &String[7], pGPS);
    }

    if (strncmp("$LXWP2", String, 6) == 0) {
      return cLXWP2(d, &String[7], pGPS);
    }
  }

  return FALSE;
}

}  // namespace

void CondorInstall(DeviceDescriptor_t* d) {
  d->ParseNMEA = CondorParseNMEA<0>;
  DevIsCondor = true;
}

void Condor3Install(DeviceDescriptor_t* d) {
  d->ParseNMEA = CondorParseNMEA<3>;
  DevIsCondor = true;
}
