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

void genInstall(PDeviceDescriptor_t d){
  _tcscpy(d->Name, TEXT("Generic"));
  d->IsGPSSource = &IsGPSSource;
}

void internalInstall(PDeviceDescriptor_t d){
  _tcscpy(d->Name, DEV_INTERNAL_NAME);
  d->IsGPSSource = &IsGPSSource;
}
