/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Sideview.h"
#include "LKObjects.h"
#include "Asset.hpp"


// draw aircraft
void RenderPlaneSideview(LKSurface& Surface, double fDist, double fAltitude,double brg, DiagrammStruct* psDia )
{
//BOOL bInvCol = true ; //INVERTCOLORS
  #define NO_AP_PTS 17
  const double fCos = fastcosine(brg);
  const double fSin = fastsine(brg);

  int TAIL   = 6;
  int PROFIL = 1;
  int FINB   = 3;
  int BODY   = 2;
  int NOSE   = 7;
  int WING   = (int) (22.0 );
  int TUBE   = (int) (14.0  ) ;
  int FINH   = 6+BODY;

  POINT Start;
  int HEAD = TUBE / 2;
  TUBE =  3 * TUBE/ 2;
  POINT AircraftSide
  [8] = {
      {(int)(fSin * (HEAD+0   )    ), -BODY-1},  // 1
      {(int)(fSin * (HEAD+NOSE)    ),  0},       // 2
      {(int)(fSin * (HEAD+0   )    ),  BODY+1},  // 3
      {(int)(fSin * (-TUBE)        ),  BODY},    // 4   -1
      {(int)(fSin * -TUBE          ), -FINH},    // 5
      {(int)(fSin * (-TUBE+FINB)   ), -FINH},    // 6
      {(int)(fSin * (-TUBE+FINB+3) ), -BODY+1},  // 7  +1
      {(int)(fSin * (HEAD+0)       ), -BODY-1}     // 8
  };

  #define  FACT 2

  BODY = (int)((double)(BODY+1) * fCos * fCos);

  int DIA = (BODY + PROFIL);

  /* both wings */
  POINT AircraftWing
  [13] = {
      {(int)(fCos * BODY              ) ,  -DIA},    // 1
      {(int)(fCos * (int)( FACT*BODY) ), -PROFIL},    // 2
      {(int)(fCos * WING              ) ,  -PROFIL},    // 3
      {(int)(fCos * WING              ), 0* PROFIL},    // 4
      {(int)(fCos * (int)( FACT*BODY) ) , PROFIL},    // 5
      {(int)(fCos *  BODY             ), DIA},    // 6
      {(int)(fCos * -BODY             ) , DIA},    // 7
      {(int)(fCos * (int)( -FACT*BODY)), PROFIL},    // 8
      {(int)(fCos * -WING             ), 0* PROFIL  },    // 9
      {(int)(fCos * -WING             ) , -PROFIL}  ,    // 10
      {(int)(fCos * (int)( -FACT*BODY)), -PROFIL},    // 11
      {(int)(fCos * -BODY             ) , -DIA},    // 12
      {(int)(fCos *  BODY             ), -DIA}    // 13
  };


  POINT AircraftWingL
  [7] = {

      {(int)(0 * -BODY                ),  DIA       },    // 1
      {(int)(fCos * (int)( -FACT*BODY)),  PROFIL    },    // 2
      {(int)(fCos * -WING             ),  0* PROFIL },    // 3
      {(int)(fCos * -WING             ), -PROFIL    },    // 4
      {(int)(fCos * (int)( -FACT*BODY)), -PROFIL    },    // 5
      {(int)(0 * -BODY                ), -DIA       },    // 6
      {(int)(0 * -BODY                ),  DIA       }     // 7
  };


  POINT AircraftWingR
  [7] = {
      {(int)(0 * BODY                 ) ,  -DIA    },   // 1
      {(int)(fCos * (int)( FACT*BODY) ) , -PROFIL  },   // 2
      {(int)(fCos * WING              ) ,  -PROFIL },   // 3
      {(int)(fCos * WING              ) , 0* PROFIL},   // 4
      {(int)(fCos * (int)( FACT*BODY) ) , PROFIL   },   // 5
      {(int)(0 *  BODY                ) , DIA      },   // 6
      {(int)(0 *  BODY                ) , -DIA     }    // 7
  };



  POINT AircraftTail
  [5] = {
      {(int)(fCos *  TAIL - fSin*TUBE), -FINH},            // 1
      {(int)(fCos *  TAIL - fSin*TUBE), -FINH +PROFIL},    // 2
      {(int)(fCos * -TAIL - fSin*TUBE), -FINH +PROFIL},    // 3
      {(int)(fCos * -TAIL - fSin*TUBE), -FINH },           // 4
      {(int)(fCos *  TAIL - fSin*TUBE), -FINH},            // 5

  };

  Start.x = CalcDistanceCoordinat(fDist,  psDia);
  Start.y = CalcHeightCoordinat(fAltitude, psDia);

  const auto oldPen = Surface.SelectObject(LK_BLACK_PEN);
  const auto oldBrush = Surface.SelectObject(LKBrush_White);

  PolygonRotateShift(AircraftWing, 13,  Start.x, Start.y,  0);
  PolygonRotateShift(AircraftSide, 8,   Start.x, Start.y,  0);
  PolygonRotateShift(AircraftTail, 5,   Start.x, Start.y,  0);
  PolygonRotateShift(AircraftWingL, 7,   Start.x, Start.y,  0);
  PolygonRotateShift(AircraftWingR, 7,   Start.x, Start.y,  0);

  LKColor green = RGB_GREEN;
  LKColor red = RGB_RED;
  if (IsDithered()) {
    green = RGB_WHITE;
    red = RGB_BLACK;
  }
  LKBrush GreenBrush(green);
  LKBrush RedBrush(red);
  if((brg < 180))
  {
    Surface.SelectObject(RedBrush);
    Surface.Polygon(AircraftWingL ,7 );

    Surface.SelectObject(LKBrush_White);
    Surface.Polygon(AircraftSide  ,8 );

    Surface.SelectObject(GreenBrush);
    Surface.Polygon(AircraftWingR ,7 );

    Surface.SelectObject(oldBrush);
  }
  else
  {
    Surface.SelectObject(GreenBrush);
    Surface.Polygon(AircraftWingR ,7 );

    Surface.SelectObject(LKBrush_White);
    Surface.Polygon(AircraftSide  ,8 );

    Surface.SelectObject(RedBrush);
    Surface.Polygon(AircraftWingL ,7 );

    Surface.SelectObject(oldBrush);
  }
  if((brg < 90)|| (brg > 270)) {
    Surface.Polygon(AircraftTail  ,5 );
  }

  Surface.SelectObject(oldPen);
  Surface.SelectObject(oldBrush);
} //else !asp_heading_task
