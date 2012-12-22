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


void MapWindow::ClearAirSpace(bool fill, const RECT& rc) {
  COLORREF whitecolor = RGB(0xff,0xff,0xff);

  SetTextColor(hDCTemp, whitecolor);
  SetBkMode(hDCTemp, TRANSPARENT);	  
  SelectObject(hDCTemp, (HBITMAP)hDrawBitMapTmp);
  SetBkColor(hDCTemp, whitecolor);	  
  SelectObject(hDCTemp, GetStockObject(WHITE_PEN));
  SelectObject(hDCTemp, GetStockObject(WHITE_BRUSH));
  FillRect(hDCTemp, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));        

  if (GetAirSpaceFillType() == asp_fill_patterns_borders) {
    FillRect(mhdcbuffer, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));        
    SelectObject(mhdcbuffer, GetStockObject(NULL_PEN));
  
    FillRect(hDCMask, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));        
    SelectObject(hDCMask, hAirspaceBorderPen);
    SelectObject(hDCMask, GetStockObject(HOLLOW_BRUSH));
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
  #if ASPOUTLINE
  #else
  bool outlined_only=(GetAirSpaceFillType()==asp_fill_border_only);
  #endif
  static bool asp_selected_flash = false;
  asp_selected_flash = !asp_selected_flash;
  
  int nDC1 = SaveDC(mhdcbuffer);
  int nDC2 = SaveDC(hDCMask);
  int nDC3 = SaveDC(hDCTemp);
  
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
            SetTextColor(mhdcbuffer, Colours[iAirspaceColour[airspace_type]]);
            // get brush, can be solid or a 1bpp bitmap
            SelectObject(mhdcbuffer, hAirspaceBrushes[iAirspaceBrush[airspace_type]]);
            (*itr)->Draw(mhdcbuffer, rc, true);
            (*itr)->Draw(hDCMask, rc, false);
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
        SetTextColor(mhdcbuffer, RGB_BLACK);
        MaskBlt(hDCTemp,
                rc.left,rc.top,
                rc.right-rc.left,rc.bottom-rc.top,
                mhdcbuffer,rc.left,rc.top,
                hMaskBitMap,rc.left,rc.top, MAKEROP4(SRCAND,  0x00AA0029));
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
            ClearAirSpace(true, rc);
            found = true;
          }
	  #if ASPOUTLINE
	  if (bAirspaceBlackOutline ^ (asp_selected_flash && (*it)->Selected()) ) {
	  #else
          if ( (((*it)->DrawStyle()==adsFilled)&&!outlined_only&&!borders_only)  ^ (asp_selected_flash && (*it)->Selected()) ) {
	  #endif
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
  RestoreDC(mhdcbuffer, nDC1);
  RestoreDC(hDCMask, nDC2);    
  RestoreDC(hDCTemp, nDC3);    
}


