/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "MapWindow.h"
#include "Multimap.h"
#include "Sideview.h"

//
// Simple thin black line separator between side and topview, if needed
//
void MapWindow::DrawMultimap_SideTopSeparator(LKSurface& Surface, const RECT& rci) {
  if ( DrawRect.bottom>0 && Current_Multimap_SizeY<SIZE4 ) {
	POINT line[2];
	line[0].x = rci.left;
	line[1].x = rci.right;
	line[0].y=DrawRect.bottom;
	line[1].y=DrawRect.bottom;
        Surface.DrawLine(PEN_SOLID, 1, line[0], line[1], RGB_BLACK,rci);
  }
}

