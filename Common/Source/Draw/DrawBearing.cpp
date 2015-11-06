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
    if (!ValidTaskPoint(ActiveTaskPoint)) {
      return; 
    }
    LockTaskData();

    if (AATEnabled && ( DoOptimizeRoute() || ((ActiveTaskPoint>0) && ValidTaskPoint(ActiveTaskPoint+1))) ) {
      targetLat = Task[ActiveTaskPoint].AATTargetLat;
      targetLon = Task[ActiveTaskPoint].AATTargetLon; 
    } else {
      targetLat = WayPointList[Task[ActiveTaskPoint].Index].Latitude;
      targetLon = WayPointList[Task[ActiveTaskPoint].Index].Longitude; 
    }
    UnlockTaskData();

    DrawGreatCircle(Surface, startLon, startLat, targetLon, targetLat, rc);

    if (mode.Is(Mode::MODE_TARGET_PAN)) {
      // Draw all of task if in target pan mode
      startLat = targetLat;
      startLon = targetLon;

      LockTaskData();
      for (int i=ActiveTaskPoint+1; i<MAXTASKPOINTS; i++) {
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
      for(int i=ActiveTaskPoint+1; i<MAXTASKPOINTS; i++) {
        if(ValidTaskPoint(i) && ValidTaskPoint(i+1)) {
          if(i>= ActiveTaskPoint) {
            POINT sct;
            LatLon2Screen(Task[i].AATTargetLon, 
                          Task[i].AATTargetLat, 
                          sct);
            DrawBitmapIn(Surface, sct, hBmpTarget,true);
          }
        }
      }
    }
    if(ValidTaskPoint(ActiveTaskPoint+1) && (DoOptimizeRoute() || (ActiveTaskPoint>0)) ) {
      POINT sct;
      LatLon2Screen(Task[ActiveTaskPoint].AATTargetLon, 
                    Task[ActiveTaskPoint].AATTargetLat, 
                    sct);
      DrawBitmapIn(Surface, sct, hBmpTarget,true);
    }
    UnlockTaskData();
  }
}

