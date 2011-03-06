/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: MapWindow2.cpp,v 8.16 2010/12/26 22:05:15 root Exp root $
*/

#include "StdAfx.h"
#include "compatibility.h"
#include "options.h"
#include "Defines.h"

#include "MapWindow.h"
#include "externs.h"
#include "Terrain.h"
#ifdef LKAIRSPACE
#include "LKAirspace.h"
#endif

#include "utils/heapcheck.h"


// pointer to AlphaBlend() function (initialized in AlphaBlendInit())
//static 
MapWindow::TAlphaBlendF MapWindow::AlphaBlendF = NULL;

// airspace drawing type
//static 
int MapWindow::AirspaceFillType = MapWindow::asp_fill_pattern;

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
#ifdef LKAIRSPACE
  CAirspaceList::const_iterator it;
  const CAirspaceList& airspaces_to_draw = CAirspaceManager::Instance().GetNearAirspacesRef();
  int airspace_type;
#else
  unsigned int i;
#endif
  bool found = false;

  // draw without border
#ifdef LKAIRSPACE
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
#else
  if (AirspaceCircle) {
    for(i = 0; i < NumberOfAirspaceCircles; i++) {
      if (AirspaceCircle[i].Visible == 2) {
        if (!found) {
          found = true;
          ClearTptAirSpace(hdc, rc);
        }
        
        // set filling brush
        SelectObject(hDCTemp, GetAirSpaceSldBrushByClass(AirspaceCircle[i].Type));
        Circle(hDCTemp,
               AirspaceCircle[i].Screen.x ,
               AirspaceCircle[i].Screen.y ,
               AirspaceCircle[i].ScreenR ,rc, true, true);
      }
    }
  }


  if (AirspaceArea) {
    for(i = 0; i < NumberOfAirspaceAreas; i++) {
      if(AirspaceArea[i].Visible == 2) {
        if (!found) {
          found = true;
          ClearTptAirSpace(hdc, rc);
        }
        
        // set filling brush
        SelectObject(hDCTemp, GetAirSpaceSldBrushByClass(AirspaceArea[i].Type));
        ClipPolygon(hDCTemp,
                    AirspaceScreenPoint + AirspaceArea[i].FirstPoint,
                    AirspaceArea[i].NumPoints, rc, true);
      }
    }
  }
#endif

  // alpha blending
  if (found)
    DoAlphaBlend(hdc, rc, hDCTemp, rc, (255 * GetAirSpaceOpacity()) / 100);
  
  // draw it again, just the outlines
  
  // we will be drawing directly into given hdc, so store original PEN object
  HPEN hOrigPen = (HPEN) SelectObject(hdc, GetStockObject(WHITE_PEN));

#ifdef LKAIRSPACE
	for (it=airspaces_to_draw.begin(); it != airspaces_to_draw.end(); it++) {
        if ((*it)->DrawStyle()) {
		  airspace_type = (*it)->Type();
		  if (bAirspaceBlackOutline) {
			SelectObject(hdc, GetStockObject(BLACK_PEN));
		  } else {
			SelectObject(hdc, hAirspacePens[airspace_type]);
		  }
		  (*it)->Draw(hdc, rc, false);
		  
		  // Draw warning position if any
		  if ((*it)->UserWarningState() > awNone) {
			POINT sc;
			double lon;
			double lat;
			(*it)->GetWarningPoint(lon,lat);
			if (PointVisible(lon, lat)) {
				LatLon2Screen(lon, lat, sc);
				DrawBitmapIn(hdc, sc, hTerrainWarning);			//TODO
			}
		  }
        }
	}//for
#else
  if (AirspaceCircle) {
    for(i = 0; i < NumberOfAirspaceCircles; i++) {
      if (AirspaceCircle[i].Visible) {
        if (bAirspaceBlackOutline) {
          SelectObject(hdc, GetStockObject(BLACK_PEN));
        } else {
          SelectObject(hdc, hAirspacePens[AirspaceCircle[i].Type]);
        }
        
        Circle(hdc,
               AirspaceCircle[i].Screen.x ,
               AirspaceCircle[i].Screen.y ,
               AirspaceCircle[i].ScreenR ,rc, true, false);
      }
    }
  }

  if (AirspaceArea) {
    for(i = 0; i < NumberOfAirspaceAreas; i++) {
      if(AirspaceArea[i].Visible) {
        if (bAirspaceBlackOutline) {
          SelectObject(hdc, GetStockObject(BLACK_PEN));
        } else {
          SelectObject(hdc, hAirspacePens[AirspaceArea[i].Type]);
        }

        POINT *pstart = AirspaceScreenPoint + AirspaceArea[i].FirstPoint;
        ClipPolygon(hdc, pstart,
                    AirspaceArea[i].NumPoints, rc, false);
        
        if (AirspaceArea[i].NumPoints>2) {
          // JMW close if open
          if ((pstart[0].x != pstart[AirspaceArea[i].NumPoints-1].x) ||
              (pstart[0].y != pstart[AirspaceArea[i].NumPoints-1].y)) {
            POINT ps[2];
            ps[0] = pstart[0];
            ps[1] = pstart[AirspaceArea[i].NumPoints-1];
            _Polyline(hdc, ps, 2, rc);
          }
        }
      }
    }
  }
#endif
  // restore original PEN
  SelectObject(hdc, hOrigPen);
} // DrawTptAirSpace()


