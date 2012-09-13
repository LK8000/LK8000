/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RasterTerrain.h"
#include "McReady.h"


// no need to use LegToGo and LegBearing, we use the active waypoint instead
// calculate also arrival altitude on obstacle

void CheckGlideThroughTerrain(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  if (ValidNotResWayPoint(TASKINDEX)) { 
	double lat, lon;
	double farlat, farlon;
	double oldfarlat, oldfarlon, oldfardist;
	static double oldfarbearing=361;
	static double oldstartaltitude=-1;
	double startaltitude;
	double distance_soarable;
	bool out_of_range, farout_of_range;
	double fardistance_soarable;
	double minaltitude, maxaltitude;
	double newaltitude;
	int selwp;

	selwp=TASKINDEX;
	if ( WayPointCalc[selwp].AltArriv[AltArrivMode]<0 ) {
		return;
	}

	distance_soarable = 
	#ifdef GTL2
		FinalGlideThroughTerrain(Calculated->WaypointBearing, Basic->Latitude,
                Basic->Longitude, Calculated->NavAltitude, Calculated, &lat, &lon, 
	#else
		FinalGlideThroughTerrain(Calculated->WaypointBearing, Basic, Calculated, &lat, &lon, 
	#endif
		Calculated->WaypointDistance, &out_of_range, NULL);

	// Calculate obstacles ONLY if we are in glide range, otherwise it is useless 
	if ((!out_of_range)&&(distance_soarable< Calculated->WaypointDistance)) {

		Calculated->TerrainWarningLatitude = lat;
		Calculated->TerrainWarningLongitude = lon;

		Calculated->ObstacleDistance = distance_soarable;

		Calculated->ObstacleHeight =  max((short)0, RasterTerrain::GetTerrainHeight(lat,lon));
		if (Calculated->ObstacleHeight == TERRAIN_INVALID) Calculated->ObstacleHeight=0; //@ 101027 FIX

		// how much height I will loose to get there
		Calculated->ObstacleAltReqd = GlidePolar::MacCreadyAltitude (MACCREADY, 
			distance_soarable, 
			Calculated->WaypointBearing,
			Calculated->WindSpeed, Calculated->WindBearing,
			0, 0, true,0);

		// arrival altitude over the obstacle
		// sometimes it is positive
		Calculated->ObstacleAltArriv = Calculated->NavAltitude
			 - Calculated->ObstacleAltReqd
			 - Calculated->ObstacleHeight
			 - SAFETYALTITUDETERRAIN;

		// Reminder: we already have a glide range on destination.
		minaltitude=Calculated->NavAltitude;
		maxaltitude=minaltitude*2;

		// if no far obstacle will be found, we shall use the first obstacle. 
		oldfarlat=lat;
		oldfarlon=lon;
		oldfardist=distance_soarable;
		if (oldstartaltitude<0) oldstartaltitude=minaltitude;

		// if bearing has changed for more than 1 deg, we dont use shortcuts
		if (fabs(oldfarbearing-Calculated->WaypointBearing) >= 1)  {
			startaltitude=minaltitude;
			oldfarbearing=Calculated->WaypointBearing;
		} else {
			startaltitude=oldstartaltitude-200;
			if (startaltitude <minaltitude) startaltitude=minaltitude;
		}

		// need to recalculate, init with first obstacle, forget old far obstacle
		// new bearing reference

		for ( newaltitude=minaltitude; newaltitude<maxaltitude; newaltitude+=50) {

			fardistance_soarable = FarFinalGlideThroughTerrain( Calculated->WaypointBearing, Basic, Calculated, 
				&farlat, &farlon, Calculated->WaypointDistance, &farout_of_range, newaltitude, NULL);

			if (fardistance_soarable< Calculated->WaypointDistance) {
				oldfarlat=farlat;
				oldfarlon=farlon;
				oldfardist=fardistance_soarable;
			} else break;
		}

		oldstartaltitude=newaltitude;
		Calculated->FarObstacle_Lat = oldfarlat;
		Calculated->FarObstacle_Lon = oldfarlon;
		Calculated->FarObstacle_Dist = oldfardist;
		// 0-50m positive rounding
		Calculated->FarObstacle_AltArriv = minaltitude - newaltitude;


	} else {
		Calculated->TerrainWarningLatitude = 0.0;
		Calculated->TerrainWarningLongitude = 0.0;
	}
  } else {
	Calculated->TerrainWarningLatitude = 0.0;
	Calculated->TerrainWarningLongitude = 0.0;
  }
}

