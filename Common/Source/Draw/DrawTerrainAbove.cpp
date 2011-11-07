/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

//
// Draw the reachable terrain glide amoeba
//
void MapWindow::DrawTerrainAbove(HDC hDC, const RECT rc) {

  if (!DerivedDrawInfo.Flying &&
	!(SIMMODE && DerivedDrawInfo.AltitudeAGL>100) ) return;

  if (!DerivedDrawInfo.FreeFlying) return;

  COLORREF whitecolor = RGB(0xff,0xff,0xff);
  COLORREF graycolor = RGB(0xf0,0xf0,0xf0);
  COLORREF origcolor = SetTextColor(hDCTemp, whitecolor);

  SetBkMode(hDCTemp, TRANSPARENT);

  SelectObject(hDCTemp, (HBITMAP)hDrawBitMapTmp);
  SetBkColor(hDCTemp, whitecolor);

  SelectObject(hDCTemp, GetStockObject(WHITE_PEN));
  SetTextColor(hDCTemp, graycolor);
  SelectObject(hDCTemp, hAboveTerrainBrush); // hAirspaceBrushes[3] or 6
  Rectangle(hDCTemp,rc.left,rc.top,rc.right,rc.bottom);

  SelectObject(hDCTemp, GetStockObject(WHITE_PEN));
  SelectObject(hDCTemp, GetStockObject(WHITE_BRUSH));
  Polygon(hDCTemp,Groundline,NUMTERRAINSWEEPS+1);

  // need to do this to prevent drawing of colored outline
  SelectObject(hDCTemp, GetStockObject(WHITE_PEN));
#if (WINDOWSPC<1)
    TransparentImage(hDC,
                     rc.left, rc.top,
                     rc.right-rc.left,rc.bottom-rc.top,
                     hDCTemp,
                     rc.left, rc.top,
                     rc.right-rc.left,rc.bottom-rc.top,
                     whitecolor
                     );
    
#else
    TransparentBlt(hDC,
                   rc.left,rc.top,
                   rc.right-rc.left,rc.bottom-rc.top,
                   hDCTemp,
                   rc.left,rc.top,
                   rc.right-rc.left,rc.bottom-rc.top,
                   whitecolor
                   );
  #endif

  // restore original color
  SetTextColor(hDCTemp, origcolor);
  SetBkMode(hDCTemp,OPAQUE);

}

