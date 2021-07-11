/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKStyle.h"
#include "DoInits.h"
#include "LKObjects.h"
#include <string.h>
#include "ScreenGeometry.h"
#include "Draw/ScreenProjection.h"

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

void MapWindow::DrawRunway(LKSurface& Surface, const WAYPOINT* wp, const RECT& rc, const ScreenProjection* _Proj, double fScaleFact, BOOL picto)
{
  if(!picto && !_Proj) {
      // if we don't draw picto ScreenProjection parameter are mandatory.
      assert(false);
      return;
  }

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

  RasterPoint Center;
  if(picto) {
	  Center.x = rc.left+ (rc.right- rc.left)/2;
	  Center.y = rc.bottom +(rc.top-rc.bottom)/2;
  } else {
      Center =  _Proj->ToRasterPoint(wp->Latitude, wp->Longitude);
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
     case ss800x600: rwl = 6.0; rwb = 2.5;cir = 5.0; break; // 43
     case ss800x480: rwl = 6.0; rwb = 2.5;cir = 5.0; break; // 53
     case ssnone:
#if defined(ANDROID) || defined(KOBO)
                 rwl = 6.0; rwb = 2.5; cir = 5.0; break;
#else
      //   the line above is only a workaround to avoid runaway to be tool long on  Anrtoid and Kobo
      //   waiting  for a definitive and more elegant solution.
      //

         #define X ScreenSizeX==
	 #define Y ScreenSizeY==
         // Ok this doesnt look nice, but lets remember we are inside a DoInit.
         // Generally the generic setup is good for everyone, but in case we
         // can make it custom for any resolution.
         if (X 1024 && Y 768) {; rwl = 4.5; rwb = 2.5; cir = 5.0; } else
         if (X 1014 && Y 758) {; rwl = 4.5; rwb = 2.5; cir = 5.0; } else
         if (X 720 && Y 408) {; rwl = 10.0; rwb = 6.0; cir = 5.0; } else
         if (X 720 && Y 432) {; rwl = 10.0; rwb = 4.0; cir = 5.0; } else

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
#endif

    }


    //
    // These are the (absolute) scale thresholds for painting informations
    // For example, paint just radio starting from 5.0km (3.6abs) etc.
    // There is no rule possible, it matters only testing.
    //
    switch(ScreenSize)
    {

	case ss480x272:
		scale_drawradio=2.6;
		scale_bigfont=1.1;
		scale_fullinfos=0.8;
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
#ifndef __linux__
       // We cant rescale runways , no method found.
       if (ScreenSize==ssnone)
           l = (int) (rwl * (1.0+ (680.0/800.0-1.0)/4.0)); // wtf..!
       else
#endif
           l = (int) (rwl * (1.0+ ((double)wp->RunwayLen/800.0-1.0)/4.0));
    }

    b = (int) (rwb/1.5 );
  } else
  {
    l = (int)( rwl*0.5);
    b = l ;
  }

#if defined(ANDROID) || defined(KOBO)
  l = (int)(l * 2.0 * fScaleFact / ScreenScale); if(l==0) l=1;
  b = (int)(b * 2.0 * fScaleFact / ScreenScale); if(b==0) b=1;
  p = (int)(cir * 2.0 * fScaleFact); if(p==0) p=1;
#else
  l = (int)(l * fScaleFact); if(l==0) l=1;
  b = (int)(b * fScaleFact); if(b==0) b=1;
  p = (int)(cir * 2.0 * fScaleFact); if(p==0) p=1;
#endif

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
		Surface.DrawCircle(Center.x, Center.y, p, true);
	else
		Surface.DrawCircle(Center.x, Center.y, p,  rc, true);
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
		if (!IsDithered()) {
			if (solid)
				Surface.SelectObject(LKBrush_DarkGrey);
			else
				Surface.SelectObject(LKBrush_White);
		} else {
			if (solid)
				Surface.SelectObject(LKBrush_Black);
			else
				Surface.SelectObject(LKBrush_White);
		}
	}
	if(picto) {
	  threadsafePolygonRotateShift(Runway, 5,  Center.x, Center.y,  wp->RunwayDir);
	} else {
	  PolygonRotateShift(Runway, 5,  Center.x, Center.y,  wp->RunwayDir- (int)MapWindow::GetDisplayAngle());
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
	       threadsafePolygonRotateShift(WhiteWing, 17,  Center.x, Center.y,  0/*+ wp->RunwayDir-Brg*/);
	    else
	       PolygonRotateShift(WhiteWing, 17,  Center.x, Center.y,  0/*+ wp->RunwayDir-Brg*/);

	    Surface.Polygon(WhiteWing ,17 );
    }
  }

  // StartupStore(_T(".......fscale=%f *1600=%f realscale = %f\n"), fScaleFact, fScaleFact*1600, MapWindow::zoom.RealScale());


  if( !picto && (MapWindow::zoom.RealScale() <= scale_drawradio)  )
  {

	const auto hfOld = Surface.SelectObject(MapWindow::zoom.RealScale() <= scale_bigfont
                                                ? LK8PanelUnitFont
                                                : LK8GenericVar02Font);
	  if (!IsDithered()) {
		  if (INVERTCOLORS)
			  Surface.SelectObject(LKBrush_Petrol);
		  else
			  Surface.SelectObject(LKBrush_LightCyan);
	  } else {
		  if (INVERTCOLORS)
			  Surface.SelectObject(LKBrush_Black);
		  else
			  Surface.SelectObject(LKBrush_White);
	  }

	unsigned int offset = p + NIBLSCALE(1) ;
	{
		if ( _tcslen(wp->Freq)>0 ) {
			MapWindow::LKWriteBoxedText(Surface,rc,wp->Freq, Center.x- offset, Center.y -offset, WTALIGN_RIGHT, RGB_WHITE, RGB_BLACK);
		}

		//
		// Full infos! 1.5km scale
		//
		if (MapWindow::zoom.RealScale() <=scale_fullinfos) {
			if ( _tcslen(wp->Code)==4 ) {
				MapWindow::LKWriteBoxedText(Surface,rc,wp->Code,Center.x + offset, Center.y - offset, WTALIGN_LEFT, RGB_WHITE,RGB_BLACK);
			}

			if (wp->Altitude >0) {
				TCHAR tAlt[20];
				_stprintf(tAlt,_T("%.0f %s"),wp->Altitude*ALTITUDEMODIFY,Units::GetUnitName(Units::GetUserAltitudeUnit()));
				MapWindow::LKWriteBoxedText(Surface,rc,tAlt, Center.x + offset, Center.y + offset, WTALIGN_LEFT, RGB_WHITE, RGB_BLACK);
			}

		}
	}
	Surface.SelectObject(hfOld);

  }



  Surface.SelectObject(oldPen);
  Surface.SelectObject(oldBrush);

}
