/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"
#include "RGB.h"


/*
 * The VisualGlide is a sort of Stocker dynamic chart
 *
 * VisualGlide=1 : Steady sector/circle
 *             2 : Moving sector/circle   optional configurable, not much useful.
 */
void MapWindow::DrawGlideCircle(HDC hdc, POINT Orig, RECT rc )
{
  double tmp=0;
  TCHAR gtext[LKSIZEBUFFERLARGE];
  char text[LKSIZETEXT]; 
  double cruise=1;
  int i;
  double gunit;
  COLORREF oldcolor=0;
  HFONT oldfont;

  static double maxcruise;
  static double mincruise;
  static int spread;

  maxcruise=(GlidePolar::bestld); 
  mincruise=(GlidePolar::bestld/4);

  cruise= DerivedDrawInfo.AverageLD; 

  if ( cruise <= 0 ) cruise = GlidePolar::bestld; // 091215 let cruise be always reasonable
  if ( cruise < mincruise ) return;
  if ( cruise >maxcruise ) cruise=maxcruise;


  // Spread from 
  static short turn=1;
  static short count=0;
  spread += (10 * turn); 
  if ( spread <-25 || spread >25 ) turn*=-1;
  if ( ++count >6) count=-1;

  SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
  // SetBkMode(hdc,TRANSPARENT);

  //oldfont = (HFONT)SelectObject(hdc, MapWindowBoldFont); // FIXFONT
  oldfont = (HFONT)SelectObject(hdc, LK8InfoNormalFont); // FIXFONT

  // 100m or 300ft scale
  if ( Units::GetUserAltitudeUnit() == unMeter ) gunit=100; else gunit = 91.44;

  for (i=1; i<9; i++) {

      SelectObject(hdc, hpVisualGlideHeavyBlack); // 091215
#if 0
    // toggling colors 
    if (turn>0 ) {
      if ( (i==2 || i==4 || i==6 || i == 8) ) SelectObject(hdc, hpVisualGlideHeavyRed);
      else SelectObject(hdc, hpVisualGlideLightRed);
    } else {
      if ( (i==2 || i==4 || i==6 || i == 8) ) SelectObject(hdc, hpVisualGlideHeavyBlack);
      else SelectObject(hdc, hpVisualGlideLightBlack);
    }
#endif
#if 0
    if (turn>0 )
		SelectObject(hdc, hpVisualGlideHeavyRed);
    else
		SelectObject(hdc, hpVisualGlideHeavyBlack);
#endif

    /*
     * TRACKUP, NORTHUP, NORTHCIRCLE, TRACKCIRCLE, NORTHTRACK
     */
	if ( ((DisplayOrientation == TRACKUP) || (DisplayOrientation == NORTHCIRCLE) || (DisplayOrientation == TRACKCIRCLE))
           && (!mode.Is(MapWindow::Mode::MODE_CIRCLING)) ) {
		if ( VisualGlide == 1 ) {
			tmp = i*gunit*cruise*zoom.ResScaleOverDistanceModify();
			DrawArc(hdc, Orig.x, Orig.y,(int)tmp, rc, 315, 45);
		} else {
			tmp = i*gunit*cruise*zoom.ResScaleOverDistanceModify();
			DrawArc(hdc, Orig.x, Orig.y,(int)tmp, rc, 330+spread, 30+spread);
		}
	} else {
		tmp = i*gunit*cruise*zoom.ResScaleOverDistanceModify();
		Circle(hdc, Orig.x,Orig.y,(int)tmp, rc, true, false);
	}


	if (turn>0) oldcolor=SetTextColor(hdc, RGB_BLACK); 
	else oldcolor=SetTextColor(hdc, RGB_BLUE); // red

	if ( i==2 || i==4 || i==6 || i==8 ) { 
		if ( Units::GetUserAltitudeUnit() == unMeter ) 
			wsprintf(gtext,_T("-%dm"),i*100); 
		else
			wsprintf(gtext,_T("-%dft"),i*300);

			ExtTextOut( hdc, Orig.x+35, Orig.y-5 - (int) tmp, 0, NULL, gtext , _tcslen(gtext), NULL );
	}

	SetTextColor(hdc,oldcolor);
	if (turn>0)
		oldcolor=SetTextColor(hdc, RGB_BLACK); // dark grey
	else
		oldcolor=SetTextColor(hdc, RGB_BLUE); // red

	if ( i==2 || i==4 || i==6 || i==8 ) {
		if ( Units::GetUserDistanceUnit() == unKiloMeter ) {

			//sprintf(text,"%3.1f Km",i*100*cruise /1000);
			sprintf(text,"%3.0fkm",i*100*cruise /1000);
		} else
			if ( Units::GetUserDistanceUnit() == unNauticalMiles ) {

				//sprintf(text,"%3.1f nmi", i*100*cruise / 1852);
				sprintf(text,"%3.0fnm", i*100*cruise / 1852);
			} else
				if ( Units::GetUserDistanceUnit() == unStatuteMiles ) {
					//sprintf(text,"%3.1f mi", i*100*cruise / 1609);
					sprintf(text,"%3.0fm", i*100*cruise / 1609);
				}

				wsprintf(gtext,_T("%S"),text);; 
					ExtTextOut( hdc, Orig.x-100, Orig.y-5 - (int) tmp, 0, NULL, gtext , _tcslen(gtext), NULL );
	}	
	SetTextColor(hdc,oldcolor);

  }

  SelectObject(hdc, oldfont);
  SetTextColor(hdc,oldcolor);

}

