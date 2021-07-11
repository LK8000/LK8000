/*
 LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 Released under GNU/GPL License v.2 or later
 See CREDITS.TXT file for authors and copyrights

 $Id$
 */

#include "externs.h"
#include "LKObjects.h"
#include "Multimap.h"
#include "Asset.hpp"

//
// List follows enums in dlgConfiguration. 
//
#define AICON_TRIANGLE         1
#define AICON_PARAGLIDER       2
#define AICON_HANGGLIDER       3
#define AICON_GLIDER_THICK     4
#define AICON_GAAIRCRAFT       5
#define AICON_GLIDER_THIN      6
#define AICON_GEM_BIG          7
#define AICON_GLIDER_BLACK     8
#define AICON_GLIDER_BIGBLACK     9

void MapWindow::DrawAircraft(LKSurface& Surface, const POINT& Orig) {

   const double angle = DisplayAircraftAngle + (DerivedDrawInfo.Heading - DrawInfo.TrackBearing);
   unsigned short aicon=0;

   //
   // If we are using default shape, pick up the right one for mode type
   //
   if (GliderSymbol==0) {
      if (ISPARAGLIDER) aicon=AICON_TRIANGLE; else
      if (ISGLIDER) aicon=AICON_GLIDER_THICK; else
      if (ISGAAIRCRAFT) aicon=AICON_GAAIRCRAFT; else
      if (ISCAR) aicon=AICON_TRIANGLE; else {
         StartupStore(_T(". UNKNOWN AIRCRAFT STATUS%s"),NEWLINE);
         LKASSERT(0);
         aicon=AICON_TRIANGLE;
      }
   } else {
      aicon=GliderSymbol;
   }

   auto hpOld = Surface.SelectObject(LK_NULL_PEN);
   auto hbOld = Surface.SelectObject(LK_BLACK_BRUSH);

   switch (aicon) {

      //
      //  1 TRIANGLE
      //
      case AICON_TRIANGLE:
      {
         POINT AircraftInner[] = {
            { 0, -5 }, { 5, 9 }, { -5, 9 }, { 0, -5 }
            };
         POINT AircraftOuter[] = {
            { 0, -6 }, { 6, 10 }, { -6, 10 }, { 0, -6 }
            };

         BrushReference hbPAircraftSolid;
         BrushReference hbPAircraftSolidBg;

         if (BlackScreen) {
            hbPAircraftSolid = LK_WHITE_BRUSH;
            hbPAircraftSolidBg = LKBrush_LightCyan;
         } else {
            hbPAircraftSolid = LK_WHITE_BRUSH;
            hbPAircraftSolidBg = (IsDithered() ? LK_BLACK_BRUSH : LKBrush_Blue);
         }

         PolygonRotateShift(AircraftInner, std::size(AircraftInner), Orig.x, Orig.y, angle);
         PolygonRotateShift(AircraftOuter, std::size(AircraftOuter), Orig.x, Orig.y, angle);

         Surface.SelectObject(LK_WHITE_PEN);
         Surface.SelectObject(hbPAircraftSolid);

         Surface.Polygon(AircraftOuter, std::size(AircraftOuter));

         Surface.SelectObject(hbPAircraftSolidBg);
         Surface.Polygon(AircraftInner, std::size(AircraftInner));

         break;
      }
      //
      //  2 GLIDER THICK
      //
      case AICON_GLIDER_THICK:
      {
         POINT AircraftInner[] = {
            { 1, -5 }, { 2, -2 }, { 2, 0 }, { 10, 0 }, { 16, 1 }, { 16, 2 }, { 2, 2 }, { 1, 6 },
            { 1, 13 }, { 5, 13 }, { 5, 14 }, { -4, 14 }, { -4, 13 }, { 0, 13 }, { 0, 6 }, { -1, 2 }, 
            { -15, 2 }, { -15, 1 }, { -9, 0 }, { -1, 0 }, { -1, -2 }, { 0, -5 }, { 1, -5 }
            };
         POINT AircraftOuter[] = {
            { 2, -6 }, { 3, -3 }, { 3, -1 }, { 11, -1 }, { 17, 0 }, { 17, 3 }, { 3, 3 }, { 2, 7 },
            { 2, 12 }, { 6, 12 }, { 6, 15 }, { -5, 15 }, { -5, 12 }, { -1, 12 }, { -1, 7 }, { -2, 3 },
            { -16, 3 }, { -16, 0 }, { -8, -1 }, { -2, -1 }, { -2, -3 }, { -1, -6 }, { 2, -6 }
            };

         PolygonRotateShift(AircraftInner, std::size(AircraftInner), Orig.x, Orig.y, angle);
         PolygonRotateShift(AircraftOuter, std::size(AircraftOuter), Orig.x, Orig.y, angle);

         Surface.SelectObject(LK_NULL_PEN);

         Surface.SelectObject(IsDithered() ? LKBrush_Grey : LK_BLACK_BRUSH);
         Surface.Polygon(AircraftOuter, std::size(AircraftOuter));

         Surface.SelectObject(IsDithered() ? LK_BLACK_BRUSH : LK_WHITE_BRUSH);
         Surface.Polygon(AircraftInner, std::size(AircraftInner));

         break;
      }
      //
      //  3 GLIDER THIN
      //
      case AICON_GLIDER_THIN:
      {
         POINT AircraftOuter[] = {
            { 2, -6 }, { 2, -1 }, { 15, -1 },{ 15, 2 }, { 2, 2 }, { 2, 7 },
            { 5, 7 }, { 5, 10 }, { -4, 10 }, { -4, 7 }, { -1, 7 },
            { -1, 2 }, { -14, 2 }, { -14, -1 }, { -1, -1 }, { -1, -6 },
            { 2, -6 }
            };
         POINT AircraftInner[] = {
            { 1, -5 }, { 1, 0 }, { 14, 0 },{ 14, 1 }, { 1, 1 }, { 1, 8 },
            { 4, 8 }, { 4, 9 }, { -3, 9 }, { -3, 8 }, { 0, 8 },
            { 0, 1 }, { -13, 1 }, { -13, 0 }, { 0, 0 }, { 0, -5 },
            { 1, -5 }
            };

         PolygonRotateShift(AircraftInner, std::size(AircraftInner), Orig.x, Orig.y, angle);
         PolygonRotateShift(AircraftOuter, std::size(AircraftOuter), Orig.x, Orig.y, angle);

         Surface.SelectObject(LK_NULL_PEN);

         Surface.SelectObject(IsDithered() ? LKBrush_Grey : LK_BLACK_BRUSH);
         Surface.Polygon(AircraftOuter, std::size(AircraftOuter));

         Surface.SelectObject(IsDithered() ? LK_BLACK_BRUSH : LK_WHITE_BRUSH);
         Surface.Polygon(AircraftInner, std::size(AircraftInner));

         break;
      }
      //
      //  4 PARAGLIDER
      //
      case AICON_PARAGLIDER:
      {
         POINT AircraftInner[] = {
            { -5, 3 }, { -11, 2 }, { -14, 1 }, { -14, -1 }, { -11, -2 },
            { -5, -3 }, { 5, -3 }, { 11, -2 }, { 14, -1 }, { 14, 1 },
            { 11, 2 }, { 5, 3 }, { -5, 3 }
            };

         PolygonRotateShift(AircraftInner, std::size(AircraftInner), Orig.x, Orig.y, angle);

         Surface.SelectObject(LK_WHITE_PEN);
         Surface.SelectObject(LK_BLACK_BRUSH);

         Surface.Polygon(AircraftInner, std::size(AircraftInner));

         break;
      }
      //
      //  5 HANG GLIDER
      //
      case AICON_HANGGLIDER:
      {
         POINT AircraftInner[] = {
            { 1, -3 }, { 7, 0 }, { 13, 4 }, { 13, 6 }, { 6, 3 }, { 1, 2 },
            { -1, 2 }, { -6, 3 }, { -13, 6 }, { -13, 4 }, { -7, 0 },
            { -1, -3 }, { 1, -3 }
            };

         PolygonRotateShift(AircraftInner, std::size(AircraftInner), Orig.x, Orig.y, angle);

         Surface.SelectObject(LK_WHITE_PEN);
         Surface.SelectObject(LK_BLACK_BRUSH);

         Surface.Polygon(AircraftInner, std::size(AircraftInner));

         break;
      }
      //
      //  6 GENERAL AVIATION AIRCRAFT
      //
      case AICON_GAAIRCRAFT:
      {
         POINT AircraftInner[] = {
            { 1, -6 }, { 2, -1 }, { 15, 0 }, { 15, 2 }, { 1, 2 }, { 0, 10 },
            { 4, 11 }, { 4, 12 }, { -4, 12 }, { -4, 11 }, { 0, 10 },
            { -1, 2 }, { -15, 2 }, { -15, 0 }, { -2, -1 }, { -1, -6 },
            { 1, -6 }
            };
         POINT AircraftOuter[] = {
            { 2, -7 }, { 3, -2 }, { 16, -1 }, { 16, 3 }, { 2, 3 }, { 1, 9 },
            { 5, 10 }, { 5, 13 }, { -5, 13 }, { -5, 10 }, { -1, 9 },
            { -2, 3 }, { -16, 3 }, { -16, -1 }, { -3, -2 }, { -2, -7 },
            { 2, -7 }
            };

         Surface.SelectObject(LK_NULL_PEN);

         Surface.SelectObject(IsDithered() ? LKBrush_Grey : LK_BLACK_BRUSH);
         PolygonRotateShift(AircraftOuter, std::size(AircraftOuter), Orig.x, Orig.y, angle);
         Surface.Polygon(AircraftOuter, std::size(AircraftOuter));


         Surface.SelectObject(IsDithered() ? LK_BLACK_BRUSH : LK_WHITE_BRUSH);
         PolygonRotateShift(AircraftInner, std::size(AircraftInner), Orig.x, Orig.y, angle);
         Surface.Polygon(AircraftInner, std::size(AircraftInner));


         break;
      }
      //
      //  7 GEM BIG
      //
      case AICON_GEM_BIG:
      {
         POINT AircraftInner[] = {
            { 0, -13 }, { 8, 7 }, { 5, 12 }, { 0, 14 }, { -5, 12}, { -8, 7 },
            { 0, -13 }
            };
         POINT AircraftOuter[] = {
            { 0, -16 }, { 10, 7 }, { 7, 12 }, { 0, 16 }, { -7, 12}, { -10, 7 },
            { 0, -16 }
            };

         Surface.SelectObject(LK_NULL_PEN);
         Surface.SelectObject(IsDithered() ? LKBrush_Grey : LK_BLACK_BRUSH);
         PolygonRotateShift(AircraftOuter, std::size(AircraftOuter), Orig.x, Orig.y, angle);
         Surface.Polygon(AircraftOuter, std::size(AircraftOuter));

         Surface.SelectObject(IsDithered() ? LK_BLACK_BRUSH : LKBrush_Yellow);
         PolygonRotateShift(AircraftInner, std::size(AircraftInner), Orig.x, Orig.y, angle);
         Surface.Polygon(AircraftInner, std::size(AircraftInner));

         break;
      }
       //
       //  8 Glider Black .. expecially for Kobo
       //
     case AICON_GLIDER_BLACK:
     {
       POINT AircraftOuter[] = {
           { 2, -6 }, { 2, -1 }, { 15, -1 },{ 15, 2 }, { 2, 2 }, { 2, 7 },
           { 5, 7 }, { 5, 10 }, { -4, 10 }, { -4, 7 }, { -1, 7 },
           { -1, 2 }, { -14, 2 }, { -14, -1 }, { -1, -1 }, { -1, -6 },
           { 2, -6 }
       };

       PolygonRotateShift(AircraftOuter, std::size(AircraftOuter), Orig.x, Orig.y, angle);
       Surface.SelectObject(LK_NULL_PEN);
       Surface.SelectObject(LK_BLACK_BRUSH);
       Surface.Polygon(AircraftOuter, std::size(AircraftOuter));
       break;
     }
       //
       //  9 Bg Glider Black .. expecially for Kobo
       //
     case AICON_GLIDER_BIGBLACK:
     {
       POINT AircraftOuter[] = {
           { 2, -6 }, { 3, -3 }, { 3, -1 }, { 11, -1 }, { 17, 0 }, { 17, 3 }, { 3, 3 }, { 2, 7 },
           { 2, 12 }, { 6, 12 }, { 6, 15 }, { -5, 15 }, { -5, 12 }, { -1, 12 }, { -1, 7 }, { -2, 3 },
           { -16, 3 }, { -16, 0 }, { -8, -1 }, { -2, -1 }, { -2, -3 }, { -1, -6 }, { 2, -6 }
       };
       PolygonRotateShift(AircraftOuter, std::size(AircraftOuter), Orig.x, Orig.y, angle);
       Surface.SelectObject(LK_NULL_PEN);
       Surface.SelectObject(LK_BLACK_BRUSH);
       Surface.Polygon(AircraftOuter, std::size(AircraftOuter));
       break;
     }




      //
      // IMPOSSIBLE
      //
      default:
         LKASSERT(0);
         break;
    
   }

   Surface.SelectObject(hpOld);
   Surface.SelectObject(hbOld);

}




