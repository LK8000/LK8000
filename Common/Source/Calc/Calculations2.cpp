/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Calculations2.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "McReady.h"
#include "TeamCodeCalculation.h"
#include "InputEvents.h"
#include "NavFunctions.h"
#include "Time/PeriodClock.hpp"



double MacCreadyTimeLimit(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
			  const double this_bearing,
			  const double time_remaining,
			  const double h_final) {

  // find highest Mc to achieve greatest distance in remaining time and height
  (void)Basic;

  double time_to_go;
  double mc;
  double mc_best = 0.0;
  double d_best = 0.0;
  const double windspeed =   Calculated->WindSpeed;
  const double windbearing = Calculated->WindBearing;
  const double navaltitude = Calculated->NavAltitude;

  for (mc=0; mc<10.0; mc+= 0.1) {

    double h_unit = GlidePolar::MacCreadyAltitude(mc,
						 1.0, // unit distance
						 this_bearing,
						 windspeed,
						 windbearing,
						 NULL,
						 NULL,
						 1, // final glide
						 &time_to_go);
    if (time_to_go>0) {
      double p = time_remaining/time_to_go;
      double h_spent = h_unit*p;
      double dh = navaltitude-h_spent-h_final;
      double d = 1.0*p;

      if ((d>d_best) && (dh>=0)) {
	mc_best = mc;
      }
    }
  }
  return mc_best;
}


PeriodClock lastTeamCodeUpdateTime;

void CalculateOwnTeamCode(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  if (WayPointList.empty()) return;
  if (TeamCodeRefWaypoint < 0) return;

  unsigned TimeOut(10 * 1000); // 10s
  // only calculate each 10s
  if(!lastTeamCodeUpdateTime.CheckUpdate(TimeOut)) return;

  double distance = 0;
  double bearing = 0;
  TCHAR code[10];


  /*
  distance =  Distance(WayPointList[TeamCodeRefWaypoint].Latitude,
	WayPointList[TeamCodeRefWaypoint].Longitude,
	Basic->Latitude,
	Basic->Longitude);

  bearing = Bearing(WayPointList[TeamCodeRefWaypoint].Latitude,
	WayPointList[TeamCodeRefWaypoint].Longitude,
	Basic->Latitude,
	Basic->Longitude);
  */

  DistanceBearing(WayPointList[TeamCodeRefWaypoint].Latitude,
                  WayPointList[TeamCodeRefWaypoint].Longitude,
                  Basic->Latitude,
                  Basic->Longitude,
                  &distance, &bearing);

  GetTeamCode(code, bearing, distance);

  Calculated->TeammateBearing = bearing;
  Calculated->TeammateRange = distance;

  //double teammateBearing = GetTeammateBearingFromRef(TeammateCode);
  //double teammateRange = GetTeammateRangeFromRef(TeammateCode);

  //Calculated->TeammateLongitude = FindLongitude(

  LK_tcsncpy(Calculated->OwnTeamCode, code, 5);
}


