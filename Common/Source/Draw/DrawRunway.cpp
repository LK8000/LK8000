/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKStyle.h"
#include "DoInits.h"
#include "LKObjects.h"


#if (WINDOWSPC>0)
#include <wingdi.h>
#endif

#include <string.h>



void MapWindow::DrawRunway(HDC hdc,WAYPOINT* wp, RECT rc, double fScaleFact, BOOL picto)
{
  int solid= false;
  HPEN    oldPen  ;
  HBRUSH  oldBrush ;
  bool bGlider = false;
  bool bOutland = false;
  bool bRunway = false;
  static double rwl = 8;
  static double rwb = 1;
  static double cir = 6;
  static double scale_drawradio=0;
  static double scale_bigfont=0;
  static double scale_fullinfos=0;
  int Center_x = wp->Screen.x;
  int Center_y = wp->Screen.y;
  if(picto)
  {
	  Center_x = rc.left+ (rc.right- rc.left)/2;
	  Center_y = rc.bottom +(rc.top-rc.bottom)/2;
  }
  int l,p,b;

  fScaleFact /=1600;

  if (DoInit[MDI_MAPWPVECTORS])
  {
    switch(ScreenSize)
    {
     case ss240x320: rwl = 9.0; rwb = 2.0;cir = 4.0; break;
     case ss240x400: rwl = 9.0; rwb = 1.0;cir = 4.0; break;
     case ss272x480: rwl = 9.0; rwb = 2.5;cir = 4.0; break;
     case ss480x640: rwl = 6.0; rwb = 2.5;cir = 5.0; break;
     case ss480x800: rwl = 6.0; rwb = 2.5;cir = 5.0; break;
     case sslandscape: rwl = 6.0; rwb = 1.0;cir = 5.0; break;
     case ss320x240: rwl = 9.0; rwb = 2.0;cir = 4.0; break;
     case ss400x240: rwl = 9.0; rwb = 1.0;cir = 4.0; break;
     case ss480x234: rwl = 9.0; rwb = 1.0;cir = 4.0; break;
     case ss480x272: rwl = 9.0; rwb = 2.5;cir = 4.0; break;
     case ss640x480: rwl = 6.0; rwb = 2.5;cir = 5.0; break;
     case ss720x408: rwl = 6.0; rwb = 2.5;cir = 5.0; break;
     case ss800x480: rwl = 6.0; rwb = 2.5;cir = 5.0; break;
     case ss896x672: rwl = 6.0; rwb = 2.5;cir = 5.0; break;
     case ssnone: 
	if (ScreenLandscape) {
		rwl = 6.0; rwb = 2.5;cir = 4.0;
	} else {
		rwl = 9.0; rwb = 2.5;cir = 4.0;
	}
	break;
    }

    // All values <=
    switch(ScreenSize)
    {
	case ss480x272:
		if (ScreenSizeX==854) {
			scale_drawradio=3.6;
			scale_bigfont=1.5;
			scale_fullinfos=1.5;

		} else {
			scale_drawradio=2.6;
			scale_bigfont=1.1;
			scale_fullinfos=0.8;
		}
		break;
	case ss800x480:
		scale_drawradio=2.6;
		scale_bigfont=1.5;
		scale_fullinfos=1.5;
		break;
	case ss640x480:
		scale_drawradio=3.6;
		scale_bigfont=1.5;
		scale_fullinfos=1.5;
		break;

	default:
		scale_drawradio=2.6;
		scale_bigfont=1.5;
		scale_fullinfos=1.5;
		break;
    }
    DoInit[MDI_MAPWPVECTORS]=false;
  }


  if( wp->RunwayLen > 100) /* square if no runway defined */
  {
    l = (int) (rwl * (1.0+ ((double)wp->RunwayLen/800.0-1.0)/4.0));
    b = (int) (rwb/1.5 );
  } else
  {
    l = (int)( rwl*0.5);
    b = l ;
  }

  l = (int)(l * fScaleFact); if(l==0) l=1;
  b = (int)(b * fScaleFact); if(b==0) b=1;
  p = (int)(cir * 2.0 * fScaleFact); if(p==0) p=1;

  switch(wp->Style) {
	case STYLE_AIRFIELDSOLID: solid = true;  bRunway  = true;  bOutland = false;  bGlider  = false;	break;
	case STYLE_AIRFIELDGRASS: solid = false; bRunway  = true;  bOutland = false;  bGlider  = false;	break;
	case STYLE_OUTLANDING	: solid = false; bRunway  = true;  bOutland = true;   bGlider  = false; b*=2; break;
	case STYLE_GLIDERSITE	: solid = false; bRunway  = true;  bOutland = false;  bGlider  = true;	break;
	default: return; break;
  }

  oldPen   = (HPEN) SelectObject(hdc, GetStockObject(BLACK_PEN));
  oldBrush = (HBRUSH)SelectObject(hdc, LKBrush_Red);

  if( wp->Reachable == TRUE)
    SelectObject(hdc, LKBrush_Green);


  if(!bOutland)
  {
	Circle( hdc,Center_x, Center_y, p,  rc,true, true);
  }

  if(bRunway)
  {
	POINT Runway[5] = {
		  { b, l },  // 1
		  {-b, l },  // 2
		  {-b,-l },  // 3
		  { b,-l },  // 4
		  { b,l  }   // 5
	};
	if(!bOutland)
	{
	    if(solid)
	  	  SelectObject(hdc, LKBrush_DarkGrey );
	    else
		  SelectObject(hdc, LKBrush_White);
	}
	PolygonRotateShift(Runway, 5,  Center_x, Center_y,  wp->RunwayDir- (int)MapWindow::GetDisplayAngle());
	Polygon(hdc,Runway ,5 );

  } // bRunway


  if(fScaleFact >= 0.9) { 
    if(bGlider)
    {
	    int iScale = (int)(fScaleFact*2.0);
	    if(iScale==0) iScale=1;
	    POINT WhiteWing [15]  = {
		  { 0 * iScale, 0 * iScale },   // 1
		  { 1 * iScale,-1 * iScale },   // 2
		  { 2 * iScale,-1 * iScale },   // 3
		  { 3 * iScale, 0 * iScale },   // 4
		  { 3 * iScale, 1 * iScale },   // 5
		  { 2 * iScale, 0 * iScale },   // 6
		  { 1 * iScale, 0 * iScale },   // 7
		  { 0 * iScale, 1 * iScale },   // 8
		  {-1 * iScale, 0 * iScale },   // 9
	 	  {-2 * iScale, 0 * iScale },   // 10
		  {-3 * iScale, 1 * iScale },   // 11
		  {-3 * iScale, 0 * iScale },   // 12
		  {-2 * iScale,-1 * iScale },   // 13
		  {-1 * iScale,-1 * iScale },   // 14
		  { 0 * iScale, 0 * iScale }    // 15
	    };
	    PolygonRotateShift(WhiteWing, 15,  Center_x, Center_y,  0/*+ wp->RunwayDir-Brg*/);
	    Polygon(hdc,WhiteWing ,15 );
    }
  }

  // StartupStore(_T(".......fscale=%f *1600=%f realscale = %f\n"), fScaleFact, fScaleFact*1600, MapWindow::zoom.RealScale());


  if( MapWindow::zoom.RealScale() <= scale_drawradio ) 
  {

	HFONT hfOld;

	if (MapWindow::zoom.RealScale() <= scale_bigfont) 
		hfOld = (HFONT)SelectObject(hdc, LK8PanelUnitFont);
	else
		hfOld = (HFONT)SelectObject(hdc, LK8UnitFont);

	if (INVERTCOLORS)
		SelectObject(hdc,LKBrush_Petrol);
	else
		SelectObject(hdc,LKBrush_LightCyan);

	unsigned int offset = p + NIBLSCALE(1) ;
	if( !picto)
	{
		if ( _tcslen(wp->Freq)>0 ) {
			MapWindow::LKWriteBoxedText(hdc,&DrawRect,wp->Freq, Center_x- offset, Center_y -offset, 0, WTALIGN_RIGHT, RGB_WHITE, RGB_BLACK);
		}

		//
		// Full infos! 1.5km scale
		//
		if (MapWindow::zoom.RealScale() <=scale_fullinfos) {
			if ( _tcslen(wp->Code)==4 ) {
				MapWindow::LKWriteBoxedText(hdc,&DrawRect,wp->Code,Center_x + offset, Center_y - offset, 0, WTALIGN_LEFT, RGB_WHITE,RGB_BLACK);
			}

			if (wp->Altitude >0) {
				TCHAR tAlt[20];
				_stprintf(tAlt,_T("%.0f %s"),wp->Altitude*ALTITUDEMODIFY,Units::GetUnitName(Units::GetUserAltitudeUnit()));
				MapWindow::LKWriteBoxedText(hdc,&DrawRect,tAlt, Center_x + offset, Center_y + offset, 0, WTALIGN_LEFT, RGB_WHITE, RGB_BLACK);
			}

		}
	}
	SelectObject(hdc, hfOld);

  }



  SelectObject(hdc, oldPen);
  SelectObject(hdc, oldBrush);


}


