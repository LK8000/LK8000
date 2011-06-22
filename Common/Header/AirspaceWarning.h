/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef AIRSPACE_WARNING_H
#define AIRSPACE_WARNING_H

#include "Calculations.h"

#ifdef LKAIRSPACE
extern int LKAirspaceDistance(NMEA_INFO *Basic, DERIVED_INFO *Calculated, 
                                bool Predicted, const CAirspace *airspace,
                                bool ackDay=false);
#else
extern void AirspaceWarnListAdd(NMEA_INFO *Basic, DERIVED_INFO *Calculated, 
                                bool Predicted, bool IsCircle, int AsIdx,
                                bool ackDay=false);

extern int LKAirspaceDistance(NMEA_INFO *Basic, DERIVED_INFO *Calculated, 
                                bool Predicted, bool IsCircle, int AsIdx,
                                bool ackDay=false);

extern void AirspaceWarnListProcess(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
#endif


#endif
