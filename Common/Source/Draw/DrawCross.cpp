/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "MapWindow.h"
#include "RGB.h"


//
// Cross is drawn when in Pan mode, and aircraft icon is not painted of course
//

void MapWindow::DrawCrossHairs(HDC hdc, const POINT Orig,
			       const RECT rc)
{
  POINT o1, o2;
  
  o1.x = Orig.x+20;
  o2.x = Orig.x-20;
  o1.y = Orig.y;
  o2.y = Orig.y;

  if (BlackScreen)
	  _DrawLine(hdc, PS_SOLID, 1, o1, o2, RGB_INVDRAW, rc);
  else
	  _DrawLine(hdc, PS_SOLID, 1, o1, o2, RGB_DARKGREY, rc);

  o1.x = Orig.x;
  o2.x = Orig.x;
  o1.y = Orig.y+20;
  o2.y = Orig.y-20;

  if (BlackScreen)
	  _DrawLine(hdc, PS_SOLID, 1, o1, o2, RGB_INVDRAW, rc); // 091219
  else
	  _DrawLine(hdc, PS_SOLID, 1, o1, o2, RGB_DARKGREY, rc); // 091219


}


