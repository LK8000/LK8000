/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

// simple localtime with no 24h exceeding
int LocalTime(int utc_time) {
  int localtime = utc_time + GetUTCOffset();
  return (localtime % (24 * 3600));
}

int LocalTime() {
  return LocalTime(GPS_INFO.Time);
}


