/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKInterface.h"
#include "Terrain.h"
#include "Bitmaps.h"



void MapWindow::DrawBearing(LKSurface& Surface, const RECT& rc)
{
  int overindex=GetOvertargetIndex();
  if (overindex<0) return;

  double startLat = DrawInfo.Latitude;
  double startLon = DrawInfo.Longitude;
  double targetLat;
  double targetLon;

  if (OvertargetMode>OVT_TASK) {
    LockTaskData();
    targetLat = WayPointList[overindex].Latitude;
    targetLon = WayPointList[overindex].Longitude;
    UnlockTaskData();
    DrawGreatCircle(Surface, startLon, startLat, targetLon, targetLat, rc);
  }
  else {
    if (!ValidTaskPoint(ActiveWayPoint)) {
      return; 
    }
    LockTaskData();

    if (AATEnabled && ( DoOptimizeRoute() || ((ActiveWayPoint>0) && ValidTaskPoint(ActiveWayPoint+1))) ) {
      targetLat = Task[ActiveWayPoint].AATTargetLat;
      targetLon = Task[ActiveWayPoint].AATTargetLon; 
    } else {
      targetLat = WayPointList[Task[ActiveWayPoint].Index].Latitude;
      targetLon = WayPointList[Task[ActiveWayPoint].Index].Longitude; 
    }
    UnlockTaskData();

    DrawGreatCircle(Surface, startLon, startLat, targetLon, targetLat, rc);

    if (mode.Is(Mode::MODE_TARGET_PAN)) {
      // Draw all of task if in target pan mode
      startLat = targetLat;
      startLon = targetLon;

      LockTaskData();
      for (int i=ActiveWayPoint+1; i<MAXTASKPOINTS; i++) {
        if (ValidTaskPoint(i)) {

          if (AATEnabled && ValidTaskPoint(i+1)) {
            targetLat = Task[i].AATTargetLat;
            targetLon = Task[i].AATTargetLon; 
          } else {
            targetLat = WayPointList[Task[i].Index].Latitude;
            targetLon = WayPointList[Task[i].Index].Longitude; 
          }
       
          DrawGreatCircle(Surface, startLon, startLat,
                          targetLon, targetLat, rc);

          startLat = targetLat;
          startLon = targetLon;
        }
      }
      UnlockTaskData();
    }
  }

  if (AATEnabled) {
    // draw symbol at target, makes it easier to see
    LockTaskData();
    if(mode.Is(Mode::MODE_TARGET_PAN)) {
      for(int i=ActiveWayPoint+1; i<MAXTASKPOINTS; i++) {
        if(ValidTaskPoint(i) && ValidTaskPoint(i+1)) {
          if(i>= ActiveWayPoint) {
            POINT sct;
            LatLon2Screen(Task[i].AATTargetLon, 
                          Task[i].AATTargetLat, 
                          sct);
            DrawBitmapIn(Surface, sct, hBmpTarget,true);
          }
        }
      }
    }
    if(ValidTaskPoint(ActiveWayPoint+1) && (DoOptimizeRoute() || (ActiveWayPoint>0)) ) {
      POINT sct;
      LatLon2Screen(Task[ActiveWayPoint].AATTargetLon, 
                    Task[ActiveWayPoint].AATTargetLat, 
                    sct);
      DrawBitmapIn(Surface, sct, hBmpTarget,true);
    }
    UnlockTaskData();
  }
}

