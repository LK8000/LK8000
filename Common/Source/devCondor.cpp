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

#include "devCondor.h"

#include "utils/heapcheck.h"

static BOOL cLXWP0(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO);
static BOOL cLXWP1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO);
static BOOL cLXWP2(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO);


static BOOL CondorParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO){
  (void)d;
  if(_tcsncmp(TEXT("$LXWP0"), String, 6)==0)
    {
      return cLXWP0(d, &String[7], GPS_INFO);
    } 
  if(_tcsncmp(TEXT("$LXWP1"), String, 6)==0)
    {
      return cLXWP1(d, &String[7], GPS_INFO);
    } 
  if(_tcsncmp(TEXT("$LXWP2"), String, 6)==0)
    {
      return cLXWP2(d, &String[7], GPS_INFO);
    }

  return FALSE;

}

static BOOL CondorIsCondor(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE); 
}


static BOOL CondorIsGPSSource(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE); 
}


static BOOL CondorIsBaroSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}


static BOOL CondorLinkTimeout(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


static BOOL condorInstall(PDeviceDescriptor_t d){

  // StartupStore(_T(". Condor device installed%s"),NEWLINE);
  _tcscpy(d->Name, TEXT("Condor"));
  d->ParseNMEA = CondorParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = CondorLinkTimeout;
  d->Declare = NULL;
  d->IsGPSSource = CondorIsGPSSource;
  d->IsBaroSource = CondorIsBaroSource;
  d->IsCondor = CondorIsCondor;

  return(TRUE);

}


BOOL condorRegister(void){
  return(devRegister(
    TEXT("Condor"),
    (1l << dfGPS)
    | (1l << dfBaroAlt)
    | (1l << dfSpeed)
    | (1l << dfVario),
    condorInstall
  ));
}


// *****************************************************************************
// local stuff


static BOOL cLXWP1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO)
{
  //  TCHAR ctemp[80];
  (void)GPS_INFO;
  // do nothing!
  return TRUE;
}


static BOOL cLXWP2(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO)
{
  TCHAR ctemp[80];
  (void)GPS_INFO;

  NMEAParser::ExtractParameter(String,ctemp,0);
  MACCREADY = StrToDouble(ctemp,NULL);
  return TRUE;
}


static BOOL cLXWP0(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO) {
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
  double alt, airspeed, wspeed, wfrom;
 
  NMEAParser::ExtractParameter(String,ctemp,1);
  airspeed = StrToDouble(ctemp,NULL)/TOKPH;

  NMEAParser::ExtractParameter(String,ctemp,2);
  alt = StrToDouble(ctemp,NULL);

  GPS_INFO->IndicatedAirspeed = airspeed/AirDensityRatio(alt);
  GPS_INFO->TrueAirspeed = airspeed;

  if (d == pDevPrimaryBaroSource){
    GPS_INFO->BaroAltitudeAvailable = TRUE;
    GPS_INFO->BaroAltitude = AltitudeToQNHAltitude(alt);
  }

  NMEAParser::ExtractParameter(String,ctemp,3);
  GPS_INFO->Vario = StrToDouble(ctemp,NULL);

  GPS_INFO->AirspeedAvailable = TRUE;
  GPS_INFO->VarioAvailable = TRUE;

  // we don't use heading for wind calculation since... wind is already calculated in condor!!
  NMEAParser::ExtractParameter(String,ctemp,11);
  wspeed=StrToDouble(ctemp,NULL);
  NMEAParser::ExtractParameter(String,ctemp,10);
  wfrom=StrToDouble(ctemp,NULL);

  if (wspeed>0) {

	wfrom+=180;
	if (wfrom==360) wfrom=0;
	if (wfrom>360) wfrom-=360;
	wspeed/=3.6;

	// do not update if it has not changed
	if ( (wspeed!=CALCULATED_INFO.WindSpeed) || (wfrom != CALCULATED_INFO.WindBearing) ) {

		SetWindEstimate(wspeed, wfrom,9);
		CALCULATED_INFO.WindSpeed=wspeed;
		CALCULATED_INFO.WindBearing=wfrom;

	}
  }

  // TriggerVarioUpdate(); 

  return TRUE;
}
