/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

// created 20070428 sgi, basic NMEA sentance supported
// WARNING PosiGraph dont send any GGA sentance
// GGA sentance trigger nav process in XCSoar
// Posigraph driver trigger Nav process with RMC sentance (if driver is devA())

// ToDo



#include "externs.h"

#include "devPosiGraph.h"

extern bool UpdateBaroSource(NMEA_INFO* GPS_INFO, const short parserid, const PDeviceDescriptor_t d, const double fAlt);

static BOOL GPWIN(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO);

BOOL PGParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO){
  (void)d;
  (void)String;
  (void)GPS_INFO;

  // $GPWIN ... Winpilot proprietary sentance includinh baro altitude
  // $GPWIN ,01900 , 0 , 5159 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 * 6 B , 0 7 * 6 0 E
  if(_tcsncmp(TEXT("$GPWIN"), String, 6)==0)
    {
      return GPWIN(d, &String[7], GPS_INFO);
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
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = PGLinkTimeout;
  d->Declare = NULL;
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

static BOOL GPWIN(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO)
{
  TCHAR ctemp[80];
  (void)GPS_INFO;
  (void)d;

  NMEAParser::ExtractParameter(String, ctemp, 2);

  UpdateBaroSource( GPS_INFO, 0, d,   AltitudeToQNHAltitude(  iround(StrToDouble(ctemp, NULL) / 10)));

  return FALSE;

}
