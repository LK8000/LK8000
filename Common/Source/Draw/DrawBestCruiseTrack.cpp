/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKObjects.h"


void MapWindow::DrawBestCruiseTrack(LKSurface& Surface, const POINT& Orig)
{
  if (OvertargetMode>OVT_TASK) return;

  if (!ValidTaskPoint(ActiveTaskPoint)) return;

  if (DerivedDrawInfo.WaypointDistance < 0.010) return;

  // dont draw bestcruise indicator if not needed
  if (fabs(DerivedDrawInfo.BestCruiseTrack-DerivedDrawInfo.WaypointBearing)<2) { // 091202 10 to 2
	return;
  } 


  const auto hpOld = Surface.SelectObject(LKPen_Blue_N1);
  const auto hbOld = Surface.SelectObject(LKBrush_Blue);

  if (Appearance.BestCruiseTrack == ctBestCruiseTrackDefault){

    int dy = (long)(70); 
    POINT Arrow[7] = { {-1,-40}, {1,-40}, {1,0}, {6,8}, {-6,8}, {-1,0}, {-1,-40}};

    Arrow[2].y -= dy;
    Arrow[3].y -= dy;
    Arrow[4].y -= dy;
    Arrow[5].y -= dy;

    PolygonRotateShift(Arrow, 7, Orig.x, Orig.y, 
                       DerivedDrawInfo.BestCruiseTrack-DisplayAngle);

    Surface.Polygon(Arrow,7);

  } else
  if (Appearance.BestCruiseTrack == ctBestCruiseTrackAltA){

    POINT Arrow[] = { {-1,-40}, {-1,-62}, {-6,-62}, {0,-70}, {6,-62}, {1,-62}, {1,-40}, {-1,-40}};

    PolygonRotateShift(Arrow, sizeof(Arrow)/sizeof(Arrow[0]),
                       Orig.x, Orig.y, 
                       DerivedDrawInfo.BestCruiseTrack-DisplayAngle);
    Surface.Polygon(Arrow, (sizeof(Arrow)/sizeof(Arrow[0])));
  }

  Surface.SelectObject(hpOld);
  Surface.SelectObject(hbOld);
}


