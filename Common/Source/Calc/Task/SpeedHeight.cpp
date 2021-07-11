/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"




double SpeedHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  (void)Basic;
  if (Calculated->TaskDistanceToGo<=0) {
    return 0;
  }

  // Fraction of task distance covered
  double d_fraction = Calculated->TaskDistanceCovered/
    (Calculated->TaskDistanceCovered+Calculated->TaskDistanceToGo);

  double dh_start = Calculated->TaskStartAltitude;

  double dh_finish = FAIFinishHeight(Basic, Calculated, -1);

  // Excess height
  return Calculated->NavAltitude 
    - (dh_start*(1.0-d_fraction)+dh_finish*(d_fraction));
}

