/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Bitmaps.h"



void MapWindow::DrawTeammate(HDC hdc, RECT rc)
{
  POINT point;

  if (TeammateCodeValid)
    {
      if(PointVisible(TeammateLongitude, TeammateLatitude) )
	{
	  LatLon2Screen(TeammateLongitude, TeammateLatitude, point);

	  SelectObject(hDCTemp,hBmpTeammatePosition);
	  DrawBitmapX(hdc,
		      point.x-NIBLSCALE(10), 
		      point.y-NIBLSCALE(10),
		      20,20,
		      hDCTemp,0,0,SRCPAINT);
	
	  DrawBitmapX(hdc,
		      point.x-NIBLSCALE(10), 
		      point.y-NIBLSCALE(10),
		      20,20,
		      hDCTemp,20,0,SRCAND);
	}
    }
}

