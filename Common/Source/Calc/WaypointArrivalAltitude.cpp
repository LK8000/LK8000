/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"
#include "NavFunctions.h"
#include "Calc/Task/TimeGates.h"

void simpleETE(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int i) {
   if (Basic->Speed <1 || !Calculated->Flying || Calculated->Circling) {
       return;
   }
   BUGSTOP_LKASSERT(ValidWayPoint(i));
   if (!ValidWayPoint(i)) return;

   WayPointCalc[i].NextETE= WayPointCalc[i].Distance / Basic->Speed;
   if (Calculated->AverageGS>0) {
       WayPointCalc[i].NextAvrETE = WayPointCalc[i].Distance / Calculated->AverageGS;
   } else {
       WayPointCalc[i].NextAvrETE= -1;
   }
}

// This is also called by DoNearest and it is overwriting AltitudeRequired
double CalculateWaypointArrivalAltitude(NMEA_INFO* Basic, DERIVED_INFO* Calculated, int i) {
  ScopeLock lock(CritSec_TaskData);

  double wDistance, wBearing;
  DistanceBearing(Basic->Latitude, Basic->Longitude,
                  WayPointList[i].Latitude, WayPointList[i].Longitude,
                  &wDistance, &wBearing);

  WayPointCalc[i].Distance = wDistance;
  WayPointCalc[i].Bearing = wBearing;

  if (ISCAR) {
    simpleETE(Basic, Calculated, i);
    return (Basic->Altitude - WayPointList[i].Altitude);
  }

  double wStartDistance = 0, wStartBearing = 0;
  double safetyaltitudearrival = GetSafetyAltitude(i);

  double altReqd = GlidePolar::MacCreadyAltitude(GetMacCready(i, GMC_DEFAULT), 
                                                 wDistance, wBearing, 
                                                 Calculated->WindSpeed, Calculated->WindBearing,
                                                 0, 0, true, &WayPointCalc[i].NextETE);
  // if gates are in use with a real task, and we are at start
  // then calculate ETE for reaching the cylinder. Also working when we are
  // in the wrong side of cylinder
  if (UseGates() && !DoOptimizeRoute()) {
    if (ActiveTaskPoint == 0 && i == Task[0].Index) {
      if (Calculated->IsInSector) {
        // start in, correct side is inside cylinder
        // or start out,  but inside cylinder
        wStartDistance = StartRadius - wDistance;
        wStartBearing = AngleLimit360(wBearing + 180);
      } else {
        // start out, from outside cylinder
        // or start in, and we are still outside
        wStartDistance = wDistance - StartRadius;
        wStartBearing = wBearing;
      }

      // we don't use GetMacCready(i,GMC_DEFAULT)
      GlidePolar::MacCreadyAltitude(MACCREADY, wStartDistance, wStartBearing, Calculated->WindSpeed,
                                    Calculated->WindBearing, 0, 0, true, &WayPointCalc[i].NextETE);
      DebugLog(_T("wStartDistance=%f wStartBearing=%f\n"), wStartDistance, wStartBearing);
    }
  }

  double TotalAltitude = WayPointList[i].Altitude + safetyaltitudearrival + altReqd;

  WayPointCalc[i].AltReqd[AltArrivMode] = TotalAltitude - Calculated->EnergyHeight;
  WayPointCalc[i].AltArriv[AltArrivMode] = Calculated->NavAltitude + Calculated->EnergyHeight - TotalAltitude;

  // for GA recalculate simple ETE
  if (ISGAAIRCRAFT) {
    simpleETE(Basic, Calculated, i);
  }

  return (WayPointCalc[i].AltArriv[AltArrivMode]);
}
