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

#include "devZander.h"

#include "utils/heapcheck.h"

static BOOL PZAN1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *aGPS_INFO);
static BOOL PZAN2(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *aGPS_INFO);


static BOOL ZanderParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *aGPS_INFO){
  (void)d;

  if(_tcsncmp(TEXT("$PZAN1"), String, 6)==0)
    {
      return PZAN1(d, &String[7], aGPS_INFO);
    } 
  if(_tcsncmp(TEXT("$PZAN2"), String, 6)==0)
    {
      return PZAN2(d, &String[7], aGPS_INFO);
    } 

  return FALSE;

}


/*
static BOOL ZanderIsLogger(PDeviceDescriptor_t d){
  (void)d;
  return(FALSE);
}
*/


static BOOL ZanderIsGPSSource(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE); 
}


static BOOL ZanderIsBaroSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}


static BOOL ZanderLinkTimeout(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


static BOOL zanderInstall(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("Zander"));
  d->ParseNMEA = ZanderParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = ZanderLinkTimeout;
  d->Declare = NULL;
  d->IsGPSSource = ZanderIsGPSSource;
  d->IsBaroSource = ZanderIsBaroSource;

  return(TRUE);

}


BOOL zanderRegister(void){
  return(devRegister(
    TEXT("Zander"),
    (1l << dfGPS)
    | (1l << dfBaroAlt)
    | (1l << dfSpeed)
    | (1l << dfVario),
    zanderInstall
  ));
}


// *****************************************************************************
// local stuff

static BOOL PZAN1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *aGPS_INFO)
{
  TCHAR ctemp[80];
  NMEAParser::ExtractParameter(String,ctemp,0);
  #if DUALBARO
  if (d == pDevPrimaryBaroSource) {
  	aGPS_INFO->BaroAltitude = AltitudeToQNHAltitude(StrToDouble(ctemp,NULL));
  	aGPS_INFO->BaroAltitudeAvailable = TRUE;
  }
  #else
  aGPS_INFO->BaroAltitude = AltitudeToQNHAltitude(StrToDouble(ctemp,NULL));
  aGPS_INFO->BaroAltitudeAvailable = TRUE;
  #endif
  return TRUE;
}


static BOOL PZAN2(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *aGPS_INFO)
{
  TCHAR ctemp[80];
  double vtas, wnet, vias;

  NMEAParser::ExtractParameter(String,ctemp,0);
  vtas = StrToDouble(ctemp,NULL)/3.6;
  // JMW 20080721 fixed km/h->m/s conversion
  
  NMEAParser::ExtractParameter(String,ctemp,1);
  wnet = (StrToDouble(ctemp,NULL)-10000)/100; // cm/s
  aGPS_INFO->Vario = wnet;

  if (aGPS_INFO->BaroAltitudeAvailable) {
    vias = vtas/AirDensityRatio(aGPS_INFO->BaroAltitude);
  } else {
    vias = 0.0;
  }

  aGPS_INFO->AirspeedAvailable = TRUE;
  aGPS_INFO->TrueAirspeed = vtas;
  aGPS_INFO->IndicatedAirspeed = vias;
  aGPS_INFO->VarioAvailable = TRUE;

  TriggerVarioUpdate();

  return TRUE;
}
