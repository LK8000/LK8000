/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "DoInits.h"
#include "Logger.h"
#include "Waypointparser.h"


void DoAutoQNH(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  static int done_autoqnh = 0;

  if (DoInit[MDI_DOAUTOQNH]) {
	done_autoqnh=0;
	DoInit[MDI_DOAUTOQNH]=false;
  }

  // Reject if already done
  if (done_autoqnh==10) return;

  // Reject if in IGC logger mode
  if (ReplayLogger::IsEnabled()) return;

  // Reject if no valid GPS fix
  if (Basic->NAVWarning) return;

  // Reject if no baro altitude
  if (!Basic->BaroAltitudeAvailable) return;

  // Reject if terrain height is invalid
  if (!Calculated->TerrainValid) return;

  // Once set, even at standard pressure, this QNH will be different from 1013.25 which is a rounded up double
  // So we are saying: for the case we did not set QNH yet, then continue.
  if (QNH != PRESSURE_STANDARD) return;

  if (Basic->Speed<TakeOffSpeedThreshold) {
    done_autoqnh++;
  } else {
    done_autoqnh= 0; // restart...
  }

  if (done_autoqnh==10) {
	double fixaltitude = Calculated->TerrainAlt;

	// if we have a valid fix, and a valid home waypoint, then if we are close to it assume we are at home
	// and use known altitude, instead of presumed terrain altitude which is always approximated
	double hdist=0;
	if (ValidWayPoint(HomeWaypoint)) {
		DistanceBearing(Basic->Latitude, Basic->Longitude, 
			WayPointList[HomeWaypoint].Latitude, WayPointList[HomeWaypoint].Longitude,&hdist,NULL);

		if (hdist <2000) {
			fixaltitude=WayPointList[HomeWaypoint].Altitude;
			StartupStore(_T(". AutoQNH: setting QNH to HOME waypoint <%s> altitude=%.0f m%s"),WayPointList[HomeWaypoint].Name, fixaltitude,NEWLINE);
		} else {
			if (fixaltitude!=0)
				StartupStore(_T(". AutoQNH: setting QNH to average terrain altitude=%.0f m%s"),fixaltitude,NEWLINE);
			else
				StartupStore(_T(". AutoQNH: cannot set QNH, impossible terrain altitude%s"),NEWLINE);
		}
	} else {
		// 101121 extend search for nearest wp
		int i=FindNearestWayPoint(Basic->Longitude, Basic->Latitude, 2000);
		if ( (i>RESWP_END) && (WayPointList[i].Altitude>0) ) {  // avoid using TAKEOFF wp
			fixaltitude=WayPointList[i].Altitude;
			#if TESTBENCH
			StartupStore(_T(". AutoQNH: setting QNH to nearest <%s> waypoint altitude=%.0f m%s"),
				WayPointList[i].Name,fixaltitude,NEWLINE);
			#endif
		} else {
			#if TESTBENCH
			if (fixaltitude!=0)
				StartupStore(_T(". AutoQNH: setting QNH to average terrain altitude=%.0f m%s"),fixaltitude,NEWLINE);
			else
				StartupStore(_T(". AutoQNH: cannot set QNH, impossible terrain altitude%s"),NEWLINE);
			#endif
		}
	}
	if (fixaltitude!=0) {
		QNH = FindQNH(Basic->BaroAltitude, fixaltitude);
		TCHAR qmes[80];
		if (PressureHg) 
			_stprintf(qmes,_T("QNH set to %.2f, Altitude %.0f%s"),QNH/TOHPA,fixaltitude*ALTITUDEMODIFY,
			Units::GetUnitName(Units::GetUserAltitudeUnit()));
		else
			_stprintf(qmes,_T("QNH set to %.2f, Altitude %.0f%s"),QNH,fixaltitude*ALTITUDEMODIFY,
			Units::GetUnitName(Units::GetUserAltitudeUnit()));
		DoStatusMessage(qmes);
		#if TESTBENCH
		StartupStore(_T("%s%s"),qmes,NEWLINE);
		#endif
		CAirspaceManager::Instance().QnhChangeNotify(QNH);
	}
  }
}


