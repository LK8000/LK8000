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

  if (DoInit[MDI_MAPWPVECTORS])
  {
    //
    // How long and thick is the runway drawn, and how big is the circle.
    // No rule possible, it depends on how we are dealing with many other
    // custom things on screen, because LK is drawing differently depending
    // on geometry, resolution, orientation. We dont split bits.
    //
    switch(ScreenSize)
    {
     case ss240x320: rwl = 9.0; rwb = 2.0;cir = 4.0; break; // 43
     case ss240x400: rwl = 9.0; rwb = 1.0;cir = 4.0; break; // 53
     case ss272x480: rwl = 9.0; rwb = 2.5;cir = 4.0; break; // 169
     case ss480x640: rwl = 6.0; rwb = 2.5;cir = 5.0; break; // 43
     case ss600x800: rwl = 6.0; rwb = 2.5;cir = 5.0; break; // 43
     case ss480x800: rwl = 6.0; rwb = 2.5;cir = 5.0; break; // 53
     case sslandscape: rwl = 6.0; rwb = 1.0;cir = 5.0; break; // sslandscape is never assigned!
     case ss320x240: rwl = 9.0; rwb = 2.0;cir = 4.0; break; // 43
     case ss400x240: rwl = 9.0; rwb = 1.0;cir = 4.0; break; // 53
     case ss480x234: rwl = 9.0; rwb = 1.0;cir = 4.0; break; // 21
     case ss480x272: rwl = 9.0; rwb = 2.5;cir = 4.0; break; // 169
     case ss640x480: rwl = 6.0; rwb = 2.5;cir = 5.0; break; // 43
     case ss720x408: rwl = 6.0; rwb = 2.5;cir = 5.0; break; // 169
     case ss800x480: rwl = 6.0; rwb = 2.5;cir = 5.0; break; // 53
     case ss896x672: rwl = 6.0; rwb = 2.5;cir = 5.0; break; // 43
     case ssnone:

         #define X ScreenSizeX==
	 #define Y ScreenSizeY==
         // Ok this doesnt look nice, but lets remember we are inside a DoInit.
         // Generally the generic setup is good for everyone, but in case we 
         // can make it custom for any resolution.
         if (X 1024 && Y 768) {; rwl = 4.5; rwb = 2.5; cir = 5.0; } else
         if (X 1014 && Y 758) {; rwl = 4.5; rwb = 2.5; cir = 5.0; } else

         // .. and this is the rule of thumb, I could not find a better idea
         // I think it is easier to redesign the drawing approach and make it
         // really scalable. 
         //
         // This hack works for: 800x600 960x540 1280x720 (and relative portrait modes)
         // These are the screen resolutions used by kobo, samsung s4 mini and s5 mini
         // 
         if (ScreenLandscape) {
             if (ScreenSizeX<=480) {
		 // testlk> Ok: t5     Bad for: -
                 rwl = 9.0*(480.0/ScreenSizeX); rwb = 2.5*(480.0/ScreenSizeX); cir = 4.0;
             } else {
		 // testlk> Ok:  3,13,14,22    Bad: t4, t15, t23
                 rwl = 6.0; rwb = 2.5; cir = 5.0;
             }
         } else {
             if (ScreenSizeX<480) {
		 // testlk> Ok: 10,19     Bad for: 20
                 rwl = 6.0*(480.0/ScreenSizeX); rwb = 2.5*(480.0/ScreenSizeX); cir = 4.0;
             } else {
		 // testlk> Ok: 8,28     Bad for: -
                 rwl = 6.0; rwb = 2.5; cir = 5.0;
             }
         }
         break;
    }

    // 
    // These are the (absolute) scale thresholds for painting informations
    // For example, paint just radio starting from 5.0km (3.6abs) etc.
    // There is no rule possible, it matters only testing. 
    //
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

                scale_drawradio=3.6;
                scale_bigfont=1.5;
                scale_fullinfos=1.5;
                break;
    }


    DoInit[MDI_MAPWPVECTORS]=false;
  }

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
		Surface.CircleNoCliping(Center_x, Center_y, p,  rc,true);
	else
		Surface.Circle(Center_x, Center_y, p,  rc, true, true);
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
	  	  Surface.SelectObject(LKBrush_DarkGrey );
	    else
		  Surface.SelectObject(LKBrush_White);
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

	    if (picto)
	       threadsafePolygonRotateShift(WhiteWing, 15,  Center_x, Center_y,  0/*+ wp->RunwayDir-Brg*/);
	    else
	       PolygonRotateShift(WhiteWing, 15,  Center_x, Center_y,  0/*+ wp->RunwayDir-Brg*/);

	    Surface.Polygon(WhiteWing ,15 );
    }
  }

  // StartupStore(_T(".......fscale=%f *1600=%f realscale = %f\n"), fScaleFact, fScaleFact*1600, MapWindow::zoom.RealScale());


  if( !picto && (MapWindow::zoom.RealScale() <= scale_drawradio)  ) 
  {

	const auto hfOld = Surface.SelectObject(MapWindow::zoom.RealScale() <= scale_bigfont
                                                ? LK8PanelUnitFont
                                                : LK8GenericVar02Font);

	if (INVERTCOLORS)
		Surface.SelectObject(LKBrush_Petrol);
	else
		Surface.SelectObject(LKBrush_LightCyan);

	unsigned int offset = p + NIBLSCALE(1) ;
	{
		if ( _tcslen(wp->Freq)>0 ) {
			MapWindow::LKWriteBoxedText(Surface,rc,wp->Freq, Center_x- offset, Center_y -offset, 0, WTALIGN_RIGHT, RGB_WHITE, RGB_BLACK);
		}

		//
		// Full infos! 1.5km scale
		//
		if (MapWindow::zoom.RealScale() <=scale_fullinfos) {
			if ( _tcslen(wp->Code)==4 ) {
				MapWindow::LKWriteBoxedText(Surface,rc,wp->Code,Center_x + offset, Center_y - offset, 0, WTALIGN_LEFT, RGB_WHITE,RGB_BLACK);
			}

			if (wp->Altitude >0) {
				TCHAR tAlt[20];
				_stprintf(tAlt,_T("%.0f %s"),wp->Altitude*ALTITUDEMODIFY,Units::GetUnitName(Units::GetUserAltitudeUnit()));
				MapWindow::LKWriteBoxedText(Surface,rc,tAlt, Center_x + offset, Center_y + offset, 0, WTALIGN_LEFT, RGB_WHITE, RGB_BLACK);
			}

		}
	}
	Surface.SelectObject(hfOld);

  }



  Surface.SelectObject(oldPen);
  Surface.SelectObject(oldBrush);

}


