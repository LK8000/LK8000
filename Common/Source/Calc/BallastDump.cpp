/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"

void BallastDump() {

  static double BallastTimeLast = -1;
  if (BallastTimerActive) {

    // JMW only update every 5 seconds to stop flooding the devices
    if (GPS_INFO.Time > BallastTimeLast + 5) {

      double dt = GPS_INFO.Time - BallastTimeLast;
      double percent_per_second = 1.0 / max(10, BallastSecsToEmpty);
      BALLAST -= dt * percent_per_second;
      if (BALLAST < 0) {
        BallastTimerActive = false;
        BALLAST = 0.0;
      }

      GlidePolar::SetBallast();
      devPutBallast(BALLAST);

      BallastTimeLast = GPS_INFO.Time;
    }
  } else {
    BallastTimeLast = GPS_INFO.Time;
  }
}
