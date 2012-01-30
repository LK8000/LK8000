/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: MapWindow2.cpp,v 8.16 2010/12/26 22:05:15 root Exp root $
*/

#include "externs.h"

#include "MapWindow.h"
#include "Terrain.h"
#include "LKAirspace.h"
#include "RGB.h"



// pointer to AlphaBlend() function (initialized in AlphaBlendInit())
//static 
MapWindow::TAlphaBlendF MapWindow::AlphaBlendF = NULL;

// airspace drawing type
//static 
MapWindow::EAirspaceFillType MapWindow::AirspaceFillType = MapWindow::asp_fill_patterns_full;

// alpha blended airspace opacity (0..100)
//static 
BYTE MapWindow::AirspaceOpacity = 30;

  // solid brushes for airspace drawing (initialized in InitAirSpaceSldBrushes())
//static 
HBRUSH MapWindow::hAirSpaceSldBrushes[NUMAIRSPACECOLORS];


// tries to locate AlphaBlend() function and initializes some data needed for alpha blending;
// sets pointer to AlphaBlend function (AlphaBlendF) 
// (returns false when AlphaBlending is not supported)
//static
bool MapWindow::AlphaBlendInit() {
  #if (WINDOWSPC>0)
    AlphaBlendF = AlphaBlend;
  #else
    AlphaBlendF = (TAlphaBlendF) GetProcAddress(GetModuleHandle(TEXT("coredll.dll")), TEXT("AlphaBlend"));
  #endif

  if (AlphaBlendF == NULL) {
    #if ALPHADEBUG
    StartupStore(_T(". CoreDll::AlphaBlend() is not available\n"));
    #endif
    return false;
  }

  InitAirSpaceSldBrushes(Colours);
  
  return true;
} // AlphaBlendInit()


// release resources used for alpha blending
//static 
void MapWindow::AlphaBlendDestroy() {
  for (int i = 0; i < NUMAIRSPACECOLORS; i++) {
    if (hAirSpaceSldBrushes[i] != NULL) {
      DeleteObject(hAirSpaceSldBrushes[i]);
      hAirSpaceSldBrushes[i] = NULL;
    }
  }
} // AlphaBlendDestroy()


// performs AlphaBlend
//static 
void MapWindow::DoAlphaBlend(HDC dstHdc, const RECT dstRect, HDC srcHdc, const RECT srcRect, BYTE globalOpacity) {
  if (AlphaBlendF == NULL)
    return;
  
  //BLENDFUNCTION bf = { AC_SRC_OVER, 0, globalOpacity, AC_SRC_ALPHA };
  // we are not using per-pixel alpha, so do not use AC_SRC_ALPHA flag
  BLENDFUNCTION bf = { AC_SRC_OVER, 0, globalOpacity, 0 };
  
  AlphaBlendF(
    dstHdc, dstRect.left, dstRect.top, dstRect.right - dstRect.left, dstRect.bottom - dstRect.top,
    srcHdc, srcRect.left, srcRect.top, srcRect.right - srcRect.left, srcRect.bottom - srcRect.top, bf);
} // DoAlphaBlend()



// initialize solid color brushes for airspace drawing (initializes hAirSpaceSldBrushes[])
//static 
void MapWindow::InitAirSpaceSldBrushes(const COLORREF colours[]) {
  // initialize solid color brushes for airspace drawing
  for (int i = 0; i < NUMAIRSPACECOLORS; i++) {
    if (hAirSpaceSldBrushes[i] != NULL)
      DeleteObject(hAirSpaceSldBrushes[i]);
    
    hAirSpaceSldBrushes[i] = CreateSolidBrush(colours[i]);
  }
} // InitAirSpaceSldBrushes()



// draw airspace using alpha blending
//static 
void MapWindow::ClearTptAirSpace(HDC hdc, const RECT rc) {
  // select temp bitmap
  SelectObject(hDCTemp, hDrawBitMapTmp);
  
  // copy original bitmap into temp (for saving fully transparent areas)
  int width  = rc.right - rc.left;
  int height = rc.bottom - rc.top;
  BitBlt(hDCTemp, rc.left, rc.top, width, height, hdc, rc.left, rc.top, SRCCOPY);
} // ClearTptAirSpace()