void CalculateTeammateBearingRange(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  static bool InTeamSector = false;

  if (WayPointList.empty()) return;
  if (TeamCodeRefWaypoint < 0) return;

  double ownDistance = 0;
  double ownBearing = 0;
  double mateDistance = 0;
  double mateBearing = 0;

  //ownBearing = Bearing(Basic->Latitude, Basic->Longitude,
  //	WayPointList[TeamCodeRefWaypoint].Latitude,
  //	WayPointList[TeamCodeRefWaypoint].Longitude);
  //
  //ownDistance =  Distance(Basic->Latitude, Basic->Longitude,
  //	WayPointList[TeamCodeRefWaypoint].Latitude,
  //	WayPointList[TeamCodeRefWaypoint].Longitude);

  /*
  ownBearing = Bearing(WayPointList[TeamCodeRefWaypoint].Latitude,
                       WayPointList[TeamCodeRefWaypoint].Longitude,
                       Basic->Latitude,
                       Basic->Longitude
                       );
  //
  ownDistance =  Distance(WayPointList[TeamCodeRefWaypoint].Latitude,
                          WayPointList[TeamCodeRefWaypoint].Longitude,
                          Basic->Latitude,
                          Basic->Longitude
                          );
  */

  DistanceBearing(WayPointList[TeamCodeRefWaypoint].Latitude,
                  WayPointList[TeamCodeRefWaypoint].Longitude,
                  Basic->Latitude,
                  Basic->Longitude,
                  &ownDistance, &ownBearing);

  if (TeammateCodeValid)
    {

      //mateBearing = Bearing(Basic->Latitude, Basic->Longitude, TeammateLatitude, TeammateLongitude);
      //mateDistance = Distance(Basic->Latitude, Basic->Longitude, TeammateLatitude, TeammateLongitude);

      CalcTeammateBearingRange(ownBearing, ownDistance,
                               TeammateCode,
                               &mateBearing, &mateDistance);

      // TODO code ....change the result of CalcTeammateBearingRange to do this !
      if (mateBearing > 180)
        {
          mateBearing -= 180;
        }
      else
        {
          mateBearing += 180;
        }


      Calculated->TeammateBearing = mateBearing;
      Calculated->TeammateRange = mateDistance;

      FindLatitudeLongitude(Basic->Latitude,
                            Basic->Longitude,
                            mateBearing,
                            mateDistance,
                            &TeammateLatitude,
                            &TeammateLongitude);

      WayPointList[RESWP_TEAMMATE].Latitude   = TeammateLatitude;
      WayPointList[RESWP_TEAMMATE].Longitude  = TeammateLongitude;
      WayPointList[RESWP_TEAMMATE].Altitude   = Calculated->NavAltitude;

      if (mateDistance < 100 && InTeamSector==false)
        {
          InTeamSector=true;
          InputEvents::processGlideComputer(GCE_TEAM_POS_REACHED);
        }
      else if (mateDistance > 300)
        {
          InTeamSector = false;
        }
    }
  else
    {
      Calculated->TeammateBearing = 0;
      Calculated->TeammateRange = 0;
      WayPointList[RESWP_TEAMMATE].Latitude   = RESWP_INVALIDNUMBER;
      WayPointList[RESWP_TEAMMATE].Longitude  = RESWP_INVALIDNUMBER;
      WayPointList[RESWP_TEAMMATE].Altitude   = RESWP_INVALIDNUMBER;
    }

}

extern double CRUISE_EFFICIENCY;


