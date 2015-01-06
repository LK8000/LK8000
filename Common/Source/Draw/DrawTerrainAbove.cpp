/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKObjects.h"
//
// Draw the reachable SHADED terrain glide amoeba
// This is not the outlined perimeter
//
void MapWindow::DrawTerrainAbove(LKSurface& Surface, const RECT& rc) {

  // Lets try to make it better understandable with a goto.
  // If CAR or GA users dont want amoeba, they should disable it in config. 
  // Otherwise we should paint it, not hide it automatically!
  // Here are the conditions we print this amoeba, otherwise we return;

  // First is we are in SIM mode and we changed the altitude;
  if (SIMMODE && DerivedDrawInfo.AltitudeAGL>100) goto _doit;

  // Second, if we are flying
  if (DerivedDrawInfo.Flying) goto _doit;

  return;
#if !defined(USE_MEMORY_CANVAS) && !defined(HAVE_HATCHED_BRUSH) 
#error "Shading Glide not supported"
#endif

_doit:

  LKColor whitecolor = LKColor(0xff,0xff,0xff);
  LKColor graycolor = LKColor(0xf0,0xf0,0xf0);
  LKColor origcolor = hdcTempTerrainAbove.SetTextColor(whitecolor);

  hdcTempTerrainAbove.SetBackgroundTransparent();

  hdcTempTerrainAbove.SetBkColor(whitecolor);

  hdcTempTerrainAbove.SelectObject(LK_WHITE_PEN);
  hdcTempTerrainAbove.SetTextColor(graycolor);
  hdcTempTerrainAbove.SelectObject(hAboveTerrainBrush);
  hdcTempTerrainAbove.Rectangle(rc.left,rc.top,rc.right,rc.bottom);
  hdcTempTerrainAbove.SelectObject(LK_WHITE_PEN);
  hdcTempTerrainAbove.SelectObject(LKBrush_White);
  hdcTempTerrainAbove.Polygon(Groundline,NUMTERRAINSWEEPS+1);

  // need to do this to prevent drawing of colored outline
  hdcTempTerrainAbove.SelectObject(LK_WHITE_PEN);
#ifdef HAVE_HATCHED_BRUSH
  Surface.TransparentCopy(
          rc.left,rc.top,
          rc.right-rc.left,rc.bottom-rc.top,
          hdcTempTerrainAbove,
          rc.left,rc.top);
#else
  Surface.AlphaBlendNotWhite(rc, hdcTempTerrainAbove, rc, 255/2);
#endif

  // restore original color
  hdcTempTerrainAbove.SetTextColor(origcolor);
  hdcTempTerrainAbove.SetBackgroundOpaque();

}

