/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: MapWindow2.cpp,v 8.16 2010/12/26 22:05:15 root Exp root $
*/

#include "externs.h"

#include "Terrain.h"
#include "RGB.h"
#include "LKObjects.h"
#include <functional>
#include <utils/stl_utils.h>

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Scope.hpp"
#include <memory>
#endif

using std::placeholders::_1;

#define AIRSPACE_BORDER        // switch for new airspace orders


// airspace drawing type
//static
#ifdef HAVE_HATCHED_BRUSH
MapWindow::EAirspaceFillType MapWindow::AirspaceFillType = MapWindow::asp_fill_patterns_full;
#else
MapWindow::EAirspaceFillType MapWindow::AirspaceFillType = MapWindow::asp_fill_ablend_borders;
#endif
// alpha blended airspace opacity (0..100)
//static
BYTE MapWindow::AirspaceOpacity = 30;

  // solid brushes for airspace drawing (initialized in InitAirSpaceSldBrushes())
//static
LKBrush MapWindow::hAirSpaceSldBrushes[NUMAIRSPACECOLORS];

// initialize solid color brushes for airspace drawing (initializes hAirSpaceSldBrushes[])
//static
void MapWindow::InitAirSpaceSldBrushes(const LKColor (&colours)[NUMAIRSPACECOLORS]) {
#ifdef ENABLE_OPENGL
    const int8_t alpha = 0xFF * AirspaceOpacity/100;
#endif
  // initialize solid color brushes for airspace drawing
  for (int i = 0; i < NUMAIRSPACECOLORS; i++) {
#ifdef ENABLE_OPENGL
    hAirSpaceSldBrushes[i].Create(colours[i].WithAlpha(alpha));
#else
    hAirSpaceSldBrushes[i].Create(colours[i]);
#endif
  }
} // InitAirSpaceSldBrushes()


#ifndef ENABLE_OPENGL
// draw airspace using alpha blending
//static
void MapWindow::ClearTptAirSpace(LKSurface& Surface, const RECT& rc) {
  // copy original bitmap into temp (for saving fully transparent areas)
  int width  = rc.right - rc.left;
  int height = rc.bottom - rc.top;
  TempSurface.Copy(rc.left, rc.top, width, height, Surface, rc.left, rc.top);

  if (GetAirSpaceFillType() == asp_fill_ablend_borders) {
    // Prepare layers
    hdcbuffer.FillRect(&rc, LKBrush_White);
    hdcbuffer.SelectObject(LK_NULL_PEN);

    hdcMask.FillRect(&rc, LKBrush_Black);
    hdcMask.SelectObject(hAirspaceBorderPen);
    hdcMask.SelectObject(LKBrush_Hollow);
  }
} // ClearTptAirSpace()


