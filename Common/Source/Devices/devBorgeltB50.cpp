/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#include "devBorgeltB50.h"

#include "devCAI302.h"


BOOL bBaroAvailable = FALSE;

BOOL PBB50(TCHAR *String, NMEA_INFO *pGPS);

extern BOOL vl_PGCS1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS);

BOOL B50ParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){
  (void)d;

  if(_tcsncmp(TEXT("$PBB50"), String, 6)==0)
    return PBB50(&String[7], pGPS);
  else
    if(_tcsncmp(TEXT("$PGCS"), String, 5)==0)
    {
      bBaroAvailable = true;
      return vl_PGCS1( d, &String[6], pGPS);
    }
//  if(_tcsstr(String,TEXT("$PGCS,")) == String){
//    return vl_PGCS1(d, &String[6], pGPS);
#ifdef GPRMZ__
    else
      if(_tcsncmp(TEXT("$GPRMZ"), String, 6)==0)
      {
    	TCHAR ctemp[80], *params[5];
    	int nparams = NMEAParser::ValidateAndExtract(String, ctemp, 80, params, 5);
    	if (nparams < 1)
    	  return FALSE;

    	  double altitude = NMEAParser::ParseAltitude(params[1], params[2]);
    	  UpdateBaroSource( pGPS, BARO__RMZ, d, AltitudeToQNHAltitude(altitude));

    	return TRUE;
      }
#endif


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
  d->PutMacCready = cai302PutMacCready;
  d->PutBugs = cai302PutBugs;
  d->PutBallast = cai302PutBallast;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = B50LinkTimeout;
  d->Declare = NULL;
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


// *****************************************************************************
// local stuff

/*
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

BOOL PBB50(TCHAR *String, NMEA_INFO *pGPS) {
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
  MACCREADY = pGPS->MacReady;

  NMEAParser::ExtractParameter(String,ctemp,3);
  vias = sqrt(StrToDouble(ctemp,NULL))/TOKNOTS;

  // RMN: Changed bugs-calculation, swapped ballast and bugs to suit
  // the B50-string for Borgelt, it's % degradation, for us, it is %
  // of max performance
  /*

  JMW disabled bugs/ballast due to problems with test b50
*/
  NMEAParser::ExtractParameter(String,ctemp,4);
 double bugs = StrToDouble(ctemp,NULL);
 bugs = (100.0-bugs)/100.0;
 if(bugs > 1.0) bugs = 1.0;
 if(bugs < 0.1) bugs = 0.1;
 BUGS = bugs;
 pGPS->Bugs = BUGS;
		 // pGPS->Bugs;
//  StartupStore(TEXT(">>>>>BUGS<<<< %s %f "),ctemp, BUGS);
  // for Borgelt it's % of empty weight,
  // for us, it's % of ballast capacity
  // RMN: Borgelt ballast->XCSoar ballast

  NMEAParser::ExtractParameter(String,ctemp,5);
  double bal = StrToDouble(ctemp,NULL);
  BALLAST =  (bal-1.0)/0.6;
  /*************************************************/
 // StartupStore(TEXT(">NMEA:$PBB50,%s                                    BUG:%d %4.2f:BAL %s%s"),String, (int)BUGS,BALLAST, NEWLINE, NEWLINE);
  /*************************************************/
  /*
  if (WEIGHTS[2]>0) {
    pGPS->Ballast = min(1.0, max(0.0,
                                     bal*(WEIGHTS[0]+WEIGHTS[1])/WEIGHTS[2]));
    BALLAST = pGPS->Ballast;
  } else {
    pGPS->Ballast = 0;
    BALLAST = 0;
  }
  */
  /*
  // w0 pilot weight, w1 glider empty weight, w2 ballast weight
  */

  // inclimb/incruise 1=cruise,0=climb, OAT
  NMEAParser::ExtractParameter(String,ctemp,6);
  

  #if USESWITCHES
  int climb = lround(StrToDouble(ctemp,NULL));
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
//  if()
  {
    pGPS->OutsideAirTemperature = StrToDouble(ctemp,NULL);
    pGPS->TemperatureAvailable = true;
  }

  pGPS->AirspeedAvailable = TRUE;
  pGPS->IndicatedAirspeed = vias;
  pGPS->TrueAirspeed = vtas;
  pGPS->VarioAvailable = TRUE;
  pGPS->Vario = wnet;

  TriggerVarioUpdate();

  return FALSE;
}
