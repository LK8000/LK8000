/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "AATDistance.h"
#include "Waypointparser.h"
#include "NavFunctions.h"


extern AATDistance aatdistance;




void CalculateOptimizedTargetPos(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

	if (!DoOptimizeRoute()) 
		return;

	int curwp; // cur for current pos
	int stdwp; // std for current standard wp
	int nxtwp; // nxt for next after current wp

	double curlat, curlon;
	double stdlat, stdlon;
	double stddst, stdbrg;
	double nxtlat, nxtlon;
	double nxtbrg;

	curwp  = ActiveWayPoint;
	curlat = Basic->Latitude; 
	curlon = Basic->Longitude; 

	bool bCalcPrev = false;

	while(ValidWayPoint(nxtwp=Task[curwp+1].Index)) {
		
		stdwp=Task[curwp].Index;

		stdlat = WayPointList[stdwp].Latitude;
		stdlon = WayPointList[stdwp].Longitude;

		nxtlat = WayPointList[nxtwp].Latitude;
		nxtlon = WayPointList[nxtwp].Longitude;

		double radius= (curwp>0)?(Task[curwp].AATCircleRadius):StartRadius;
		DistanceBearing(curlat, curlon, stdlat, stdlon, &stddst, &stdbrg);

		// if Same Wpt Calc Next before if Exist
		if(stdwp == nxtwp && stddst > radius && ValidWayPoint(Task[curwp+2].Index)){  
			bCalcPrev = true;
			Task[curwp].AATTargetLat= stdlat;
			Task[curwp].AATTargetLon= stdlon;
			Task[curwp].AATTargetLocked=true;
			++curwp;
			continue;
		}

		double optlat, optlon;
		// From Current Position To Current Wpt
		double obrg_f = stdbrg;

		if (stdwp == nxtwp && stddst < radius) {
			double radius= (curwp>0)?(Task[curwp].AATCircleRadius):StartRadius;
			double obrg_f = Reciprocal(stdbrg);

			FindLatitudeLongitude(stdlat,stdlon, obrg_f, radius, &optlat, &optlon);
		}
		else {
			// From Current Wpt To Next Wpt
			DistanceBearing(stdlat, stdlon, nxtlat, nxtlon, NULL, &nxtbrg);
			double radius= (curwp>0)?(Task[curwp].AATCircleRadius):StartRadius;

			double obrg_f = nxtbrg;

			if(radius < stddst || bCalcPrev) {
				double inlat, inlon;
				FindLatitudeLongitude(stdlat,stdlon, Reciprocal(stdbrg), radius, &inlat, &inlon);
			
				double outlat, outlon;
				FindLatitudeLongitude(stdlat,stdlon, nxtbrg, radius, &outlat, &outlon);

				Coor intersec(0,0);
				CalcIntersection(Coor(curlat,curlon), Coor(outlat, outlon), Coor(nxtlat,nxtlon), Coor(inlat,inlon), intersec);

				DistanceBearing(stdlat, stdlon, intersec.lat, intersec.lon, NULL, &obrg_f);
			}

			FindLatitudeLongitude(stdlat,stdlon, obrg_f, radius, &optlat, &optlon);

			double errbrg, optbrg, errdst; // beraing from current to next
			DistanceBearing(curlat, curlon, nxtlat, nxtlon, &errdst, &errbrg);
			if(errdst > stddst ) {
				DistanceBearing(curlat, curlon, optlat, optlon, NULL, &optbrg);

				double dBrg = fabs((stdbrg - errbrg) * DEG_TO_RAD);
				if(dBrg > PI) 
					dBrg = fabs((stdbrg-errbrg+360) * DEG_TO_RAD);

				if(radius > stddst || (dBrg < PI/2 && sin(dBrg) < radius/stddst)) {
					if( (dBrg < PI/2) && (radius < stddst) && (PGStartOut || curwp>0)) {
						dBrg = - dBrg + asin((stddst * sin(dBrg)) / radius);
					}
					else{
						dBrg = PI - dBrg - asin((stddst * sin(dBrg)) / radius);
					}
					dBrg *= RAD_TO_DEG * (((stdbrg-errbrg)>-180 && (stdbrg-errbrg)<0)?-1:1);
					obrg_f = AngleLimit360(dBrg + 180 + stdbrg);

					FindLatitudeLongitude(stdlat,stdlon, obrg_f, radius, &optlat, &optlon);
				}
			}
		}
		Task[curwp].AATTargetLat= optlat;
		Task[curwp].AATTargetLon= optlon;
		Task[curwp].AATTargetLocked=true;

		if(bCalcPrev) {
			double errbrg; // beraing from current to optNext
			DistanceBearing(curlat, curlon, optlat, optlon, NULL, &errbrg);
			radius= ((curwp-1)>0)?(Task[curwp-1].AATCircleRadius):StartRadius;
			double dBrg = fabs((stdbrg - errbrg) * DEG_TO_RAD);
			if(dBrg > PI) 
				dBrg = fabs((stdbrg-errbrg+360) * DEG_TO_RAD);
			if( (dBrg > (PI/360)) && (sin(dBrg) < radius/stddst)) {
                          if( (dBrg < PI/2) || ((dBrg > 2*PI) && (radius < stddst))) {
					dBrg = - dBrg + asin((stddst * sin(dBrg)) / radius);
				}
				else{
					dBrg = PI - dBrg - asin((stddst * sin(dBrg)) / radius);
				}
				dBrg *= RAD_TO_DEG * (((stdbrg-errbrg)>-180 && (stdbrg-errbrg)<0)?-1:1);
				obrg_f = AngleLimit360(dBrg + 180 + stdbrg);
			}
			else {
				obrg_f = AngleLimit360(stdbrg + 180);
			}
			FindLatitudeLongitude(stdlat,stdlon, obrg_f, radius, &optlat, &optlon);


			Task[curwp-1].AATTargetLat= optlat;
			Task[curwp-1].AATTargetLon= optlon;
			Task[curwp-1].AATTargetLocked=true;

			bCalcPrev = false;
		}

		curlat = Task[curwp].AATTargetLat;
		curlon = Task[curwp].AATTargetLon;
		Task[curwp].AATTargetLocked=true;
		++curwp;
	}

	// Last radius
	stdwp=Task[curwp].Index;
	stdlat = WayPointList[stdwp].Latitude;
	stdlon = WayPointList[stdwp].Longitude;

	DistanceBearing(stdlat, stdlon, curlat, curlon, NULL, &stdbrg);
	FindLatitudeLongitude(stdlat,stdlon, stdbrg, FinishRadius, &(Task[curwp].AATTargetLat), &(Task[curwp].AATTargetLon));
	Task[curwp].AATTargetLocked=true;



	stdwp=Task[ActiveWayPoint].Index;

	WayPointList[RESWP_OPTIMIZED].Latitude = Task[ActiveWayPoint].AATTargetLat;
	WayPointList[RESWP_OPTIMIZED].Longitude = Task[ActiveWayPoint].AATTargetLon;
	WayPointList[RESWP_OPTIMIZED].Altitude = WayPointList[stdwp].Altitude;
	WaypointAltitudeFromTerrain(&WayPointList[RESWP_OPTIMIZED]);

	wsprintf(WayPointList[RESWP_OPTIMIZED].Name, _T("!%s"),WayPointList[stdwp].Name);
}

// Clear PG 
void ClearOptimizedTargetPos() {

	WayPointList[RESWP_OPTIMIZED].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_OPTIMIZED].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_OPTIMIZED].Altitude=RESWP_INVALIDNUMBER;
	// name will be assigned by function dynamically
	_tcscpy(WayPointList[RESWP_OPTIMIZED].Name, _T("OPTIMIZED") );

	for(int i = 0; ValidWayPoint(Task[i].Index); ++i) {
		Task[i].AATTargetLat = WayPointList[Task[i].Index].Latitude;
		Task[i].AATTargetLon = WayPointList[Task[i].Index].Longitude;
		Task[i].AATTargetLocked = false;
	}
}