// TODO code: optimise airspace drawing (same as DrawAirSpace())
// draw airspace using alpha blending
//static
void MapWindow::DrawTptAirSpace(LKSurface& Surface, const RECT& rc) {
  // since standard GDI functions (brushes, pens...) ignore alpha octet in ARGB
  // color value and don't set it in the resulting bitmap, we cannot use
  // perpixel alpha blending, instead we use global opacity for alpha blend
  // (same opacity for all pixels); for fully "transparent" areas (without
  // airspace) we must copy destination bitmap into source bitmap first so that
  // alpha blending of such areas results in the same pixels as origin pixels
  // in destination
  const CAirspaceList& airspaces_to_draw = CAirspaceManager::Instance().GetNearAirspacesRef();
  int airspace_type;
  bool found = false;
  bool borders_only = (GetAirSpaceFillType() == asp_fill_ablend_borders);
  #if ASPOUTLINE
  #else
  bool outlined_only=(GetAirSpaceFillType()==asp_fill_border_only);
  #endif

  static bool asp_selected_flash = false;
  asp_selected_flash = !asp_selected_flash;

  int nDC1 = hdcbuffer.SaveState();
  int nDC2 = hdcMask.SaveState();
  int nDC3 = TempSurface.SaveState();
#ifdef AIRSPACE_BORDER
DrawAirSpaceBorders(Surface, rc);
#endif
  // Draw airspace area
    if (1) {
    ScopeLock guard(CAirspaceManager::Instance().MutexRef());
    if (borders_only) {
       // Draw in reverse order!
       // The idea behind this, is lower top level airspaces are smaller. (statistically)
       // They have to be draw later, because inside border area have to be in correct color,
       // not the color of the bigger airspace above this small one.
      for (auto itr=airspaces_to_draw.rbegin(); itr != airspaces_to_draw.rend(); ++itr) {
            if ((*itr)->DrawStyle() == adsFilled) {
              airspace_type = (*itr)->Type();
              if (!found) {
                found = true;
                ClearTptAirSpace(Surface, rc);
              }
              // set filling brush
              hdcbuffer.SelectObject(GetAirSpaceSldBrushByClass(airspace_type));
              (*itr)->Draw(hdcbuffer, true);
              (*itr)->Draw(hdcMask, false);
            }
      }//for
    } else {
       // Draw in direct order!
      for (auto it=airspaces_to_draw.begin(); it != airspaces_to_draw.end(); ++it) {
            if ((*it)->DrawStyle() == adsFilled) {
              airspace_type = (*it)->Type();
              if (!found) {
                found = true;
                ClearTptAirSpace(Surface, rc);
              }
              // set filling brush
              TempSurface.SelectObject(GetAirSpaceSldBrushByClass(airspace_type));
              (*it)->Draw(TempSurface, true);
            }
      }//for
    }//else borders_only
    }//mutex release

  // alpha blending
  if (found) {
    if (borders_only) {
        TempSurface.CopyWithMask(
                rc.left,rc.top,
                rc.right-rc.left,rc.bottom-rc.top,
                hdcbuffer,rc.left,rc.top,
                hdcMask,rc.left,rc.top);
    }
#ifdef USE_MEMORY_CANVAS
    Surface.AlphaBlendNotWhite(rc, TempSurface, rc, (255 * GetAirSpaceOpacity()) / 100);
#else
    Surface.AlphaBlend(rc, TempSurface, rc, (255 * GetAirSpaceOpacity()) / 100);
#endif
  }

  // draw it again, just the outlines

  // we will be drawing directly into given hdc, so store original PEN object
  const auto hOrigPen = Surface.SelectObject(LK_WHITE_PEN);
#ifdef AIRSPACE_BORDER
  if(0)
#endif
    if (1) {
    ScopeLock guard(CAirspaceManager::Instance().MutexRef());
	for (auto it=airspaces_to_draw.begin(); it != airspaces_to_draw.end(); ++it) {
        if ((*it)->DrawStyle()) {
		  airspace_type = (*it)->Type();
#if ASPOUTLINE
if (bAirspaceBlackOutline ^ (asp_selected_flash && (*it)->Selected()) ) {
#else
if ( (((*it)->DrawStyle()==adsFilled)&&!outlined_only&&!borders_only) ^ (asp_selected_flash && (*it)->Selected()) ) {
#endif
			Surface.SelectObject(LK_BLACK_PEN);
		  } else
		   if(  (*it)->DrawStyle()==adsDisabled)   {
			Surface.SelectObject(LKPen_Grey_N2);
		   } else {
			Surface.SelectObject(hAirspacePens[airspace_type]);
		  }
#ifndef AIRSPACE_BORDER
   (*it)->Draw(hdc, rc, false);
#endif
        }
	}//for
    }

  // restore original PEN
  Surface.SelectObject(hOrigPen);

  hdcbuffer.RestoreState(nDC1);
  hdcMask.RestoreState(nDC2);
  TempSurface.RestoreState(nDC3);
} // DrawTptAirSpace()
#else
void MapWindow::DrawTptAirSpace(LKSurface& Surface, const RECT& rc) {

    ScopeLock guard(CAirspaceManager::Instance().MutexRef());
    const CAirspaceList& airspaces_to_draw = CAirspaceManager::Instance().GetNearAirspacesRef();

    const bool borders_only = (GetAirSpaceFillType() == asp_fill_ablend_borders);

    static bool asp_selected_flash = false;
    asp_selected_flash = !asp_selected_flash;

    for (auto it=airspaces_to_draw.begin(); it != airspaces_to_draw.end(); ++it) {

        if (((*it)->DrawStyle() == adsHidden) ||
          ((  (*it)->Top().Base == abMSL) && ((*it)->Top().Altitude <= 0))){
          continue;  // don't draw on map if hidden or upper limit is on sea level or below
        }

        const int airspace_type = (*it)->Type();

        std::unique_ptr<const GLEnable<GL_STENCIL_TEST>> stencil;
        if(borders_only) {
            stencil = std::make_unique<const GLEnable<GL_STENCIL_TEST>>();

            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glDepthMask(GL_FALSE);
            glStencilFunc(GL_NEVER, 1, 0xFF);
            glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);  // draw 1s on test fail (always)
                        glStencilMask(0xFF);
            glClear(GL_STENCIL_BUFFER_BIT);  // needs mask=0xFF

            // Fill Stencil
            Surface.SelectObject(hAirspaceBorderPen);
            Surface.SelectObject(LKBrush_Hollow);
            (*it)->Draw(Surface, true);

            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);
            glStencilMask(0x00);
            // draw where stencil's value is not 0
            glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
        }

        // Draw Airspaces

        if ( (((*it)->DrawStyle()==adsFilled)&&!borders_only) ^ (asp_selected_flash && (*it)->Selected()) ) {
            Surface.SelectObject(LK_BLACK_PEN);
        } else if(  (*it)->DrawStyle()==adsDisabled)   {
            Surface.SelectObject(LKPen_Grey_N2);
        } else {
            Surface.SelectObject(hAirspacePens[airspace_type]);
        }


        if ((*it)->DrawStyle() == adsFilled) {
            Surface.SelectObject(GetAirSpaceSldBrushByClass(airspace_type));
        } else {
            Surface.SelectObject(LKBrush_Hollow);
        }
        (*it)->Draw(Surface, true);
    }//for
}
#endif