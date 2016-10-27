/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#include "devGeneric.h"



BOOL genInstall(PDeviceDescriptor_t d){
  _tcscpy(d->Name, TEXT("Generic"));
  d->ParseNMEA = NULL;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = NULL;
  d->Declare = NULL;
  d->IsLogger = NULL;
  d->IsGPSSource = NULL;
  d->IsBaroSource = NULL;
  d->PutQNH = NULL;
  d->OnSysTicker = NULL;

  return(TRUE);

}


BOOL genRegister(void){
  return(devRegister(
    TEXT("Generic"),
      (1l << dfGPS)
    ,
    genInstall
  ));
}

BOOL internalInstall(PDeviceDescriptor_t d){
  _tcscpy(d->Name, TEXT("Internal"));
  d->ParseNMEA = NULL;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = NULL;
  d->Declare = NULL;
  d->IsLogger = NULL;
  d->IsGPSSource = NULL;
  d->IsBaroSource = NULL;
  d->PutQNH = NULL;
  d->OnSysTicker = NULL;

  return(TRUE);

}


BOOL InternalRegister(void){
  return(devRegister(
    TEXT("Internal"),
      (1l << dfGPS)
    ,
    internalInstall
  ));
}
