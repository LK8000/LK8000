/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifdef PNA

#include "StdAfx.h"
#include "compatibility.h"
#include "options.h"
#include "Defines.h"
#include "externs.h"
#include "utils/heapcheck.h"


bool DeviceIsRoyaltek3200=false;

bool Init_Royaltek3200(void) {
  DeviceIsRoyaltek3200=false;
  return false;
}

void DeInit_Royaltek3200(void) {
  if (!DeviceIsRoyaltek3200) return;
  DeviceIsRoyaltek3200=false;
  return;
}

int Royaltek3200_ReadBarData(void) {
  return 0;
}

float Royaltek3200_GetPressure(void) {
  return 1013.25;
}



#endif // PNA only
