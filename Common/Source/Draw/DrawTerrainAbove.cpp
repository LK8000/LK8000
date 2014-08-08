/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

//
// Draw the reachable SHADED terrain glide amoeba
// This is not the outlined perimeter
//
void MapWindow::DrawTerrainAbove(HDC hDC, const RECT rc) {

  // Lets try to make it better understandable with a goto.
  // If CAR or GA users dont want amoeba, they should disable it in config. 
  // Otherwise we should paint it, not hide it automatically!
  // Here are the conditions we print this amoeba, otherwise we return;

  // First is we are in SIM mode and we changed the altitude;
  if (SIMMODE && DerivedDrawInfo.AltitudeAGL>100) goto _doit;

  // Second, if we are flying
  if (DerivedDrawInfo.Flying) goto _doit;

  return;


_doit:

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
  TransparentBlt(hDC,
          rc.left,rc.top,
          rc.right-rc.left,rc.bottom-rc.top,
          hDCTemp,
          rc.left,rc.top,
          rc.right-rc.left,rc.bottom-rc.top,
          whitecolor);

  // restore original color
  SetTextColor(hDCTemp, origcolor);
  SetBkMode(hDCTemp,OPAQUE);

}

