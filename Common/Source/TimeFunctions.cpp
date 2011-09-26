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

int DetectStartTime(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  // JMW added restart ability
  //
  // we want this to display landing time until next takeoff 

  static int starttime = -1;
  static int lastflighttime = -1;

  if (Calculated->Flying) {
    if (starttime == -1) {
      // hasn't been started yet
      
      starttime = (int)GPS_INFO.Time;

      lastflighttime = -1;
    }
    // bug? if Basic time rolled over 00 UTC, negative value is returned
    return (int)GPS_INFO.Time-starttime;

  } else {

    if (lastflighttime == -1) {
      // hasn't been stopped yet
      if (starttime>=0) {
	lastflighttime = (int)Basic->Time-starttime;
      } else {
	return 0; // no last flight time
      }
      // reset for next takeoff
      starttime = -1;
    }
  }

  // return last flighttime if it exists
  return max(0,lastflighttime);
}

