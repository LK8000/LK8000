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

static BOOL GPWIN(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS);

BOOL PGParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){
  (void)d;
  (void)String;
  (void)pGPS;

  if (!NMEAParser::NMEAChecksum(String) || (pGPS == NULL)){
    return FALSE;
  }


  // $GPWIN ... Winpilot proprietary sentance includinh baro altitude
  // $GPWIN ,01900 , 0 , 5159 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 * 6 B , 0 7 * 6 0 E
  if(_tcsncmp(TEXT("$GPWIN"), String, 6)==0)
    {
      return GPWIN(d, &String[7], pGPS);
    }

  return FALSE;

}


BOOL PGIsLogger(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


BOOL PGIsGPSSource(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}

BOOL PGIsBaroSource(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}

BOOL PGLinkTimeout(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


BOOL pgInstall(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("PosiGraph Logger"));
  d->ParseNMEA = PGParseNMEA;
  d->LinkTimeout = PGLinkTimeout;
  d->IsLogger = PGIsLogger;
  d->IsGPSSource = PGIsGPSSource;
  d->IsBaroSource = PGIsBaroSource;

  return(TRUE);

}


BOOL pgRegister(void){
  return(devRegister(
    TEXT("PosiGraph Logger"),
      1l << dfGPS
      | (1l << dfBaroAlt)
      | 1l << dfLogger,
    pgInstall
  ));
}


// *****************************************************************************
// local stuff

static BOOL GPWIN(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS)
{
  TCHAR ctemp[80];
  (void)pGPS;
  (void)d;

  NMEAParser::ExtractParameter(String, ctemp, 2);

  UpdateBaroSource( pGPS, 0, d,   QNEAltitudeToQNHAltitude(  iround(StrToDouble(ctemp, NULL) / 10)));

  return FALSE;

}
