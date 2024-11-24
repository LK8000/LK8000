/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id#
*/

#include "externs.h"
#include "Bitmaps.h"
#include "LKObjects.h"
#include "ScreenProjection.h"
#include "Calc/ThermalHistory.h"

//
// Draw circles and gadgets for thermals
//
void MapWindow::DrawThermalEstimate(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj) {
  if (!EnableThermalLocator) return;

  if (mode.Is(Mode::MODE_CIRCLING)) {
	if (DerivedDrawInfo.ThermalEstimate_R>0) {
		const POINT screen = _Proj.ToRasterPoint(DerivedDrawInfo.ThermalEstimate_Latitude, DerivedDrawInfo.ThermalEstimate_Longitude);
		DrawBitmapIn(Surface, screen, hBmpThermalSource);

		const auto oldBrush = Surface.SelectObject(LKBrush_Hollow);
		double tradius;
		if (ISPARAGLIDER)
			tradius=50;
		else
			tradius=100;

		const auto oldPen = Surface.SelectObject(LKPen_White_N3);
		Surface.DrawCircle(screen.x, screen.y, (int)(tradius*zoom.ResScaleOverDistanceModify()), rc, true);
		Surface.SelectObject(LKPen_Black_N1);
		Surface.DrawCircle(screen.x, screen.y, (int)(tradius*zoom.ResScaleOverDistanceModify())+NIBLSCALE(2), rc, true);
		Surface.DrawCircle(screen.x, screen.y, (int)(tradius*zoom.ResScaleOverDistanceModify()), rc, true);

		Surface.SelectObject(oldPen);
        Surface.SelectObject(oldBrush);
	}
  } else {
	if (zoom.RealScale() <= 4) {
		for (int i=0; i<MAX_THERMAL_SOURCES; i++) {
			if (DerivedDrawInfo.ThermalSources[i].Visible) {
				DrawBitmapIn(Surface, DerivedDrawInfo.ThermalSources[i].Screen, hBmpThermalSource);
			}
		}
	}
  }
}



//
// Paint a circle around thermal multitarget
// Called only during map mode L>
//
void MapWindow::DrawThermalEstimateMultitarget(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj) {

  // do not mix old and new thermals
  if (mode.Is(Mode::MODE_CIRCLING))
    return;

  // draw only when visible , at high zoom level
  if ( MapWindow::zoom.RealScale() >1 ) return;

  auto thermal = GetThermalMultitarget();
  // no L> target destination
  if (!thermal) {
    return;
  }

  auto screen = _Proj.ToScreen<RasterPoint>(thermal->position);

  double tradius;
  if (ISPARAGLIDER)
     tradius=100;
  else
     tradius=200;

  const auto oldPen = Surface.SelectObject(LKPen_White_N3);

  Surface.DrawCircle(screen.x, screen.y, (int)(tradius*zoom.ResScaleOverDistanceModify()), rc, false);
  Surface.SelectObject(LKPen_White_N2);
  Surface.DrawCircle(screen.x, screen.y, (int)(tradius*zoom.ResScaleOverDistanceModify())+NIBLSCALE(2), rc, false);
  Surface.DrawCircle(screen.x, screen.y, (int)(tradius*zoom.ResScaleOverDistanceModify()), rc, false);

  Surface.SelectObject(oldPen);
}
