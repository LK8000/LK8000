/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Draw/Task/TaskRendererMgr.h"
#include "Geographic/GeoPoint.h"



void CalculateTaskSectors(int Idx) {

    double SectorBearing = .0;

    const TASK_POINT &TaskPt = Task[Idx];
    if (ValidWayPointFast(TaskPt.Index)) {
        const WAYPOINT &TaskWpt = WayPointList[TaskPt.Index];
        const GeoPoint center(TaskWpt.Latitude, TaskWpt.Longitude);

        if (Idx == 0) {
            SectorBearing = TaskPt.OutBound + 180;

            // start turnpoint sector
            switch (StartLine) {
                case sector_type_t::CIRCLE:
                    gTaskSectorRenderer.SetCircle(Idx, center, StartRadius);
                    break;
                case sector_type_t::LINE:
                    gTaskSectorRenderer.SetLine(Idx, center, StartRadius, SectorBearing);
                    break;
                case sector_type_t::SECTOR:
                    gTaskSectorRenderer.SetStartSector(Idx, center, StartRadius,
                                                       SectorBearing - 45,
                                                       SectorBearing + 45);
                    break;
                default:
                    assert(false);
                    break;
            }
        } else if (ValidTaskPointFast(Idx + 1)) {
            if (gTaskType == TSK_DEFAULT) {
                // normal turnpoint sector
                SectorBearing = TaskPt.Bisector;

                switch (SectorType) {
                    case sector_type_t::CIRCLE:
                        gTaskSectorRenderer.SetCircle(Idx, center, SectorRadius);
                        break;
                    case sector_type_t::SECTOR:
                        gTaskSectorRenderer.SetSector(Idx, center, SectorRadius,
                                                      SectorBearing - 45,
                                                      SectorBearing + 45);
                        break;
                    case sector_type_t::DAe:
                        gTaskSectorRenderer.SetDae(Idx, center,
                                                   SectorBearing - 45,
                                                   SectorBearing + 45);
                        break;
                    case sector_type_t::LINE:
                        gTaskSectorRenderer.SetLine(Idx, center, SectorRadius, SectorBearing + 90);
                        break;
                    default:
                        assert(false);
                        break;
                }
            } else {
                switch (TaskPt.AATType) {
                    case sector_type_t::CIRCLE:
                    case sector_type_t::CONE:
                    case sector_type_t::ESS_CIRCLE:
                        gTaskSectorRenderer.SetCircle(Idx, center, TaskPt.AATCircleRadius);
                        break;
                    case sector_type_t::SECTOR:
                        gTaskSectorRenderer.SetSector(Idx, center, TaskPt.AATSectorRadius,
                                                      TaskPt.AATStartRadial,
                                                      TaskPt.AATFinishRadial);
                        break;
                    default:
                        assert(false);
                        break;
                }
            }
        } else {
            SectorBearing = TaskPt.InBound;

            switch (FinishLine) {
                case sector_type_t::CIRCLE:
                    gTaskSectorRenderer.SetCircle(Idx, center, FinishRadius);
                    break;
                case sector_type_t::LINE:
                    gTaskSectorRenderer.SetLine(Idx, center, FinishRadius, SectorBearing);
                    break;
                case sector_type_t::SECTOR:
                    gTaskSectorRenderer.SetStartSector(Idx, center, FinishRadius,
                                                       SectorBearing - 45,
                                                       SectorBearing + 45);
                    break;
                default:
                    assert(false);
                    break;
            }
        }
        if (!UseAATTarget()) {
            /** this initialise AAT sector
             * maybe bad idea, because if you disable/enable AAT that override previous Values ...
             */
            Task[Idx].AATStartRadial = AngleLimit360(SectorBearing - 45);
            Task[Idx].AATFinishRadial = AngleLimit360(SectorBearing + 45);
        }
    }
}

void CalculateTaskSectors(void) {

    LockTaskData();

    if (EnableMultipleStartPoints) {
        for (int i = 0; i < MAXSTARTPOINTS - 1; i++) {
            const START_POINT &StartPt = StartPoints[i];
            if (StartPt.Active && ValidWayPoint(StartPt.Index)) {
                const WAYPOINT &StartWpt = WayPointList[StartPt.Index];
                const GeoPoint center(StartWpt.Latitude, StartWpt.Longitude);
                switch (StartLine) {
                case sector_type_t::SECTOR:
                    gStartSectorRenderer.SetStartSector(i, center, StartRadius,
                                                        StartPt.OutBound + 45,
                                                        StartPt.OutBound - 45);
                    break;                
                case sector_type_t::LINE:
                    gStartSectorRenderer.SetLine(i, center, StartRadius, StartPt.OutBound);
                    break;
                default:
                    assert(false);
                    break;
                }
            }
        }
    }

    for (int i = 0; i < MAXTASKPOINTS; i++) {
        CalculateTaskSectors(i);
    }
    UnlockTaskData();
}
