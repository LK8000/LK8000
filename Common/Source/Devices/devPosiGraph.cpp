/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

// created 20070428 sgi, basic NMEA sentance supported
// WARNING PosiGraph dont send any GGA sentance
// GGA sentance trigger nav process in XCSoar
// Posigraph driver trigger Nav process with RMC sentance (if driver is devA())

// ToDo



#include "externs.h"
#include "Baro.h"
#include "devPosiGraph.h"

static
BOOL GPWIN(DeviceDescriptor_t* d, const char *String, NMEA_INFO *pGPS);

BOOL PGParseNMEA(DeviceDescriptor_t* d, const char *String, NMEA_INFO *pGPS){
  if (!NMEAParser::NMEAChecksum(String) || (pGPS == NULL)){
    return FALSE;
  }


  // $GPWIN ... Winpilot proprietary sentance includinh baro altitude
  // $GPWIN ,01900 , 0 , 5159 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 * 6 B , 0 7 * 6 0 E
  if(strncmp("$GPWIN", String, 6)==0)
    {
      return GPWIN(d, &String[7], pGPS);
    }

  return FALSE;

}

BOOL PGIsGPSSource(DeviceDescriptor_t* d){
  (void)d;
  return(TRUE);
}

void pgInstall(DeviceDescriptor_t* d){

  _tcscpy(d->Name, TEXT("PosiGraph Logger"));
  d->ParseNMEA = PGParseNMEA;
}


// *****************************************************************************
// local stuff

static BOOL GPWIN(DeviceDescriptor_t* d, const char *String, NMEA_INFO *pGPS)
{
  char ctemp[80];

  NMEAParser::ExtractParameter(String, ctemp, 2);

  UpdateBaroSource(pGPS, d, QNEAltitudeToQNHAltitude(iround(StrToDouble(ctemp, NULL) / 10)));

  return FALSE;

}
