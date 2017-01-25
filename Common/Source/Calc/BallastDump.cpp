/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"


 void BallastDump ()
 {
   static double BallastTimeLast = -1;

   if (BallastTimerActive) {
         // JMW only update every 5 seconds to stop flooding the devices
     if (GPS_INFO.Time > BallastTimeLast+5) {
 //      double BALLAST_last = BALLAST;
       double dt = GPS_INFO.Time - BallastTimeLast;
       double percent_per_second = 1.0/max(10, BallastSecsToEmpty);
       BALLAST -= dt*percent_per_second;
       if (BALLAST<0) {
         BallastTimerActive = false;
         BALLAST = 0.0;
         GlidePolar::SetBallast(); 
         devPutBallast(devAll(), BALLAST); //

       }
  //     if (fabs(BALLAST-BALLAST_last)>0.01) removed the change check, will be send every 5s for long ballast dumps
       { // Ulli changed from 5% to 1% because sometimes it stopped at 55%
         GlidePolar::SetBallast();
         devPutBallast(devAll(), BALLAST);

       }
       BallastTimeLast = GPS_INFO.Time;
     }
   } else {
     BallastTimeLast = GPS_INFO.Time;
   }
 }
