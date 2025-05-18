/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#include "Bitmaps.h"
#include "RGB.h"
#include "LKObjects.h"
#include "Draw/ScreenProjection.h"
#include "Airspace/Renderer/AirspaceRenderer.h"

void MapWindow::ClearAirSpace(bool fill, const RECT& rc) {
#ifndef ENABLE_OPENGL
  TempSurface.SetTextColor(RGB_WHITE);
  TempSurface.SetBackgroundTransparent();
  TempSurface.SetBkColor(RGB_WHITE);
  TempSurface.SelectObject(LK_WHITE_PEN);
  TempSurface.SelectObject(LKBrush_White);

  TempSurface.FillRect(&rc, LKBrush_White);
#ifdef HAVE_HATCHED_BRUSH
  if (GetAirSpaceFillType() == asp_fill_patterns_borders) {
    hdcbuffer.FillRect(&rc, LKBrush_White);
    hdcbuffer.SelectObject(LK_NULL_PEN);

    hdcMask.FillRect(&rc, LKBrush_Black);
    hdcMask.SelectObject(hAirspaceBorderPen);
    hdcMask.SelectObject(LKBrush_Hollow);
  }
#endif
#endif
}

#ifdef HAVE_HATCHED_BRUSH
#ifdef ENABLE_OPENGL
#error "airspace pattern not supported with OpenGL"
#endif
// TODO code: optimise airspace drawing
void MapWindow::DrawAirSpacePattern(LKSurface& Surface, const RECT& rc) {
  const CAirspaceList& airspaces_to_draw = CAirspaceManager::Instance().GetNearAirspacesRef();
  int airspace_type;
  bool found = false;
  const bool borders_only = (GetAirSpaceFillType() == asp_fill_patterns_borders);

  const bool outlined_only = (GetAirSpaceFillType() == asp_fill_border_only);
  static bool asp_selected_flash = false;
  asp_selected_flash = !asp_selected_flash;

  int nDC1 = hdcbuffer.SaveState();
  int nDC2 = hdcMask.SaveState();
  int nDC3 = TempSurface.SaveState();

  if (GetAirSpaceFillType() != asp_fill_border_only) {
    ScopeLock guard(CAirspaceManager::Instance().MutexRef());
    if (borders_only) {
      // Draw in reverse order!
      // The idea behind this, is lower top level airspaces are smaller. (statistically)
      // They have to be draw later, because inside border area have to be in correct color,
      // not the color of the bigger airspace above this small one.
      for (auto itr = airspaces_to_draw.rbegin(); itr != airspaces_to_draw.rend(); ++itr) {
        if ((*itr)->DrawStyle() == adsFilled) {
          airspace_type = (*itr)->Type();
          if (!found) {
            ClearAirSpace(true, rc);
            found = true;
          }
          // this color is used as the black bit
          hdcbuffer.SetTextColor(Colours[iAirspaceColour[airspace_type]]);
          // brush, can be solid or a 1bpp bitmap
          (*itr)->FillPolygon(hdcbuffer, hAirspaceBrushes[iAirspaceBrush[airspace_type]]);
          (*itr)->DrawOutline(hdcMask, hAirspaceBorderPen);
        }
      }  // for
    }
    else {
      for (auto it = airspaces_to_draw.begin(); it != airspaces_to_draw.end(); ++it) {
        if ((*it)->DrawStyle() == adsFilled) {
          airspace_type = (*it)->Type();
          if (!found) {
            ClearAirSpace(true, rc);
            found = true;
          }
          // this color is used as the black bit
          TempSurface.SetTextColor(Colours[iAirspaceColour[airspace_type]]);
          // get brush, can be solid or a 1bpp bitmap
          // brush, can be solid or a 1bpp bitmap
          (*it)->FillPolygon(hdcbuffer, hAirspaceBrushes[iAirspaceBrush[airspace_type]]);
        }
      }  // for
    }
  }

  // draw it again, just the outlines
  if (found) {
    if (borders_only) {
      hdcbuffer.SetTextColor(RGB_BLACK);
      TempSurface.CopyWithMask(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                               hdcbuffer, rc.left, rc.top,
                               hdcMask, rc.left, rc.top);
    }
    TempSurface.SelectObject(LKBrush_Hollow);
    TempSurface.SelectObject(LK_WHITE_PEN);
  }

  if (1) {
    ScopeLock guard(CAirspaceManager::Instance().MutexRef());
    for (auto it = airspaces_to_draw.begin(); it != airspaces_to_draw.end(); ++it) {
      if ((*it)->DrawStyle()) {
        airspace_type = (*it)->Type();
        if (!found) {
          ClearAirSpace(true, rc);
          found = true;
        }
        if ((((*it)->DrawStyle() == adsFilled) && !outlined_only && !borders_only) ^
            (asp_selected_flash && (*it)->Selected())) {
          (*it)->DrawOutline(TempSurface, LK_BLACK_PEN);
        }
        else if (((*it)->DrawStyle() == adsDisabled)) {
          (*it)->DrawOutline(TempSurface, LKPen_Grey_N1);
        }
        else {
        (*it)->DrawOutline(TempSurface, hAirspacePens[airspace_type]);
        }
      }
    }  // for
  }

  if (found) {
    // need to do this to prevent drawing of colored outline
    TempSurface.SelectObject(LK_WHITE_PEN);
    Surface.TransparentCopy(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TempSurface, rc.left, rc.top);

    // restore original color
    //    SetTextColor(hDCTemp, origcolor);
    TempSurface.SetBackgroundOpaque();
  }
  hdcbuffer.RestoreState(nDC1);
  hdcMask.RestoreState(nDC2);
  TempSurface.RestoreState(nDC3);
}
#endif

void MapWindow::DrawAirSpace(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj) {

  CalculateScreenPositionsAirspace(rc, _Proj);

  if ((GetAirSpaceFillType() == asp_fill_ablend_full) || (GetAirSpaceFillType() == asp_fill_ablend_borders)) {
    DrawTptAirSpace(Surface, rc);
  }
  else {
    if (GetAirSpaceFillType() == asp_fill_border_only) {
      DrawAirSpaceBorders(Surface, rc);  // full screen, to hide clipping effect on low border
    }
    else {
#ifdef HAVE_HATCHED_BRUSH
      DrawAirSpacePattern(Surface, rc);  // full screen, to hide clipping effect on low border
#else
      DrawAirSpaceBorders(Surface, rc);  // full screen, to hide clipping effect on low border
#endif
    }
  }
}
