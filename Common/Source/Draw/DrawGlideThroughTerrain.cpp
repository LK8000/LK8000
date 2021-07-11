/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Bitmaps.h"
#include <string.h>
#include "RGB.h"
#include "Multimap.h"
#include "LKObjects.h"
#include "ScreenProjection.h"
//
// Glide through terrain will paint a cross over the first and last obstacle to
// the destination.
// It will also draw the perimeter, even if a shaded area was painted previously
//

void MapWindow::DrawGlideThroughTerrain(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj) {
  //double h,dh;
  TCHAR hbuf[10];
  static bool doinit=true;
  static TextInBoxMode_t tmode = {};
  bool wrotevalue=false;

  if (doinit) {
    memset((void*)&tmode, 0, sizeof(TextInBoxMode_t));
    tmode.Border=1;
    doinit=false;
  }

  bool ValidTP = ValidTaskPoint(ActiveTaskPoint);

  const auto hpOld = Surface.SelectObject(hpTerrainLineBg); 

  if (DerivedDrawInfo.GlideFootPrint_valid) {
    // Draw the wide, solid part of the glide terrain line.
#ifdef ENABLE_OPENGL
    // first point is center of polygon (OpenGL GL_TRIANGLE_FAN), polyline start is second point
    const auto polyline_start  = std::next(Groundline.begin());
#else
    const auto polyline_start  = Groundline.begin();
#endif
    const size_t polyline_size = std::distance(polyline_start,Groundline.end());

    Surface.Polyline(&(*polyline_start),polyline_size, rc);

    // draw perimeter if selected and during a flight
    if (((FinalGlideTerrain == 1) || (FinalGlideTerrain == 3)) ||
            ((!IsMultimapTerrain() || !DerivedDrawInfo.Flying) && FinalGlideTerrain)) {

      Surface.SelectObject(hpTerrainLine);
      Surface.Polyline(&(*polyline_start),polyline_size, rc);
    }
  }
  
  // draw glide terrain line around next waypoint
  if (DerivedDrawInfo.GlideFootPrint2_valid) {
    // Draw a solid white line.
    Surface.SelectObject(LKPen_White_N2);
    Surface.Polyline(Groundline2.data(), Groundline2.size(), rc);

    // Draw a dashed red line.
    Surface.DrawDashPoly(NIBLSCALE(2), RGB_RED, Groundline2.data(), Groundline2.size(), rc);
  }

  // draw red cross obstacles only if destination looks reachable!
  // only if using OVT_TASK of course!
  if ((OvertargetMode == OVT_TASK) && DerivedDrawInfo.Flying && ValidTP)
  if (WayPointCalc[TASKINDEX].AltArriv[AltArrivMode] >0) {

	// If calculations detected an obstacle...
	if ((DerivedDrawInfo.TerrainWarningLatitude != 0.0) &&(DerivedDrawInfo.TerrainWarningLongitude != 0.0)) {

		// only if valid position, and visible
		if (DerivedDrawInfo.FarObstacle_Lon >0) 
		if (PointVisible(DerivedDrawInfo.FarObstacle_Lon, DerivedDrawInfo.FarObstacle_Lat)) {
			const POINT sc = _Proj.ToRasterPoint(DerivedDrawInfo.FarObstacle_Lat, DerivedDrawInfo.FarObstacle_Lon);
			DrawBitmapIn(Surface, sc, hTerrainWarning);

			if (DerivedDrawInfo.FarObstacle_AltArriv <=-50 ||  DerivedDrawInfo.FarObstacle_Dist<5000 ) {
				_stprintf(hbuf,_T(" %.0f"),ALTITUDEMODIFY*DerivedDrawInfo.FarObstacle_AltArriv);
				TextInBox(Surface,&rc,hbuf,sc.x+NIBLSCALE(15), sc.y, &tmode,false); 
				wrotevalue=true;
			}
		} // visible far obstacle

		if (PointVisible(DerivedDrawInfo.TerrainWarningLongitude, DerivedDrawInfo.TerrainWarningLatitude)) {
			const POINT sc = _Proj.ToRasterPoint(DerivedDrawInfo.TerrainWarningLatitude, DerivedDrawInfo.TerrainWarningLongitude);
			DrawBitmapIn(Surface, sc, hTerrainWarning);
#if 0
			// 091203 add obstacle altitude on moving map
			RasterTerrain::Lock();
			h =  max(0,RasterTerrain::GetTerrainHeight(DerivedDrawInfo.TerrainWarningLatitude, 
				DerivedDrawInfo.TerrainWarningLongitude));
			RasterTerrain::Unlock();
			if (h==TERRAIN_INVALID) h=0; //@ 101027 FIX but unused
			dh = CALCULATED_INFO.NavAltitude - h - (SAFETYALTITUDETERRAIN/10);
			_stprintf(hbuf,_T(" %.0f"),ALTITUDEMODIFY*dh);
			TextInBox(hDC,&rc,hbuf,sc.x+NIBLSCALE(10), sc.y, 0, tmode,false); 
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
						TextInBox(Surface,&rc,hbuf,sc.x+NIBLSCALE(15), sc.y, &tmode,false); 
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
					TextInBox(Surface,&rc,hbuf,sc.x+NIBLSCALE(15), sc.y, &tmode,false); 
				}
			}
#endif
		} // visible nearest obstacle
	} // obstacles detected
  } // within glide range

  Surface.SelectObject(hpOld);
}

