/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "InputEvents.h"



void CheckTransitionFinalGlide(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  int FinalWayPoint = getFinalWaypoint();
  if (!ValidTaskPoint(FinalWayPoint)) return;

  // update final glide mode status
  if (((ActiveTaskPoint == FinalWayPoint)
       ||(ForceFinalGlide)) 
      && (ValidTaskPoint(ActiveTaskPoint))) {
    
    if (Calculated->FinalGlide == false)
      InputEvents::processGlideComputer(GCE_FLIGHTMODE_FINALGLIDE);
    Calculated->FinalGlide = true;
  } else {
    if (Calculated->FinalGlide == true)
      InputEvents::processGlideComputer(GCE_FLIGHTMODE_CRUISE);
    Calculated->FinalGlide = false;
  }

}


void CheckForceFinalGlide(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  // Auto Force Final Glide forces final glide mode
  // if above final glide...
    if (AutoForceFinalGlide) {
      if (!Calculated->FinalGlide) {
        if (Calculated->TaskAltitudeDifference>120) {
          ForceFinalGlide = true;
        } else {
          ForceFinalGlide = false;
        }
      } else {
        if (Calculated->TaskAltitudeDifference<-120) {
          ForceFinalGlide = false;
        } else {
          ForceFinalGlide = true;
        }
      }
    }
}


