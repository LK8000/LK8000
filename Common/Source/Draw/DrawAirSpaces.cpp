/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#include "Bitmaps.h"
#include "RGB.h"
#include "LKObjects.h"

#if (WINDOWSPC>0)
#include <wingdi.h>
#endif


void MapWindow::ClearAirSpace(bool fill, const RECT& rc) {
      LKColor whitecolor = LKColor(0xff,0xff,0xff);

  hdcTempAsp.SetTextColor(whitecolor);
  hdcTempAsp.SetBackgroundTransparent();
  hdcTempAsp.SetBkColor(whitecolor);
  hdcTempAsp.SelectObject(LK_WHITE_PEN);
  hdcTempAsp.SelectObject(LKBrush_White);

  hdcTempAsp.FillRect(&rc, LKBrush_White);
#ifdef HAVE_HATCHED_BRUSH
  if (GetAirSpaceFillType() == asp_fill_patterns_borders) {
    hdcbuffer.FillRect(&rc, LKBrush_White);
    hdcbuffer.SelectObject(LK_NULL_PEN);
  
    hdcMask.FillRect(&rc, LKBrush_Black);
    hdcMask.SelectObject(hAirspaceBorderPen);
    hdcMask.SelectObject(LKBrush_Hollow);
  }
#endif
}


// TODO code: optimise airspace drawing
void MapWindow::DrawAirSpace(LKSurface& Surface, const RECT& rc)
{
  CAirspaceList::const_iterator it;
  CAirspaceList::const_reverse_iterator itr;
  const CAirspaceList& airspaces_to_draw = CAirspaceManager::Instance().GetNearAirspacesRef();
  int airspace_type;
  bool found = false;
#ifdef HAVE_HATCHED_BRUSH
  const bool borders_only = (GetAirSpaceFillType() == asp_fill_patterns_borders);
#else
  const bool borders_only = false;
#endif
  const bool outlined_only=(GetAirSpaceFillType()==asp_fill_border_only);
  static bool asp_selected_flash = false;
  asp_selected_flash = !asp_selected_flash;
  
  int nDC1 = hdcbuffer.SaveState();
  int nDC2 = hdcMask.SaveState();
  int nDC3 = hdcTempAsp.SaveState();

#ifdef HAVE_HATCHED_BRUSH    
  if (GetAirSpaceFillType() != asp_fill_border_only) {
    if (1) {
    CCriticalSection::CGuard guard(CAirspaceManager::Instance().MutexRef());
    if (borders_only) {
       // Draw in reverse order!
       // The idea behind this, is lower top level airspaces are smaller. (statistically)
       // They have to be draw later, because inside border area have to be in correct color,
       // not the color of the bigger airspace above this small one.
      for (itr=airspaces_to_draw.rbegin(); itr != airspaces_to_draw.rend(); ++itr) {
          if ((*itr)->DrawStyle() == adsFilled) {
            airspace_type = (*itr)->Type();
            if (!found) {
              ClearAirSpace(true, rc);
              found = true;
            }
            // this color is used as the black bit
            hdcbuffer.SetTextColor(Colours[iAirspaceColour[airspace_type]]);
            // get brush, can be solid or a 1bpp bitmap
            hdcbuffer.SelectObject(hAirspaceBrushes[iAirspaceBrush[airspace_type]]);
            (*itr)->Draw(hdcbuffer, rc, true);
            (*itr)->Draw(hdcMask, rc, false);
        }
      }//for
    } else {
      for (it=airspaces_to_draw.begin(); it != airspaces_to_draw.end(); ++it) {
          if ((*it)->DrawStyle() == adsFilled) {
            airspace_type = (*it)->Type();
            if (!found) {
              ClearAirSpace(true, rc);
              found = true;
            }
            // this color is used as the black bit
            hdcTempAsp.SetTextColor(Colours[iAirspaceColour[airspace_type]]);
            // get brush, can be solid or a 1bpp bitmap
            hdcTempAsp.SelectObject(hAirspaceBrushes[iAirspaceBrush[airspace_type]]);
            (*it)->Draw(hdcTempAsp, rc, true);
        }
      }//for
    }
    }
  }
#endif
  // draw it again, just the outlines
  if (found) {
    if (borders_only) {
        hdcbuffer.SetTextColor(RGB_BLACK);
        hdcTempAsp.CopyWithMask(
                rc.left,rc.top,
                rc.right-rc.left,rc.bottom-rc.top,
                hdcbuffer,rc.left,rc.top,
                hdcMask,rc.left,rc.top);
    }
    hdcTempAsp.SelectObject(LKBrush_Hollow);
    hdcTempAsp.SelectObject(LK_WHITE_PEN);
  }

    if (1) {
    CCriticalSection::CGuard guard(CAirspaceManager::Instance().MutexRef());
      for (it=airspaces_to_draw.begin(); it != airspaces_to_draw.end(); ++it) {
        if ((*it)->DrawStyle()) {
          airspace_type = (*it)->Type();
          if (!found) {
            ClearAirSpace(true, rc);
            found = true;
          }
          if ( (((*it)->DrawStyle()==adsFilled)&&!outlined_only&&!borders_only)  ^ (asp_selected_flash && (*it)->Selected()) ) {
            hdcTempAsp.SelectObject(LK_BLACK_PEN);
          } else {
            hdcTempAsp.SelectObject(hAirspacePens[airspace_type]);
          }
		  if(((*it)->DrawStyle()==adsDisabled))
		    hdcTempAsp.SelectObject(LKPen_Grey_N1);
          (*it)->Draw(hdcTempAsp, rc, false);
        }
      }//for
    }

  if (found) {
    // need to do this to prevent drawing of colored outline
    hdcTempAsp.SelectObject(LK_WHITE_PEN);
    Surface.TransparentCopy(
                   rc.left,rc.top,
                   rc.right-rc.left,rc.bottom-rc.top,
                   hdcTempAsp,
                   rc.left,rc.top);
    
    // restore original color
    //    SetTextColor(hDCTemp, origcolor);
    hdcTempAsp.SetBackgroundOpaque();
  }
  hdcbuffer.RestoreState(nDC1);
  hdcMask.RestoreState(nDC2);    
  hdcTempAsp.RestoreState(nDC3);    
}