// TODO code: optimise airspace drawing (same as DrawAirSpace())
// draw airspace using alpha blending
//static 
void MapWindow::DrawTptAirSpace(HDC hdc, const RECT rc) {
  // since standard GDI functions (brushes, pens...) ignore alpha octet in ARGB 
  // color value and don't set it in the resulting bitmap, we cannot use
  // perpixel alpha blending, instead we use global opacity for alpha blend 
  // (same opacity for all pixels); for fully "transparent" areas (without 
  // airspace) we must copy destination bitmap into source bitmap first so that 
  // alpha blending of such areas results in the same pixels as origin pixels 
  // in destination 
  CAirspaceList::const_iterator it;
  CAirspaceList::const_reverse_iterator itr;
  const CAirspaceList& airspaces_to_draw = CAirspaceManager::Instance().GetNearAirspacesRef();
  int airspace_type;
  bool found = false;
  bool borders_only = (GetAirSpaceFillType() == asp_fill_ablend_borders);
  HDC hdcbuffer = NULL;
  HBITMAP hbbufferold = NULL, hbbuffer = NULL;
  RECT rc_extended;
  static bool asp_selected_flash = false;
  asp_selected_flash = !asp_selected_flash;

  // Extend rc to not show asp borders because of clipping algo
  rc_extended.left = rc.left - NIBLSCALE(10);
  rc_extended.top = rc.top - NIBLSCALE(10);
  rc_extended.bottom = rc.bottom + NIBLSCALE(10);
  rc_extended.right = rc.right + NIBLSCALE(10);
  
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

  // Draw airspace area
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
                found = true;
                ClearTptAirSpace(hdc, rc);
              }
              // set filling brush
              SelectObject(hdcbuffer, GetAirSpaceSldBrushByClass(airspace_type));
              (*itr)->Draw(hdcbuffer, rc, true);
              (*itr)->Draw(hDCMask, rc_extended, false);
            }
      }//for
    } else {
       // Draw in direct order!
      for (it=airspaces_to_draw.begin(); it != airspaces_to_draw.end(); ++it) {
            if ((*it)->DrawStyle() == adsFilled) {
              airspace_type = (*it)->Type();
              if (!found) {
                found = true;
                ClearTptAirSpace(hdc, rc);
              }
              // set filling brush
              SelectObject(hDCTemp, GetAirSpaceSldBrushByClass(airspace_type));
              (*it)->Draw(hDCTemp, rc, true);
            }
      }//for
    }//else borders_only
    }//mutex release

  // alpha blending
  if (found) {
    if (borders_only) {
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
    DoAlphaBlend(hdc, rc, hDCTemp, rc, (255 * GetAirSpaceOpacity()) / 100);
  }
  
  // draw it again, just the outlines
  
  // we will be drawing directly into given hdc, so store original PEN object
  HPEN hOrigPen = (HPEN) SelectObject(hdc, GetStockObject(WHITE_PEN));

    if (1) {
    CCriticalSection::CGuard guard(CAirspaceManager::Instance().MutexRef());
	for (it=airspaces_to_draw.begin(); it != airspaces_to_draw.end(); ++it) {
        if ((*it)->DrawStyle()) {
		  airspace_type = (*it)->Type();
		  if (bAirspaceBlackOutline ^ (asp_selected_flash && (*it)->Selected()) ) {
			SelectObject(hdc, GetStockObject(BLACK_PEN));
		  } else {
			SelectObject(hdc, hAirspacePens[airspace_type]);
		  }
		  (*it)->Draw(hdc, rc_extended, false);
        }
	}//for
    }

  if (borders_only) {
    // Free up GDI resources
    SelectObject(hdcbuffer, hbbufferold);
    DeleteObject(hbbuffer);
    DeleteDC(hdcbuffer);
  }
  
  // restore original PEN
  SelectObject(hdc, hOrigPen);
} // DrawTptAirSpace()


