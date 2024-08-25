/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"



//
// twp is a task index reference, not a waypoint index
// CAREFUL> FAIFinishHeight is considering SafetyAltitude if enabled for the wp type.
// in Calculations  height_above_finish is the difference between first and last task wp,
// but they may have different safetyaltitude appliances! This is why it should not be
// allowed to enter a landable inside a task until we get rid of this stuff.
//
double FAIFinishHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int twp) {

  int FinalWayPoint = getFinalWaypoint();
  if (!ValidTaskPoint(FinalWayPoint)) return 0;

  double safetyaltitudearrival=SAFETYALTITUDEARRIVAL/10; 

  if (twp== -1) {
    twp = FinalWayPoint;
  }

  double wp_alt;

  if(ValidTaskPoint(twp)) {
    // for PG Optimized task, always use optimized point altitude.
    if( DoOptimizeRoute() ) {
      wp_alt = Task[twp].AATTargetAltitude;
    } else {
      wp_alt = WayPointList[Task[twp].Index].Altitude;
    }
    if (!CheckSafetyAltitudeApplies(Task[twp].Index)) safetyaltitudearrival=0;
  } else {
    TestLog(_T("..... FAIFinishHeight invalid twp=%d"), twp);
    wp_alt = 0;
  }

  if (twp==FinalWayPoint) {
    if (EnableFAIFinishHeight && !UseAATTarget()) {
      // maximum allowed loss of height in order to conform to FAI rules
      double maxHeightLoss = min(1000.0,
                                 (Calculated->TaskDistanceCovered+Calculated->TaskDistanceToGo) * 0.01);
      
      return max(max(FinishMinHeight/1000.0, safetyaltitudearrival)+ wp_alt, 
                 Calculated->TaskStartAltitude-maxHeightLoss);
    } else {
      return max(FinishMinHeight/1000.0, safetyaltitudearrival)+wp_alt;
    }
  } else {
    return wp_alt + safetyaltitudearrival;
  }
}

