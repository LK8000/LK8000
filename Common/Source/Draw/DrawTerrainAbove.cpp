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

_doit:

  LKColor whitecolor = LKColor(0xff,0xff,0xff);
  LKColor graycolor = LKColor(0xf0,0xf0,0xf0);
  LKColor origcolor = TempSurface.SetTextColor(whitecolor);

  TempSurface.SetBackgroundTransparent();

  TempSurface.SetBkColor(whitecolor);

  TempSurface.SelectObject(LK_WHITE_PEN);
  TempSurface.SetTextColor(graycolor);
  TempSurface.SelectObject(hAboveTerrainBrush);
  TempSurface.Rectangle(rc.left,rc.top,rc.right,rc.bottom);
  TempSurface.SelectObject(LK_WHITE_PEN);
  TempSurface.SelectObject(LKBrush_White);
  TempSurface.Polygon(Groundline,NUMTERRAINSWEEPS+1);

  // need to do this to prevent drawing of colored outline
  TempSurface.SelectObject(LK_WHITE_PEN);
#ifdef HAVE_HATCHED_BRUSH
  Surface.TransparentCopy(
          rc.left,rc.top,
          rc.right-rc.left,rc.bottom-rc.top,
          TempSurface,
          rc.left,rc.top);
#elif !defined(ENABLE_OPENGL)
  Surface.AlphaBlendNotWhite(rc, TempSurface, rc, 255/2);
#else
#warning "Shading Glide not supported"  
#endif

  // restore original color
  TempSurface.SetTextColor(origcolor);
  TempSurface.SetBackgroundOpaque();

}

