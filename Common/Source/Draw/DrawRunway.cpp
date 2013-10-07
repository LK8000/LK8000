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
   double rwl = 5.33;
   double rwb = 1.0;
   double cir = 4.0;
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

  switch(ScreenSize) {
	case ss240x320:
	case ss320x240:
	case ss480x272:
	case ss272x480:
 	  	fScaleFact /= 800; // (*=1.6 after /= 1600 is equale to /1000)
		break;
	default:
		fScaleFact /=1600;
		break;
  }

  if (DoInit[MDI_MAPWPVECTORS])
  {
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
  } else
  {
    l = (int)( rwl*0.5);
    rwb = l ;
  }

  l = (int)(l * fScaleFact*1.1); if(l==0) l=1;
  b = (int)(rwb * fScaleFact); if(b==0) b=1;
  p = (int)(cir * (double)ScreenScale * fScaleFact); if(p==0) p=1;

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
	Circle( hdc,Center_x, Center_y, p,  rc,false, true);
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
	if(picto) {
	  PolygonRotateShift(Runway, 5,  Center_x, Center_y,  wp->RunwayDir);
	} else {
	  PolygonRotateShift(Runway, 5,  Center_x, Center_y,  wp->RunwayDir- (int)MapWindow::GetDisplayAngle());
	}
	Polygon(hdc,Runway ,5 );

  } // bRunway


  if(fScaleFact >= 1.2) {
    if(bGlider)
    {
	    double fFact = 0.04*fScaleFact/1.5;
	    POINT WhiteWing [17]  = {
		  { (long)(-228  * fFact ) , (long)(13  * fFact)}, //1
		  { (long) (-221 * fFact ) , (long)(-5  * fFact)}, //2
		  { (long) (-102 * fFact ) , (long)(-50 * fFact)}, //3
		  { (long) (8	 * fFact ) , (long)( 5  * fFact)}, //4
		  { (long) (149  * fFact ) , (long)(-55 * fFact)}, //5
		  { (long) (270  * fFact ) , (long)(-12 * fFact)}, //6
		  { (long) (280  * fFact ) , (long)( 5  * fFact)}, //7
		  { (long) (152  * fFact ) , (long)(-30 * fFact)}, //8
		  { (long) (48	 * fFact ) , (long)( 27 * fFact)}, //9
		  { (long) (37	 * fFact ) , (long)( 44 * fFact)}, //10
		  { (long)(-20	 * fFact ) , (long)( 65 * fFact)}, //11
		  { (long)(-29	 * fFact ) , (long)( 80 * fFact)}, //12
		  { (long)(-56	 * fFact ) , (long)( 83 * fFact)}, //13
		  { (long)(-50	 * fFact ) , (long)( 40 * fFact)}, //14
		  { (long)(-30	 * fFact ) , (long)( 27 * fFact)}, //15
		  { (long)(-103  * fFact ) , (long)(-26 * fFact)}, //16
		  { (long)(-228  * fFact ) , (long)( 13 * fFact)}  //17
	    };
	    PolygonRotateShift(WhiteWing, 17,  Center_x, Center_y,  0/*+ wp->RunwayDir-Brg*/);
	    Polygon(hdc,WhiteWing ,17 );
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


