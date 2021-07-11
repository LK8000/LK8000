/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: AirspaceWarning.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef AIRSPACE_WARNING_H
#define AIRSPACE_WARNING_H


extern int LKAirspaceDistance(NMEA_INFO *Basic, DERIVED_INFO *Calculated, 
                                bool Predicted, const CAirspace *airspace,
                                bool ackDay=false);


#endif
