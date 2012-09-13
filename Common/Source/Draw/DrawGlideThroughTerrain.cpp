/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Bitmaps.h"
#include <string.h>
#ifdef GTL2
#include "RGB.h"

extern void ClearGTL2(void);
#endif

//
// Glide through terrain will paint a cross over the first and last obstacle to
// the destination.
// It will also draw the perimeter, even if a shaded area was painted previously
//

void MapWindow::DrawGlideThroughTerrain(HDC hDC, const RECT rc) {
  HPEN hpOld;

  //double h,dh;
  TCHAR hbuf[10];
  static bool doinit=true;
  static TextInBoxMode_t tmode = {0};
  bool wrotevalue=false;

  if (doinit) {
    memset((void*)&tmode, 0, sizeof(TextInBoxMode_t));
    tmode.Border=1;
    doinit=false;
  }

  #ifdef GTL2
  bool ValidTP = ValidTaskPoint(ActiveWayPoint);

  // draw glide terrain line around next WP
  bool DrawGTL2 = ValidTP && (FinalGlideTerrain > 2);
  static bool LastDrewGTL2 = false;

  if (DrawGTL2) {
    int wp_index = (DoOptimizeRoute() || ACTIVE_WP_IS_AAT_AREA) ? 
                   RESWP_OPTIMIZED : TASKINDEX;

    double alt_arriv = WayPointCalc[wp_index].AltArriv[AltArrivMode];
    
    // Calculate arrival altitude at the next waypoint relative to
    // the "terrain height" safety setting.

    if (CheckSafetyAltitudeApplies(wp_index))
      alt_arriv += SAFETYALTITUDEARRIVAL; // AGL
    alt_arriv -= SAFETYALTITUDETERRAIN;   // rel. to "terrain height"

    if (alt_arriv <= 0) DrawGTL2 = false;
  }
  
  if (LastDrewGTL2 != DrawGTL2) {
    LastDrewGTL2 = DrawGTL2;
    if (!DrawGTL2) ClearGTL2(); // clear next-WP glide terrain line
  }
  #endif

  hpOld = (HPEN)SelectObject(hDC, hpTerrainLineBg); 

  #ifdef GTL2
  // Draw the wide, solid part of the glide terrain line.
  #else
  // draw a dashed perimetral line first
  #endif
  _Polyline(hDC,Groundline,NUMTERRAINSWEEPS+1, rc);

  // draw perimeter if selected and during a flight
  #ifdef GTL2
  if (((FinalGlideTerrain == 1) || (FinalGlideTerrain == 3)) || 
     ((!EnableTerrain || !DerivedDrawInfo.Flying) && FinalGlideTerrain)) { 
  #else
  if ((FinalGlideTerrain==1) || ((!EnableTerrain || !DerivedDrawInfo.Flying) && (FinalGlideTerrain==2))) { 
  #endif
	SelectObject(hDC,hpTerrainLine);
	_Polyline(hDC,Groundline,NUMTERRAINSWEEPS+1, rc);
  }
  
  #ifdef GTL2  
  // draw glide terrain line around next waypoint
  if (DrawGTL2) {
    // Draw a solid white line.
    SelectObject(hDC, hpTerrainLine2Bg);
    _Polyline(hDC, Groundline2, NUMTERRAINSWEEPS+1, rc);

    // Draw a dashed red line.
    DrawDashPoly(hDC, NIBLSCALE(2), RGB_RED, Groundline2,
                 NUMTERRAINSWEEPS+1, rc);
  }
  #endif

  // draw red cross obstacles only if destination looks reachable!
  // only if using OVT_TASK of course!

  #ifdef GTL2
  if ((OvertargetMode == OVT_TASK) && DerivedDrawInfo.Flying && ValidTP)
  #else
  if ( (OvertargetMode==OVT_TASK) && DerivedDrawInfo.Flying && ValidTaskPoint(ActiveWayPoint))
  #endif
  if (WayPointCalc[TASKINDEX].AltArriv[AltArrivMode] >0) { 

	POINT sc;
	// If calculations detected an obstacle...
	if ((DerivedDrawInfo.TerrainWarningLatitude != 0.0) &&(DerivedDrawInfo.TerrainWarningLongitude != 0.0)) {

		// only if valid position, and visible
		if (DerivedDrawInfo.FarObstacle_Lon >0) 
		if (PointVisible(DerivedDrawInfo.FarObstacle_Lon, DerivedDrawInfo.FarObstacle_Lat)) {
			LatLon2Screen(DerivedDrawInfo.FarObstacle_Lon, DerivedDrawInfo.FarObstacle_Lat, sc);
			DrawBitmapIn(hDC, sc, hTerrainWarning,true);

			if (DerivedDrawInfo.FarObstacle_AltArriv <=-50 ||  DerivedDrawInfo.FarObstacle_Dist<5000 ) {
				_stprintf(hbuf,_T(" %.0f"),ALTITUDEMODIFY*DerivedDrawInfo.FarObstacle_AltArriv);
				TextInBox(hDC,hbuf,sc.x+NIBLSCALE(15), sc.y, 0, &tmode,false); 
				wrotevalue=true;
			}
		} // visible far obstacle

		if (PointVisible(DerivedDrawInfo.TerrainWarningLongitude, DerivedDrawInfo.TerrainWarningLatitude)) {
			LatLon2Screen(DerivedDrawInfo.TerrainWarningLongitude, DerivedDrawInfo.TerrainWarningLatitude, sc);
			DrawBitmapIn(hDC, sc, hTerrainWarning,true);
#if 0
			// 091203 add obstacle altitude on moving map
			h =  max(0,RasterTerrain::GetTerrainHeight(DerivedDrawInfo.TerrainWarningLatitude, 
				DerivedDrawInfo.TerrainWarningLongitude)); 
			if (h==TERRAIN_INVALID) h=0; //@ 101027 FIX but unused
			dh = CALCULATED_INFO.NavAltitude - h - SAFETYALTITUDETERRAIN;
			_stprintf(hbuf,_T(" %.0f"),ALTITUDEMODIFY*dh);
			TextInBox(hDC,hbuf,sc.x+NIBLSCALE(10), sc.y, 0, tmode,false); 
#else
			// if far obstacle was painted with value...
			if (wrotevalue) {
				// if it is not too near the nearest..
				if ( (fabs(DerivedDrawInfo.FarObstacle_Lon - DerivedDrawInfo.TerrainWarningLongitude) >0.02) &&
					(fabs(DerivedDrawInfo.FarObstacle_Lat - DerivedDrawInfo.TerrainWarningLatitude) >0.02)) {
					// and it the arrival altitude is actually negative (rounding terrain errors?)
					if ( DerivedDrawInfo.ObstacleAltArriv <=-50)
					// and there is a significant difference in the numbers, then paint value also for nearest
					if (  fabs(DerivedDrawInfo.ObstacleAltArriv - DerivedDrawInfo.FarObstacle_AltArriv) >100 ) {
						_stprintf(hbuf,_T(" %.0f"),ALTITUDEMODIFY*DerivedDrawInfo.ObstacleAltArriv);
						TextInBox(hDC,hbuf,sc.x+NIBLSCALE(15), sc.y, 0, &tmode,false); 
					}
				}
			} else {
				// else paint value only if meaningful or very close to us
				// -1 to 10m become -1 for rounding errors
				if ( (DerivedDrawInfo.ObstacleAltArriv >-1) && (DerivedDrawInfo.ObstacleAltArriv <10))
					DerivedDrawInfo.ObstacleAltArriv=-1;
				if (DerivedDrawInfo.ObstacleAltArriv <=-50 ||  
				 ((DerivedDrawInfo.ObstacleAltArriv<0) && (DerivedDrawInfo.ObstacleDistance<5000)) ) {

					_stprintf(hbuf,_T(" %.0f"),ALTITUDEMODIFY*DerivedDrawInfo.ObstacleAltArriv);
					TextInBox(hDC,hbuf,sc.x+NIBLSCALE(15), sc.y, 0, &tmode,false); 
				}
			}
#endif
		} // visible nearest obstacle
	} // obstacles detected
  } // within glide range

  SelectObject(hDC, hpOld);
}