static double EffectiveMacCready_internal(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
					  bool cruise_efficiency_mode) {

  if (Calculated->ValidFinish) return 0;
  if (ActiveTaskPoint<=0) return 0; // no e mc before start
  if (!Calculated->ValidStart) return 0;
  if (Calculated->TaskStartTime<0) return 0;

  if (!ValidTaskPoint(ActiveTaskPoint)
      || !ValidTaskPoint(ActiveTaskPoint-1)) return 0;
  if (Calculated->TaskDistanceToGo<=0) {
    return 0;
  }

  LockTaskData();

  double start_speed = Calculated->TaskStartSpeed;
  double V_bestld = GlidePolar::Vbestld();
  double energy_height_start =
    max(0.0, start_speed*start_speed-V_bestld*V_bestld)/(9.81*2.0);

  double telapsed = Basic->Time-Calculated->TaskStartTime;
  double height_below_start =
    Calculated->TaskStartAltitude + energy_height_start
    - Calculated->NavAltitude - Calculated->EnergyHeight;

  double LegDistances[MAXTASKPOINTS];
  double LegBearings[MAXTASKPOINTS];

  for (int i=0; i<ActiveTaskPoint; i++) {
    double w1lat = WayPointList[Task[i+1].Index].Latitude;
    double w1lon = WayPointList[Task[i+1].Index].Longitude;
    double w0lat = WayPointList[Task[i].Index].Latitude;
    double w0lon = WayPointList[Task[i].Index].Longitude;
    if (UseAATTarget()) {
      if (ValidTaskPoint(i+1)) {
        w1lat = Task[i+1].AATTargetLat;
        w1lon = Task[i+1].AATTargetLon;
      }
      if (i>0) {
        w0lat = Task[i].AATTargetLat;
        w0lon = Task[i].AATTargetLon;
      }
    }
    DistanceBearing(w0lat,
                    w0lon,
                    w1lat,
                    w1lon,
                    &LegDistances[i], &LegBearings[i]);

    if (i==ActiveTaskPoint-1) {

      double leg_covered = ProjectedDistance(w0lon, w0lat,
                                             w1lon, w1lat,
                                             Basic->Longitude,
                                             Basic->Latitude,
                                             NULL,NULL);
      LKASSERT(i>=0);
      if (i>=0) // UNMANAGED
      LegDistances[i] = leg_covered;
    }
    if ((StartLine==0) && (i==0)) {
      // Correct speed calculations for radius
      // JMW TODO accuracy: leg distance replace this with more accurate version
      // leg_distance -= StartRadius;
      LegDistances[0] = max(0.1,LegDistances[0]-StartRadius);
    }
  }

  // OK, distance/bearings calculated, now search for Mc

  double value_found;
  if (cruise_efficiency_mode) {
    value_found = 1.5; // max
  } else {
    value_found = 10.0; // max
  }

  for (double value_scan=0.01; value_scan<1.0; value_scan+= 0.01) {

    double height_remaining = height_below_start;
    double time_total=0;

    double mc_effective;
    double cruise_efficiency;

    if (cruise_efficiency_mode) {
      mc_effective = MACCREADY;
      if (Calculated->FinalGlide && (Calculated->timeCircling>0)) {
	mc_effective = CALCULATED_INFO.TotalHeightClimb
	  /CALCULATED_INFO.timeCircling;
      }
      cruise_efficiency = 0.5+value_scan;
    } else {
      mc_effective = value_scan*10.0;
      cruise_efficiency = 1.0;
    }

    // Now add times from start to this waypoint,
    // allowing for final glide where possible if aircraft height is below
    // start

    for(int i=ActiveTaskPoint-1;i>=0; i--) {

      LKASSERT(i>=0);
      if (i<0) break; // UNMANAGED

      double time_this;

      double height_used_this =
        GlidePolar::MacCreadyAltitude(mc_effective,
                                      LegDistances[i],
                                      LegBearings[i],
                                      Calculated->WindSpeed,
                                      Calculated->WindBearing,
                                      0, NULL,
                                      (height_remaining>0),
                                      &time_this,
                                      height_remaining,
				      cruise_efficiency);

      height_remaining -= height_used_this;

      if (time_this>=0) {
        time_total += time_this;
      } else {
        // invalid! break out of loop early
        time_total= time_this;
        i= -1;
        continue;
      }
    }

    if (time_total<0) {
      // invalid
      continue;
    }
    if (time_total>telapsed) {
      // already too slow
      continue;
    }

    // add time for climb from start height to height above start
    if (height_below_start<0) {

      BUGSTOP_LKASSERT(mc_effective!=0);

      if (mc_effective==0) mc_effective=0.1;
      time_total -= height_below_start/mc_effective;
    }
    // now check time..
    if (time_total<telapsed) {
      if (cruise_efficiency_mode) {
	value_found = cruise_efficiency;
      } else {
	value_found = mc_effective;
      }
      break;
    }

  }

  UnlockTaskData();

  return value_found;
}


double EffectiveCruiseEfficiency(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  double value = EffectiveMacCready_internal(Basic, Calculated, true);
  if (value<0.75) {
    return 0.75;
  }
  return value;
}


double EffectiveMacCready(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  return EffectiveMacCready_internal(Basic, Calculated, false);
}
