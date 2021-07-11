/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RGB.h"


//
// Cross is drawn when in Pan mode, and aircraft icon is not painted of course
//

void MapWindow::DrawCrossHairs(LKSurface& Surface, const POINT& Orig, const RECT& rc)
{
  POINT o1, o2;

  o1.x = Orig.x+NIBLSCALE(20);
  o2.x = Orig.x-NIBLSCALE(20);
  o1.y = Orig.y;
  o2.y = Orig.y;

  if (BlackScreen)
	  Surface.DrawLine(PEN_SOLID, ScreenThinSize, o1, o2, RGB_INVDRAW, rc);
  else
	  Surface.DrawLine(PEN_SOLID, ScreenThinSize, o1, o2, RGB_DARKGREY, rc);

  Surface.SetPixel(o1.x,o1.y,RGB_YELLOW);
  Surface.SetPixel(o2.x,o2.y,RGB_YELLOW);

  o1.x = Orig.x;
  o2.x = Orig.x;
  o1.y = Orig.y+NIBLSCALE(20);
  o2.y = Orig.y-NIBLSCALE(20);

  if (BlackScreen)
	  Surface.DrawLine(PEN_SOLID, ScreenThinSize, o1, o2, RGB_INVDRAW, rc); // 091219
  else
	  Surface.DrawLine(PEN_SOLID, ScreenThinSize, o1, o2, RGB_DARKGREY, rc); // 091219

  Surface.SetPixel(o1.x,o1.y,RGB_YELLOW);
  Surface.SetPixel(o2.x,o2.y,RGB_YELLOW);


}
