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

#include "devBorgeltB50.h"

#include "utils/heapcheck.h"

BOOL PBB50(TCHAR *String, NMEA_INFO *GPS_INFO);


BOOL B50ParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO){
  (void)d;

  if(_tcsncmp(TEXT("$PBB50"), String, 6)==0)
    {
      return PBB50(&String[7], GPS_INFO);
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


BOOL B50LinkTimeout(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


BOOL b50Install(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("Borgelt B50"));
  d->ParseNMEA = B50ParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = B50LinkTimeout;
  d->Declare = NULL;
  d->IsGPSSource = B50IsGPSSource;

  return(TRUE);

}


BOOL b50Register(void){
  return(devRegister(
    TEXT("Borgelt B50"),
    (1l << dfGPS),
    b50Install
  ));
}


// *****************************************************************************
// local stuff

/*
Sentence has following format: 

$PBB50,AAA,BBB.B,C.C,DDDDD,EE,F.FF,G,HH*CHK crlf

 

AAA = TAS 0 to 150 knots 

BBB.B = Vario, -10 to +15 knots, negative sign for sink 

C.C = Macready 0 to 8.0 knots 

DDDDD = IAS squared 0 to 22500 

EE = bugs degradation, 0 = clean to 30 % 

F.FF = Ballast 1.00 to 1.60 

G = 0 in climb, 1 in cruise 

HH = Outside airtemp in degrees celcius ( may have leading negative sign ) 

CHK = standard NMEA checksum 


*/

BOOL PBB50(TCHAR *String, NMEA_INFO *GPS_INFO) {
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
  GPS_INFO->MacReady = StrToDouble(ctemp,NULL)/TOKNOTS;
  MACCREADY = GPS_INFO->MacReady;

  NMEAParser::ExtractParameter(String,ctemp,3);
  vias = sqrt(StrToDouble(ctemp,NULL))/TOKNOTS;

  // RMN: Changed bugs-calculation, swapped ballast and bugs to suit
  // the B50-string for Borgelt, it's % degradation, for us, it is %
  // of max performance
  /*

  JMW disabled bugs/ballast due to problems with test b50

  NMEAParser::ExtractParameter(String,ctemp,4);
  GPS_INFO->Bugs = 1.0-max(0,min(30,StrToDouble(ctemp,NULL)))/100.0;
  BUGS = GPS_INFO->Bugs;

  // for Borgelt it's % of empty weight,
  // for us, it's % of ballast capacity
  // RMN: Borgelt ballast->XCSoar ballast

  NMEAParser::ExtractParameter(String,ctemp,5);
  double bal = max(1.0,min(1.60,StrToDouble(ctemp,NULL)))-1.0;
  if (WEIGHTS[2]>0) {
    GPS_INFO->Ballast = min(1.0, max(0.0,
                                     bal*(WEIGHTS[0]+WEIGHTS[1])/WEIGHTS[2]));
    BALLAST = GPS_INFO->Ballast;
  } else {
    GPS_INFO->Ballast = 0;
    BALLAST = 0;
  }
  // w0 pilot weight, w1 glider empty weight, w2 ballast weight
  */

  // inclimb/incruise 1=cruise,0=climb, OAT
  NMEAParser::ExtractParameter(String,ctemp,6);
  int climb = lround(StrToDouble(ctemp,NULL));

  #if USESWITCHES
  GPS_INFO->SwitchState.VarioCircling = (climb==1);
  #endif

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

  GPS_INFO->AirspeedAvailable = TRUE;
  GPS_INFO->IndicatedAirspeed = vias;
  GPS_INFO->TrueAirspeed = vtas;
  GPS_INFO->VarioAvailable = TRUE;
  GPS_INFO->Vario = wnet;

  TriggerVarioUpdate();

  return FALSE;
}
