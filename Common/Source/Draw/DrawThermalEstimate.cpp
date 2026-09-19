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

namespace {

void DrawThermalSource(LKSurface& Surface, const RasterPoint& screen, PixelScalar radius,
                       const PixelRect& pixelRect) {

  const PixelScalar nibl = NIBLSCALE<PixelScalar>(1);

  const auto oldPen = Surface.SelectObject(LKPen_White_N2);

  Surface.DrawCircle(screen, radius, pixelRect, false);
  Surface.SelectObject(LKPen_Black_N1);
  Surface.DrawCircle(screen, radius - nibl, pixelRect, false);
  Surface.DrawCircle(screen, radius + nibl, pixelRect, false);

  Surface.SelectObject(oldPen);
}

}  // namespace

//
// Draw circles and gadgets for thermals
//
void MapWindow::DrawThermalEstimate(LKSurface& Surface, const RECT& rc,
                                    const ScreenProjection& _Proj) {
  if (!EnableThermalLocator) {
    return;
  }

  if (mode.Is(Mode::MODE_CIRCLING)) {
    if (DerivedDrawInfo.ThermalEstimate_R > 0) {
      const auto screen =
          _Proj.ToScreen<RasterPoint>({DerivedDrawInfo.ThermalEstimate_Latitude,
                              DerivedDrawInfo.ThermalEstimate_Longitude});

      DrawBitmapIn(Surface, screen, hBmpThermalSource);

      const PixelScalar radius = ((ISPARAGLIDER) ? 50 : 100) * zoom.ResScaleOverDistanceModify();

      DrawThermalSource(Surface, screen, radius, PixelRect(rc));
    }
  }
  else {
    if (zoom.RealScale() <= 4) {
      for (auto& source : DerivedDrawInfo.ThermalSources) {
        if (source.Visible) {
          DrawBitmapIn(Surface, source.Screen, hBmpThermalSource);
        }
      }
    }
  }
}

//
// Paint a circle around thermal multitarget
// Called only during map mode L>
//
void MapWindow::DrawThermalEstimateMultitarget(LKSurface& Surface,
                                               const RECT& rc,
                                               const ScreenProjection& _Proj) {
  // do not mix old and new thermals
  if (mode.Is(Mode::MODE_CIRCLING)) {
    return;
  }

  // draw only when visible , at high zoom level
  if (MapWindow::zoom.RealScale() > 1) {
    return;
  }

  auto thermal = GetThermalMultitarget();
  // no L> target destination
  if (!thermal) {
    return;
  }

  const PixelScalar radius = ((ISPARAGLIDER) ? 100 : 200) * zoom.ResScaleOverDistanceModify();

  auto screen = _Proj.ToScreen<RasterPoint>(thermal->position);
  DrawThermalSource(Surface, screen, radius, PixelRect(rc));
}
