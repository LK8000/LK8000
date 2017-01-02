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
#include <string.h>
#include "ScreenGeometry.h"

/**
 * @brief Draw Runway on map
 *
 * @remarks THIS FUNCTION is threadsafe only if called by dialogs using picto bool true
 *
 * @param Surface : Surface to Draw
 * @param wp : Landable Waypoint
 * @param rc : Clipping Rect
 * @param fScaleFact : Scaling factor for runway symbol radius and runway length, use #MapWindow::zoom value for Moving Map or constant for waypoint Picto
 * @param picto true for drawing Waypoint Picto ( don't draw radio info )
 */

void MapWindow::DrawRunway(LKSurface& Surface, const WAYPOINT* wp, const RECT& rc, double fScaleFact, BOOL picto)
{
  int solid= false;
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

  if(picto)
	fScaleFact /=2000;
  else
        fScaleFact /=1600;

  rwl = 4.0; rwb = 2;cir = 3.5;  // Same size on all resoltion // Tony 2017.

  if( wp->RunwayLen > 100) /* square if no runway defined */
  {
    if (picto)
       l = (int) (rwl);
    else {
       // We cant rescale runways , no method found.
       if (ScreenSize==ssnone)
           l = (int) (rwl * (1.0+ (680.0/800.0-1.0)/4.0)); // wtf..!
       else
           l = (int) (rwl * (1.0+ ((double)wp->RunwayLen/800.0-1.0)/4.0));
    }

    b = (int) (rwb/1.5 );
  } else
  {
    l = (int)( rwl*0.5);
    b = l ;
  }

  l = (int)(2 * l * fScaleFact / ScreenScale ); if(l==0) l=1;
  b = (int)(2 * b * fScaleFact / ScreenScale ); if(b==0) b=1;
  p = (int)(cir * 2.0 * fScaleFact); if(p==0) p=1;

  switch(wp->Style) {
	case STYLE_AIRFIELDSOLID: solid = true;  bRunway  = true;  bOutland = false;  bGlider  = false;	break;
	case STYLE_AIRFIELDGRASS: solid = false; bRunway  = true;  bOutland = false;  bGlider  = false;	break;
	case STYLE_OUTLANDING	: solid = false; bRunway  = true;  bOutland = true;   bGlider  = false; b*=2; break;
	case STYLE_GLIDERSITE	: solid = false; bRunway  = true;  bOutland = false;  bGlider  = true;	break;
	default: return; break;
  }

  // Do not print glidersite at low zoom levels, in any case
  // not useful on some resolutions
  // if( !picto && (MapWindow::zoom.RealScale() > 3) )
  //	bGlider=false;

  const auto oldPen = Surface.SelectObject(LK_BLACK_PEN);
  const auto oldBrush = Surface.SelectObject(LKBrush_Red);

  if( wp->Reachable == TRUE)
    Surface.SelectObject(LKBrush_Green);


  if(!bOutland)
  {
	if (picto)
		Surface.DrawCircle(Center_x, Center_y, p, true);
	else
		Surface.DrawCircle(Center_x, Center_y, p,  rc, true);
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
            #ifndef DITHER
	    if(solid)
		  Surface.SelectObject(LKBrush_DarkGrey );
	    else
		  Surface.SelectObject(LKBrush_White);
            #else
	    if(solid)
		  Surface.SelectObject(LKBrush_Black);
	    else
		  Surface.SelectObject(LKBrush_White);
            #endif
	}
	if(picto) {
	  threadsafePolygonRotateShift(Runway, 5,  Center_x, Center_y,  wp->RunwayDir);
	} else {
	  PolygonRotateShift(Runway, 5,  Center_x, Center_y,  wp->RunwayDir- (int)MapWindow::GetDisplayAngle());
	}
	Surface.Polygon(Runway ,5 );

  } // bRunway


  if(fScaleFact >= 0.9) {
    if(bGlider)
    {
	    int iScale = (int)(fScaleFact*2.0);
		    double fFact = 0.04*fScaleFact/1.5;
            if(iScale==0) iScale=1;
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

	    if (picto)
	       threadsafePolygonRotateShift(WhiteWing, 17,  Center_x, Center_y,  0/*+ wp->RunwayDir-Brg*/);
	    else
	       PolygonRotateShift(WhiteWing, 17,  Center_x, Center_y,  0/*+ wp->RunwayDir-Brg*/);

	    Surface.Polygon(WhiteWing ,17 );
    }
  }

  // StartupStore(_T(".......fscale=%f *1600=%f realscale = %f\n"), fScaleFact, fScaleFact*1600, MapWindow::zoom.RealScale());


  if( !picto && (MapWindow::zoom.RealScale() <= scale_drawradio)  )
  {

	const auto hfOld = Surface.SelectObject(MapWindow::zoom.RealScale() <= scale_bigfont
                                                ? LK8PanelUnitFont
                                                : LK8GenericVar02Font);
        #ifndef DITHER
	if (INVERTCOLORS)
		Surface.SelectObject(LKBrush_Petrol);
	else
		Surface.SelectObject(LKBrush_LightCyan);
        #else
	if (INVERTCOLORS)
		Surface.SelectObject(LKBrush_Black);
	else
		Surface.SelectObject(LKBrush_White);
        #endif

	unsigned int offset = p + NIBLSCALE(1) ;
	{
		if ( _tcslen(wp->Freq)>0 ) {
			MapWindow::LKWriteBoxedText(Surface,rc,wp->Freq, Center_x- offset, Center_y -offset, WTALIGN_RIGHT, RGB_WHITE, RGB_BLACK);
		}

		//
		// Full infos! 1.5km scale
		//
		if (MapWindow::zoom.RealScale() <=scale_fullinfos) {
			if ( _tcslen(wp->Code)==4 ) {
				MapWindow::LKWriteBoxedText(Surface,rc,wp->Code,Center_x + offset, Center_y - offset, WTALIGN_LEFT, RGB_WHITE,RGB_BLACK);
			}

			if (wp->Altitude >0) {
				TCHAR tAlt[20];
				_stprintf(tAlt,_T("%.0f %s"),wp->Altitude*ALTITUDEMODIFY,Units::GetUnitName(Units::GetUserAltitudeUnit()));
				MapWindow::LKWriteBoxedText(Surface,rc,tAlt, Center_x + offset, Center_y + offset, WTALIGN_LEFT, RGB_WHITE, RGB_BLACK);
			}

		}
	}
	Surface.SelectObject(hfOld);

  }



  Surface.SelectObject(oldPen);
  Surface.SelectObject(oldBrush);

}
