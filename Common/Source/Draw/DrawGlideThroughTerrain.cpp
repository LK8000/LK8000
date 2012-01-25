/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Bitmaps.h"


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
  static TextInBoxMode_t tmode;
  bool wrotevalue=false;

  if (doinit) {
	tmode.AsInt=0;
	tmode.AsFlag.Border=1;
	doinit=false;
  }

  hpOld = (HPEN)SelectObject(hDC, hpTerrainLineBg); 

  // draw a dashed perimetral line first
  _Polyline(hDC,Groundline,NUMTERRAINSWEEPS+1, rc);

  // draw perimeter if selected and during a flight
  if ((FinalGlideTerrain==1) || ((!EnableTerrain || !DerivedDrawInfo.Flying) && (FinalGlideTerrain==2))) { 
	SelectObject(hDC,hpTerrainLine);
	_Polyline(hDC,Groundline,NUMTERRAINSWEEPS+1, rc);
  }

  // draw red cross obstacles only if destination looks reachable!
  // only if using OVT_TASK of course!

  if ( (OvertargetMode==OVT_TASK) && DerivedDrawInfo.Flying && ValidTaskPoint(ActiveWayPoint))
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
				TextInBox(hDC,hbuf,sc.x+NIBLSCALE(15), sc.y, 0, tmode,false); 
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
						TextInBox(hDC,hbuf,sc.x+NIBLSCALE(15), sc.y, 0, tmode,false); 
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
					TextInBox(hDC,hbuf,sc.x+NIBLSCALE(15), sc.y, 0, tmode,false); 
				}
			}
#endif
		} // visible nearest obstacle
	} // obstacles detected
  } // within glide range

  SelectObject(hDC, hpOld);
}

