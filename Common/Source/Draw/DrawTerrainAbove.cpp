/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKObjects.h"
#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Scope.hpp"
#include <Screen/OpenGL/VertexPointer.hpp>
#endif
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

  if (DerivedDrawInfo.GlideFootPrint_valid)  goto _doit;
    
  return;

_doit:

#ifndef ENABLE_OPENGL
  LKColor origcolor = TempSurface.SetTextColor(RGB_WHITE);

  TempSurface.SetBackgroundTransparent();

  TempSurface.SetBkColor(RGB_WHITE);

  TempSurface.SelectObject(LK_WHITE_PEN);
  TempSurface.SetTextColor(RGB_ICEWHITE);
  TempSurface.SelectObject(hAboveTerrainBrush);
  TempSurface.Rectangle(rc.left,rc.top,rc.right,rc.bottom);
  TempSurface.SelectObject(LK_WHITE_PEN);
  TempSurface.SelectObject(LKBrush_White);
  TempSurface.Polygon(Groundline.data(),Groundline.size());

  // need to do this to prevent drawing of colored outline
  TempSurface.SelectObject(LK_WHITE_PEN);
#ifdef HAVE_HATCHED_BRUSH
  Surface.TransparentCopy(
          rc.left,rc.top,
          rc.right-rc.left,rc.bottom-rc.top,
          TempSurface,
          rc.left,rc.top);
#else
  Surface.AlphaBlendNotWhite(rc, TempSurface, rc, 255/2);
#endif


  // restore original color
  TempSurface.SetTextColor(origcolor);
  TempSurface.SetBackgroundOpaque();
#else

    Canvas& canvas = Surface;
  
    const GLEnable<GL_STENCIL_TEST> stencil;
    
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    glStencilFunc(GL_NEVER, 1, 0xFF);
    glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);  // draw 1s on test fail (always)

    // draw stencil pattern
    glStencilMask(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);  // needs mask=0xFF

    ScopeVertexPointer vp(Groundline.data());
    glDrawArrays(GL_TRIANGLE_FAN, 0, Groundline.size());

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glStencilMask(0x00);
    // draw where stencil's value is 0
    glStencilFunc(GL_EQUAL, 0, 0xFF);

    canvas.DrawFilledRectangle(rc.left,rc.top,rc.right,rc.bottom, AboveTerrainColor);

#endif  

}

