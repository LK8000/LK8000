/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "devBorgeltB50.h"
#include "Calc/Vario.h"

BOOL bBaroAvailable = FALSE;

static BOOL PBB50(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS);

extern BOOL vl_PGCS1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS);


BOOL B50ParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){
  (void)d;

  if(_tcsncmp(TEXT("$PBB50"), String, 6)==0)
    return PBB50(d, &String[7], pGPS);
  else
    if(_tcsncmp(TEXT("$PGCS"), String, 5)==0)
    {
      bBaroAvailable = true;
      return vl_PGCS1( d, &String[6], pGPS);
    }

  return FALSE;

}


BOOL B50IsLogger(PDeviceDescriptor_t d){
  (void)d;
  return(FALSE);
}


BOOL B50IsGPSSource(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE); // ? is this true
}


BOOL B50IsBaroSource(PDeviceDescriptor_t d){
  (void)d;
  return(bBaroAvailable); // ? is this true
}

BOOL B50LinkTimeout(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


BOOL b50Install(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("Borgelt B50"));
  d->ParseNMEA = B50ParseNMEA;
  d->LinkTimeout = B50LinkTimeout;
  d->IsGPSSource = B50IsGPSSource;
  d->IsGPSSource  = B50IsGPSSource;
  d->IsBaroSource = B50IsBaroSource;
  return(TRUE);

}


BOOL b50Register(void){
  return(devRegister(
    TEXT("Borgelt B50"),
    (1l << dfGPS),
    b50Install
  ));
}



/*
 NOTICE: DO NOT TRUST THE FOLLOWING. BORGELT IS NOT WORKING LIKE THAT.
 NOBODY KNOWS HOW IT WORKS, THE DOCS ARE WRONG, THE NMEA LOG PROVE INCONSISTENT VALUES.
 NO HOPE TO EVER FIX IT.

Sentence has following format:

$PBB50,AAA,BBB.B,C.C,DDDDD,EE,F.FF,G,HH*CHK crlf

$PBB50,14,-.1,3.3,196,0,1.05,0,1*59

0 AAA = TAS 0 to 150 knots  14

1 BBB.B = Vario, -10 to +15 knots, negative sign for sink -.1

2 C.C = Macready 0 to 8.0 knots  3.3

3 DDDDD = IAS squared 0 to 22500  196

4 EE = bugs degradation, 0 = clean to 30 %  0

5 F.FF = Ballast 1.00 to 1.60 1.05

6 G = 0 in climb, 1 in cruise 0

7 HH = Outside airtemp in degrees celcius ( may have leading negative sign ) 1

CHK = standard NMEA checksum

*/

BOOL PBB50(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS) {
  // $PBB50,100,0,10,1,10000,0,1,0,20*4A..
  // $PBB50,0,.0,.0,0,0,1.07,0,-228*58
  // $PBB50,14,-.2,.0,196,0,.92,0,-228*71

  double vtas, vias, wnet;
  TCHAR ctemp[80];

  NMEAParser::ExtractParameter(String,ctemp,0);
  vtas = StrToDouble(ctemp,NULL)/TOKNOTS;

  NMEAParser::ExtractParameter(String,ctemp,1);
  wnet = StrToDouble(ctemp,NULL)/TOKNOTS;

  NMEAParser::ExtractParameter(String,ctemp,2);
  pGPS->MacReady = StrToDouble(ctemp,NULL)/TOKNOTS;
  CheckSetMACCREADY(pGPS->MacReady);

  NMEAParser::ExtractParameter(String,ctemp,3);
  vias = sqrt(StrToDouble(ctemp,NULL))/TOKNOTS;

  // inclimb/incruise 1=cruise,0=climb, OAT
  NMEAParser::ExtractParameter(String,ctemp,6);


  #if USESWITCHES
  int climb = iround(StrToDouble(ctemp,NULL));
  pGPS->SwitchState.VarioCircling = (climb==1);
  #endif

  #if 0 // UNUSED EnableExternalTriggerCruise
  if (EnableExternalTriggerCruise) {
    if (climb) {
      ExternalTriggerCruise = false;
      ExternalTriggerCircling = true;
    } else {
      ExternalTriggerCruise = true;
      ExternalTriggerCircling = false;
    }
  } else {
    ExternalTriggerCruise = false;
  }
  #endif

  NMEAParser::ExtractParameter(String,ctemp,7);
  pGPS->OutsideAirTemperature = StrToDouble(ctemp,NULL);
  pGPS->TemperatureAvailable = true;

  pGPS->AirspeedAvailable = TRUE;
  pGPS->IndicatedAirspeed = vias;
  pGPS->TrueAirspeed = vtas;

  UpdateVarioSource(*pGPS, *d, wnet);

  return FALSE;
}
