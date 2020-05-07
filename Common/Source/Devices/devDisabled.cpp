/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#include "devDisabled.h"



BOOL disInstall(PDeviceDescriptor_t d){
  _tcscpy(d->Name, _T(DEV_DISABLED_NAME)); // no tokens here!
  return(TRUE);
}


BOOL disRegister(void){
  return(devRegister(
    _T(DEV_DISABLED_NAME),
    (1l << dfGPS),
    disInstall
  ));
}
