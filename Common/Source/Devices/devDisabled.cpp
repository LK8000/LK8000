/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#include "devDisabled.h"



void disInstall(PDeviceDescriptor_t d){
  _tcscpy(d->Name, _T(DEV_DISABLED_NAME)); // no tokens here!
}
