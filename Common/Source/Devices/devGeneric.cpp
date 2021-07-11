/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#include "devGeneric.h"


static
BOOL IsGPSSource(PDeviceDescriptor_t d) {
  return TRUE;
}

BOOL genInstall(PDeviceDescriptor_t d){
  _tcscpy(d->Name, TEXT("Generic"));
  d->IsGPSSource = &IsGPSSource;
  return TRUE;
}


BOOL genRegister(){
  return(devRegister(
    TEXT("Generic"),
    (1l << dfGPS),
    genInstall
  ));
}

BOOL internalInstall(PDeviceDescriptor_t d){
  _tcscpy(d->Name, TEXT("Internal"));
  d->IsGPSSource = &IsGPSSource;
  return TRUE;
}


BOOL InternalRegister(){
  return(devRegister(
    TEXT("Internal"),
    (1l << dfGPS),
    internalInstall
  ));
}
