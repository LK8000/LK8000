/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#include "Bitmaps.h"
#include "RGB.h"


#if (WINDOWSPC>0)
#include <wingdi.h>
#endif


void MapWindow::ClearAirSpace(bool fill) {
  COLORREF whitecolor = RGB(0xff,0xff,0xff);

  SetTextColor(hDCTemp, whitecolor);
  SetBkMode(hDCTemp, TRANSPARENT);	  
  SelectObject(hDCTemp, (HBITMAP)hDrawBitMapTmp);
  SetBkColor(hDCTemp, whitecolor);	  
  SelectObject(hDCTemp, GetStockObject(WHITE_PEN));
  SelectObject(hDCTemp, GetStockObject(WHITE_BRUSH));
  Rectangle(hDCTemp,DrawRect.left,DrawRect.top,DrawRect.right,DrawRect.bottom);
  if (fill) {
    SelectObject(hDCTemp, GetStockObject(WHITE_PEN));
  }
}


// TODO code: optimise airspace drawing
void MapWindow::DrawAirSpace(HDC hdc, const RECT rc)
{
  COLORREF whitecolor = RGB(0xff,0xff,0xff);
  CAirspaceList::const_iterator it;
  CAirspaceList::const_reverse_iterator itr;
  const CAirspaceList& airspaces_to_draw = CAirspaceManager::Instance().GetNearAirspacesRef();
  int airspace_type;
  bool found = false;
  bool borders_only = (GetAirSpaceFillType() == asp_fill_patterns_borders);
  HDC hdcbuffer = NULL;
  HBITMAP hbbufferold = NULL, hbbuffer = NULL;
  static bool asp_selected_flash = false;
  asp_selected_flash = !asp_selected_flash;
  
  if (borders_only) {
    // Prepare layers
    hdcbuffer = CreateCompatibleDC(hdc);
    hbbuffer = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
    hbbufferold = (HBITMAP)SelectObject(hdcbuffer, hbbuffer);
    BitBlt(hdcbuffer, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, NULL, rc.left, rc.top, BLACKNESS );
    SelectObject(hdcbuffer, GetStockObject(NULL_PEN));
  
    BitBlt(hDCMask, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, NULL, rc.left, rc.top, BLACKNESS );
    SelectObject(hDCMask, hAirspaceBorderPen);
    SelectObject(hDCMask, GetStockObject(HOLLOW_BRUSH));
  }
  
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
              ClearAirSpace(true);
              found = true;
            }
            // this color is used as the black bit
            SetTextColor(hdcbuffer, Colours[iAirspaceColour[airspace_type]]);
            // get brush, can be solid or a 1bpp bitmap
            SelectObject(hdcbuffer, hAirspaceBrushes[iAirspaceBrush[airspace_type]]);
            (*itr)->Draw(hdcbuffer, rc, true);
            (*itr)->Draw(hDCMask, rc, false);
        }
      }//for
    } else {
      for (it=airspaces_to_draw.begin(); it != airspaces_to_draw.end(); ++it) {
          if ((*it)->DrawStyle() == adsFilled) {
            airspace_type = (*it)->Type();
            if (!found) {
              ClearAirSpace(true);
              found = true;
            }
            // this color is used as the black bit
            SetTextColor(hDCTemp, Colours[iAirspaceColour[airspace_type]]);
            // get brush, can be solid or a 1bpp bitmap
            SelectObject(hDCTemp, hAirspaceBrushes[iAirspaceBrush[airspace_type]]);
            (*it)->Draw(hDCTemp, rc, true);
        }
      }//for
    }
    }
  }
  // draw it again, just the outlines

  if (found) {
    if (borders_only) {
        SetTextColor(hdcbuffer, RGB_BLACK);
        #if (WINDOWSPC<1)
        TransparentImage(hdcbuffer,
                rc.left,rc.top,
                rc.right-rc.left,rc.bottom-rc.top,
                hDCMask,
                rc.left,rc.top,
                rc.right-rc.left,rc.bottom-rc.top,
                RGB_WHITE
                );
        TransparentImage(hDCTemp,
                rc.left,rc.top,
                rc.right-rc.left,rc.bottom-rc.top,
                hdcbuffer,
                rc.left,rc.top,
                rc.right-rc.left,rc.bottom-rc.top,
                RGB_BLACK
                );
        #else
        TransparentBlt(hdcbuffer,
                      rc.left,rc.top,
                      rc.right-rc.left,rc.bottom-rc.top,
                      hDCMask,
                      rc.left,rc.top,
                      rc.right-rc.left,rc.bottom-rc.top,
                      RGB_WHITE
                      );
        TransparentBlt(hDCTemp,
                      rc.left,rc.top,
                      rc.right-rc.left,rc.bottom-rc.top,
                      hdcbuffer,
                      rc.left,rc.top,
                      rc.right-rc.left,rc.bottom-rc.top,
                      RGB_BLACK
                      );
        #endif
    }
    SelectObject(hDCTemp, GetStockObject(HOLLOW_BRUSH));
    SelectObject(hDCTemp, GetStockObject(WHITE_PEN));
  }

    if (1) {
    CCriticalSection::CGuard guard(CAirspaceManager::Instance().MutexRef());
      for (it=airspaces_to_draw.begin(); it != airspaces_to_draw.end(); ++it) {
        if ((*it)->DrawStyle()) {
          airspace_type = (*it)->Type();
          if (!found) {
            ClearAirSpace(true);
            found = true;
          }
          if (bAirspaceBlackOutline ^ (asp_selected_flash && (*it)->Selected()) ) {
            SelectObject(hDCTemp, GetStockObject(BLACK_PEN));
          } else {
            SelectObject(hDCTemp, hAirspacePens[airspace_type]);
          }
          (*it)->Draw(hDCTemp, rc, false);
        }
      }//for
    }

  if (found) {
    // need to do this to prevent drawing of colored outline
    SelectObject(hDCTemp, GetStockObject(WHITE_PEN));
#if (WINDOWSPC<1)
    TransparentImage(hdc,
                     rc.left, rc.top,
                     rc.right-rc.left,rc.bottom-rc.top,
                     hDCTemp,
                     rc.left, rc.top,
                     rc.right-rc.left,rc.bottom-rc.top,
                     whitecolor
                     );
    
#else
    TransparentBlt(hdc,
                   rc.left,rc.top,
                   rc.right-rc.left,rc.bottom-rc.top,
                   hDCTemp,
                   rc.left,rc.top,
                   rc.right-rc.left,rc.bottom-rc.top,
                   whitecolor
                   );
  #endif
    
    // restore original color
    //    SetTextColor(hDCTemp, origcolor);
    SetBkMode(hDCTemp,OPAQUE);
  }
  
  if (borders_only) {
    // Free up GDI resources
    SelectObject(hdcbuffer, hbbufferold);
    DeleteObject(hbbuffer);
    DeleteDC(hdcbuffer);
  }
  
}


