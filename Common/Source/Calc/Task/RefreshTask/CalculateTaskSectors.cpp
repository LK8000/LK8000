/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Draw/Task/TaskRendererMgr.h"
#include "Geographic/GeoPoint.h"

void CalculateTaskSectors(void) {
	double SectorBearing = 0.;

  LockTaskData();

  if (EnableMultipleStartPoints) {
		for (int i = 0; i < MAXSTARTPOINTS - 1; i++) {
			const START_POINT& StartPt = StartPoints[i];
			if (StartPt.Active && ValidWayPoint(StartPt.Index)) {
				const WAYPOINT& StartWpt = WayPointList[StartPt.Index];
				const GeoPoint center(StartWpt.Latitude, StartWpt.Longitude);

	if (StartLine==2) {
					gStartSectorRenderer.SetStartSector(i, center, StartRadius, StartPt.OutBound + 45, StartPt.OutBound - 45);
        } else {
					gStartSectorRenderer.SetLine(i, center, StartRadius, StartPt.OutBound);
        }
      }
    }
  }

	for (int i = 0; i <= MAXTASKPOINTS - 1; i++) {
		const TASK_POINT& TaskPt = Task[i];
		if (ValidWayPoint(TaskPt.Index)) {
			const WAYPOINT& TaskWpt = WayPointList[TaskPt.Index];
			const GeoPoint center(TaskWpt.Latitude, TaskWpt.Longitude);

			if (i == 0) {
				SectorBearing = TaskPt.OutBound + 180;

				// start turnpoint sector
				switch (StartLine) {
					case 0:
						gTaskSectorRenderer.SetCircle(i, center, StartRadius);
						break;
					case 1:
						gTaskSectorRenderer.SetLine(i, center, StartRadius, SectorBearing);
						break;
					case 2:
						gTaskSectorRenderer.SetStartSector(i, center, StartRadius, SectorBearing - 45, SectorBearing + 45);
						break;
					default:
						assert(false);
	      }
			} else if (ValidTaskPointFast(i + 1)) {
				if(!AATEnabled && !DoOptimizeRoute()) {
		// normal turnpoint sector
					SectorBearing = TaskPt.Bisector;

					switch (SectorType) {
						case CIRCLE:
							gTaskSectorRenderer.SetCircle(i, center, SectorRadius);
							break;
						case SECTOR:
							gTaskSectorRenderer.SetSector(i, center, SectorRadius, SectorBearing - 45, SectorBearing + 45);
							break;
						case DAe:
							gTaskSectorRenderer.SetDae(i, center, SectorBearing - 45, SectorBearing + 45);
							break;
						case LINE:
							gTaskSectorRenderer.SetLine(i, center, SectorRadius, SectorBearing + 90);
							break;
						default:
							assert(false);
                }
		} else {
					switch(TaskPt.AATType) {
						case CIRCLE : // CIRCLE
						case 2 : // CONE
						case 3 : // ESS_CIRCLE
							gTaskSectorRenderer.SetCircle(i, center, TaskPt.AATCircleRadius);
							break;
						case SECTOR : // SECTOR
							gTaskSectorRenderer.SetSector(i, center, TaskPt.AATSectorRadius, TaskPt.AATStartRadial, TaskPt.AATFinishRadial);
							break;
						default:
							assert(false);
                }
	      }
	  } else {
				SectorBearing = TaskPt.InBound;

				switch (FinishLine) {
					case 0:
						gTaskSectorRenderer.SetCircle(i, center, FinishRadius);
						break;
					case 1:
						gTaskSectorRenderer.SetLine(i, center, FinishRadius, SectorBearing);
						break;
					case 2:
						gTaskSectorRenderer.SetStartSector(i, center, FinishRadius, SectorBearing - 45, SectorBearing + 45);
						break;
					default:
						assert(false);
				}
			}
			if (!AATEnabled) {
				/** this initialise AAT sector
				 * maybe bad idea, because if you disable/enable AAT that override previous Values ...
				 */
				Task[i].AATStartRadial = AngleLimit360(SectorBearing - 45);
				Task[i].AATFinishRadial = AngleLimit360(SectorBearing + 45);
          }
	}
    }
  UnlockTaskData();
}
