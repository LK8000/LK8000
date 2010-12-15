/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

*/
#include "StdAfx.h"


#include "externs.h"
#include "Utils.h"
#include "Parser.h"
#include "Port.h"

#include "devAltairPro.h"

double AltitudeToQNHAltitude(double alt);

static double lastAlt = 0;


BOOL atrParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO){

  // no propriatary sentence

  if (_tcsncmp(TEXT("$PGRMZ"), String, 6) == 0){

    TCHAR  *pWClast = NULL;
    TCHAR  *pToken;

    // eat $PGRMZ
    if ((pToken = strtok_r(String, TEXT(","), &pWClast)) == NULL)
      return FALSE;

    // get <alt>
    if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL)
      return FALSE;

    lastAlt = StrToDouble(pToken, NULL);; 
    
    // get <unit>
    if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL)
      return FALSE;

    if (*pToken == 'f' || *pToken== 'F')
      lastAlt /= TOFEET;

    
    if (d == pDevPrimaryBaroSource){
      GPS_INFO->BaroAltitudeAvailable = TRUE;
      GPS_INFO->BaroAltitude = AltitudeToQNHAltitude(lastAlt);
    }

    return(TRUE);
  }
  
  return FALSE;

}


BOOL atrDeclare(PDeviceDescriptor_t d, Declaration_t *decl){
  (void) d;
  (void) decl;

  // TODO feature: Altair declaration

  return(TRUE);

}


BOOL atrIsLogger(PDeviceDescriptor_t d){
//  return(TRUE);
  (void)d;
  return(FALSE);
}


BOOL atrIsGPSSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}

BOOL atrIsBaroSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}

BOOL atrPutQNH(DeviceDescriptor_t *d, double NewQNH){
  (void)NewQNH; // TODO code: JMW check sending QNH to Altair
  if (d == pDevPrimaryBaroSource){
    GPS_INFO.BaroAltitude = AltitudeToQNHAltitude(lastAlt);
  }

  return(TRUE);
}

BOOL atrOnSysTicker(DeviceDescriptor_t *d){
  (void)d;
  // Do To get IO data like temp, humid, etc
  
  return(TRUE);
}

BOOL atrInstall(PDeviceDescriptor_t d){
  _tcscpy(d->Name, TEXT("Altair Pro"));
  d->ParseNMEA = atrParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = NULL;
  d->Declare = atrDeclare;
  d->IsLogger = atrIsLogger;
  d->IsGPSSource = atrIsGPSSource;
  d->IsBaroSource = atrIsBaroSource;
  d->PutQNH = atrPutQNH;
  d->OnSysTicker = atrOnSysTicker;

  return(TRUE);

}


BOOL atrRegister(void){
  return(devRegister(
    TEXT("Altair Pro"), 
      (1l << dfGPS)
    | (1l << dfBaroAlt)
//      | 1l << dfLogger  // ToDo
    ,
    atrInstall
  ));
}

