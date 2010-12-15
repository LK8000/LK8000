/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"

#include "externs.h"
#include "Utils.h"
#include "Parser.h"
#include "Port.h"

#include "devLX.h"

static BOOL LXWP0(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO);
static BOOL LXWP1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO);
static BOOL LXWP2(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO);


static BOOL LXParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO){
  (void)d;

  if(_tcsncmp(TEXT("$LXWP0"), String, 6)==0)
    {
      return LXWP0(d, &String[7], GPS_INFO);
    } 
  if(_tcsncmp(TEXT("$LXWP1"), String, 6)==0)
    {
      return LXWP1(d, &String[7], GPS_INFO);
    } 
  if(_tcsncmp(TEXT("$LXWP2"), String, 6)==0)
    {
      return LXWP2(d, &String[7], GPS_INFO);
    }

  return FALSE;

}


/*
static BOOL LXIsLogger(PDeviceDescriptor_t d){
  (void)d;
  return(FALSE);
}
*/

static BOOL LXIsGPSSource(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE); // ? is this true
}


static BOOL LXIsBaroSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}


static BOOL LXLinkTimeout(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


static BOOL lxInstall(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("LX"));
  d->ParseNMEA = LXParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = LXLinkTimeout;
  d->Declare = NULL;
  d->IsGPSSource = LXIsGPSSource;
  d->IsBaroSource = LXIsBaroSource;

  return(TRUE);

}


BOOL lxRegister(void){
  return(devRegister(
    TEXT("LX"),
    (1l << dfGPS)
    | (1l << dfBaroAlt)
    | (1l << dfSpeed)
    | (1l << dfVario),
    lxInstall
  ));
}


// *****************************************************************************
// local stuff


static BOOL LXWP1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO)
{
  //  TCHAR ctemp[80];
  (void)GPS_INFO;
  // do nothing!
  return TRUE;
}


static BOOL LXWP2(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO)
{
  TCHAR ctemp[80];
  (void)GPS_INFO;

  NMEAParser::ExtractParameter(String,ctemp,0);
  MACCREADY = StrToDouble(ctemp,NULL);
  return TRUE;
}


static BOOL LXWP0(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO) {
  TCHAR ctemp[80];

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
  double alt, airspeed;
  
  NMEAParser::ExtractParameter(String,ctemp,1);
  airspeed = StrToDouble(ctemp,NULL)/TOKPH;

  NMEAParser::ExtractParameter(String,ctemp,2);
  alt = StrToDouble(ctemp,NULL);

  GPS_INFO->IndicatedAirspeed = airspeed/AirDensityRatio(alt);
  GPS_INFO->TrueAirspeed = airspeed;

  if (d == pDevPrimaryBaroSource){
    GPS_INFO->BaroAltitudeAvailable = TRUE;
    #if NEWQNH
    GPS_INFO->BaroAltitude = AltitudeToQNHAltitude(alt); // 100129
    #else
    GPS_INFO->BaroAltitude = alt;    // ToDo check if QNH correction is needed!
    #endif
  }

  NMEAParser::ExtractParameter(String,ctemp,3);
  GPS_INFO->Vario = StrToDouble(ctemp,NULL);

  GPS_INFO->AirspeedAvailable = TRUE;
  GPS_INFO->VarioAvailable = TRUE;

  TriggerVarioUpdate();

  return TRUE;
}
