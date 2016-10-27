/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: devNmeaOut.cpp,v 8.2 2011/01/02 00:32:55 root Exp root $
*/

#include "externs.h"



BOOL nmoParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){

  (void) d;
  (void) String;
  (void) pGPS;

  return FALSE;

}


BOOL nmoIsLogger(PDeviceDescriptor_t d){
  (void)d;
  return(FALSE);
}

BOOL nmoIsGPSSource(PDeviceDescriptor_t d){
  (void)d;
  return(FALSE);  // this is only true if GPS source is connected on VEGA.NmeaIn
}

BOOL nmoIsBaroSource(PDeviceDescriptor_t d){
  (void)d;
  return(FALSE);
}

BOOL nmoPutVoice(PDeviceDescriptor_t d, TCHAR *Sentence){
  return(FALSE);
}

BOOL nmoPutQNH(DeviceDescriptor_t *d, double NewQNH){
  (void)NewQNH;
  return(FALSE);
}

BOOL nmoOnSysTicker(DeviceDescriptor_t *d){
  return(FALSE);
}


BOOL nmoInstall(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("NmeaOut"));
  d->ParseNMEA = nmoParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = NULL;
  d->Declare = NULL;
  d->IsLogger = nmoIsLogger;
  d->IsGPSSource = nmoIsGPSSource;
  d->IsBaroSource = nmoIsBaroSource;
  d->PutVoice = nmoPutVoice;
  d->PutQNH = nmoPutQNH;
  d->OnSysTicker = nmoOnSysTicker;

  return(TRUE);

}


BOOL nmoRegister(void){
  return(devRegister(
    TEXT("NmeaOut"),
    (1l << dfNmeaOut),
    nmoInstall
  ));
}
