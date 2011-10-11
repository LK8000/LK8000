/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"
#include <windows.h>

#include "options.h"
#include "Defines.h"
#include "lk8000.h"
#include "externs.h"

using std::min;
using std::max;


int TimeLocal(int localtime) {
  localtime += GetUTCOffset();
  if (localtime<0) {
    localtime+= 3600*24;
  }
  return localtime;
}

int DetectCurrentTime() {
  int localtime = (int)GPS_INFO.Time;
  return TimeLocal(localtime);
}

// simple localtime with no 24h exceeding
int LocalTime() {
  int localtime = (int)GPS_INFO.Time;
  localtime += GetUTCOffset();
  if (localtime<0) {
	localtime+= 86400;
  } else {
	if (localtime>86400) {
		localtime -=86400;
	}
  }
  return localtime;
}

