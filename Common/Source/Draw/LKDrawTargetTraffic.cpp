/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RGB.h"
#include "DoInits.h"


// This is the 4.x Sight Traffic page aka LK F104 mode
// Remember that when we are painting DrawTarget there is no
// map to be calculated, and we have plenty of CPU to spend.
// On a 400Mhz PNA, only 8% cpu is used by LK in this modality.
//
void MapWindow::DrawTarget(HDC hDC, const RECT rc, int ttop, int tbottom, int tleft, int tright)
{

  HPEN   hp, hpOld;
  HBRUSH hb, hbOld;

  bool disabled=false,notraffic=false;

  static POINT cross_top, cross_bottom, cross_left, cross_right;
  static POINT degline_top[10];
  static POINT degline_bottom[10];
  static POINT altline_left[6];
  static POINT altline_right[6];


  static int nleft,nright,ntop,nbottom;
  static int ncenterx, ncentery;
  if (DoInit[MDI_DRAWTARGET]) {

	nleft=tleft+IBLSCALE(3);
	nright=tright;
	ntop=ttop;
	nbottom=tbottom;
	ncenterx=((nright-nleft)/2)+nleft;
	ncentery=((nbottom-ntop)/2)+ntop;

	cross_top.x=ncenterx;
	cross_top.y=ntop;
	cross_bottom.x=ncenterx;
	cross_bottom.y=nbottom;
	cross_left.x=nleft;
	cross_left.y=ncentery;
	cross_right.x=nright;
	cross_right.y=ncentery;

	// hoffset is the position of vertical green lines on the horizon, each one representing
	// 20 degrees shift. The [0] and [5] are for 10 to 20 degrees target's offset
	// The 9th line on extreme position is not drawn
	int deg10=(nleft-ncenterx)/9; 
	// height of the vertical line
	int hdeg=(ncentery-ntop)/7;

	degline_top[0].x		=	ncenterx-deg10;
	degline_top[0].y		=	ncentery-hdeg;
	degline_bottom[0].x		=	ncenterx-deg10;
	degline_bottom[0].y		=	ncentery+hdeg;
	// right side
	degline_top[5].x		=	ncenterx+deg10;
	degline_top[5].y		=	ncentery-hdeg;
	degline_bottom[5].x		=	ncenterx+deg10;
	degline_bottom[5].y		=	ncentery+hdeg;

	degline_top[1].x		=	ncenterx-(deg10*2);
	degline_top[1].y		=	ncentery-hdeg;
	degline_bottom[1].x		=	ncenterx-(deg10*2);
	degline_bottom[1].y		=	ncentery+hdeg;
	degline_top[6].x		=	ncenterx+(deg10*2);
	degline_top[6].y		=	ncentery-hdeg;
	degline_bottom[6].x		=	ncenterx+(deg10*2);
	degline_bottom[6].y		=	ncentery+hdeg;
	
	degline_top[2].x		=	ncenterx-(deg10*4);
	degline_top[2].y		=	ncentery-hdeg;
	degline_bottom[2].x		=	ncenterx-(deg10*4);
	degline_bottom[2].y		=	ncentery+hdeg;
	degline_top[7].x		=	ncenterx+(deg10*4);
	degline_top[7].y		=	ncentery-hdeg;
	degline_bottom[7].x		=	ncenterx+(deg10*4);
	degline_bottom[7].y		=	ncentery+hdeg;

	degline_top[3].x		=	ncenterx-(deg10*6);
	degline_top[3].y		=	ncentery-hdeg;
	degline_bottom[3].x		=	ncenterx-(deg10*6);
	degline_bottom[3].y		=	ncentery+hdeg;
	degline_top[8].x		=	ncenterx+(deg10*6);
	degline_top[8].y		=	ncentery-hdeg;
	degline_bottom[8].x		=	ncenterx+(deg10*6);
	degline_bottom[8].y		=	ncentery+hdeg;

	degline_top[4].x		=	ncenterx-(deg10*8);
	degline_top[4].y		=	ncentery-hdeg;
	degline_bottom[4].x		=	ncenterx-(deg10*8);
	degline_bottom[4].y		=	ncentery+hdeg;
	degline_top[9].x		=	ncenterx+(deg10*8);
	degline_top[9].y		=	ncentery-hdeg;
	degline_bottom[9].x		=	ncenterx+(deg10*8);
	degline_bottom[9].y		=	ncentery+hdeg;


	// sizes of horizontal altitudes lines
	int alth=(int)((ncentery-ntop)/3.5);
	int altw=(int)((ncenterx-nleft)/3.5);

	altline_left[0].x		=	ncenterx-altw;
	altline_left[0].y		=	ncentery-alth;
	altline_right[0].x		=	ncenterx+altw;
	altline_right[0].y		=	ncentery-alth;

	altline_left[1].x		=	ncenterx-altw;
	altline_left[1].y		=	ncentery-(alth*2);
	altline_right[1].x		=	ncenterx+altw;
	altline_right[1].y		=	ncentery-(alth*2);

	altline_left[2].x		=	ncenterx-altw;
	altline_left[2].y		=	ncentery-(alth*3);
	altline_right[2].x		=	ncenterx+altw;
	altline_right[2].y		=	ncentery-(alth*3);


	altline_left[3].x		=	ncenterx-altw;
	altline_left[3].y		=	ncentery+alth;
	altline_right[3].x		=	ncenterx+altw;
	altline_right[3].y		=	ncentery+alth;

	altline_left[4].x		=	ncenterx-altw;
	altline_left[4].y		=	ncentery+(alth*2);
	altline_right[4].x		=	ncenterx+altw;
	altline_right[4].y		=	ncentery+(alth*2);

	altline_left[5].x		=	ncenterx-altw;
	altline_left[5].y		=	ncentery+(alth*3);
	altline_right[5].x		=	ncenterx+altw;
	altline_right[5].y		=	ncentery+(alth*3);

	DoInit[MDI_DRAWTARGET]=false;
  } 

  // The flag "disabled" will force no plane to be painted

  // Check target exists, just for safe
  if (LKTargetIndex>=0 && LKTargetIndex<MAXTRAFFIC) {
	if (!DrawInfo.FLARM_Traffic[LKTargetIndex].Locked) {
		disabled=true;
		notraffic=true;
	}
  } else {
	disabled=true;
	notraffic=true;
  }

  // check visibility +-80 degrees
  double tangle = LKTraffic[LKTargetIndex].Bearing -  DrawInfo.TrackBearing;
  if (tangle < -180.0) {
	tangle += 360.0;
  } else {
	if (tangle > 180.0)
		tangle -= 360.0;
  }

  if (tangle<-80 || tangle >80) {
	disabled=true;
  }

  COLORREF hscalecol, vscalecol;
  // First we draw the cross sight
  if (disabled) {
	if (notraffic) {
		if (Appearance.InverseInfoBox) {
			_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_left, cross_right, RGB_GREY,rc);
			_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_top, cross_bottom, RGB_GREY,rc);
		} else {
			_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_left, cross_right, RGB_DARKGREY,rc);
			_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_top, cross_bottom, RGB_DARKGREY,rc);
		}
		hscalecol=RGB_DARKGREEN;
		vscalecol=RGB_DARKGREEN;
	} else {
		_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_left, cross_right, RGB_DARKGREY,rc);
		_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_top, cross_bottom, RGB_DARKGREY,rc);
		hscalecol=RGB_DARKGREEN;
		vscalecol=RGB_DARKGREEN;
	}
  } else {
	if (Appearance.InverseInfoBox) {
		_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_left, cross_right, RGB_ICEWHITE,rc);
		_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_top, cross_bottom, RGB_ICEWHITE,rc);
		hscalecol=RGB_GREEN;
		vscalecol=RGB_GREEN;
	} else {
		_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_left, cross_right, RGB_DARKGREY,rc);
		_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_top, cross_bottom, RGB_DARKGREY,rc);
		hscalecol=RGB_DARKGREEN;
		vscalecol=RGB_DARKGREEN;
	}
  }


  // Then we draw the scales, degrees on horizontal line
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[0], degline_bottom[0], hscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[5], degline_bottom[5], hscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[1], degline_bottom[1], hscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[6], degline_bottom[6], hscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[2], degline_bottom[2], hscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[7], degline_bottom[7], hscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[3], degline_bottom[3], hscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[8], degline_bottom[8], hscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[4], degline_bottom[4], hscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[9], degline_bottom[9], hscalecol,rc);
  // altitudes on vertical line
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), altline_left[0], altline_right[0], vscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), altline_left[1], altline_right[1], vscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), altline_left[2], altline_right[2], vscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), altline_left[3], altline_right[3], vscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), altline_left[4], altline_right[4], vscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), altline_left[5], altline_right[5], vscalecol,rc);



  // If out of range, paint diff bearing
  TCHAR tbear[10];
  if (disabled && !notraffic) {
	if (tangle > 1) {
		_stprintf(tbear, TEXT("%2.0f°»"), tangle);
	} else {
		if (tangle < -1) {
			_stprintf(tbear, TEXT("«%2.0f°"), -tangle);
		} else {
			_tcscpy(tbear, TEXT("«»"));
		}
	}
	SelectObject(hDC, LK8PanelBigFont);
	switch ( LKTraffic[LKTargetIndex].Status ) {
		case LKT_GHOST:
			LKWriteText(hDC, tbear, ncenterx,ncentery, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_LIGHTYELLOW, false);
			break;
		case LKT_ZOMBIE:
			LKWriteText(hDC, tbear, ncenterx,ncentery, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_LIGHTRED, false);
			break;
		default:
			LKWriteText(hDC, tbear, ncenterx,ncentery, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_WHITE, false);
			break;
	}

	// do not paint bearing, it is confusing
	#if 0
	double tbearing = LKTraffic[LKTargetIndex].Bearing;
	if (tbearing != 360) {
		_stprintf(tbear, TEXT("%2.0f°"), tbearing);
	} else {
		_stprintf(tbear, TEXT("0°"));
	}
	LKWriteText(hDC, tbear, ncenterx,ncentery, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_WHITE, false);
	#endif
  }


  // Target wing size, half of it
  #define TWINGSIZE	NIBLSCALE(53)
  POINT tcenter;
  
	int tailsize= (TWINGSIZE/4) +NIBLSCALE(2);
    tcenter.x= (int)(ncenterx+(((ncenterx-nleft)/80)*tangle));

    if ( LKTraffic[LKTargetIndex].AltArriv >300 ) {
        tcenter.y=nbottom;
    } else {
        if ( LKTraffic[LKTargetIndex].AltArriv <-300 ) {
            tcenter.y=ntop+IBLSCALE(5);
            tailsize=IBLSCALE(5);
        } else {
            tcenter.y=ncentery+ (int) (((ncentery-ntop)/300.0)*LKTraffic[LKTargetIndex].AltArriv);
        }
    }
  
  // Paint the airplane only if within 160 deg sight angle
  if (!disabled) {

	// Position of the glider on the sight screen
	int leftwingsize=0, rightwingsize=0;
	COLORREF planecolor;

	if (Appearance.InverseInfoBox) {
		switch(LKTraffic[LKTargetIndex].Status) {
			case LKT_GHOST:
				planecolor=RGB_LIGHTYELLOW;
				break;
			case LKT_ZOMBIE:
				planecolor=RGB_LIGHTRED;
				break;
			default:
				planecolor=RGB_WHITE;
				break;
		}
	} else {
		switch(LKTraffic[LKTargetIndex].Status) {
			case LKT_GHOST:
				planecolor=RGB_ORANGE;
				break;
			case LKT_ZOMBIE:
				planecolor=RGB_RED;
				break;
			default:
				planecolor=RGB_BLACK;
				break;
		}
	}

	hp = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), planecolor);
	hb = (HBRUSH)CreateSolidBrush( planecolor);
	hpOld = (HPEN) SelectObject(hDC, hp);
	hbOld = (HBRUSH) SelectObject(hDC, hb);

	// does the glider exceed screen space on the right?
	if (tangle>1) {
		leftwingsize=TWINGSIZE;
		// if right wing is exceeding space, reduce it
		if ( (tcenter.x+TWINGSIZE) >nright ) {
			rightwingsize=  nright-tcenter.x;
		} else
			rightwingsize=TWINGSIZE;
	}

	// Now check the left wing
	if (tangle<1) {
		rightwingsize=TWINGSIZE;
		if ( (tcenter.x-TWINGSIZE) < nleft ) {
			leftwingsize=  tcenter.x-nleft;
		} else
			leftwingsize=TWINGSIZE;
	}

	if (tangle==0) {
		rightwingsize=TWINGSIZE;
		leftwingsize=TWINGSIZE;
	}

	Circle(hDC, tcenter.x, tcenter.y, NIBLSCALE(6), rc, false, true );

	POINT a1, a2;

	// Draw the wing
	a1.x = tcenter.x - leftwingsize;
	a1.y = tcenter.y;
	a2.x = tcenter.x + rightwingsize;
	a2.y = tcenter.y;
	_DrawLine(hDC, PS_SOLID, NIBLSCALE(4), a1, a2, planecolor,rc);
	// Draw the tail
	a1.x = tcenter.x;
	a1.y = tcenter.y;
	a2.x = tcenter.x;
	a2.y = tcenter.y - tailsize;
	_DrawLine(hDC, PS_SOLID, NIBLSCALE(4), a1, a2, planecolor,rc);

	SelectObject(hDC, hbOld);
	SelectObject(hDC, hpOld);
	DeleteObject((HPEN)hp);
	DeleteObject((HBRUSH)hb);
  }
  
  // always paint the bearing difference, cleverly
  if (!disabled && !notraffic) {
	if (tangle > 1) {
		_stprintf(tbear, TEXT("%2.0f°»"), tangle);
	} else {
		if (tangle < -1) {
			_stprintf(tbear, TEXT("«%2.0f°"), -tangle);
		} else {
			_tcscpy(tbear, TEXT("«»"));
		}
	}
	SelectObject(hDC, LK8PanelBigFont);
	// if target is below middle line, paint on top
	int yposbear;
	if (tcenter.y >= ncentery ) {
		yposbear=altline_left[2].y;
	} else
		yposbear=altline_left[5].y;

//	LKWriteText(hDC, tbear, ncenterx,altline_left[5].y, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_WHITE, false);
	switch ( LKTraffic[LKTargetIndex].Status ) {
		case LKT_GHOST:
			//LKWriteText(hDC, tbear, ncenterx,ncentery, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_LIGHTYELLOW, false);
			LKWriteText(hDC, tbear, ncenterx,yposbear, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_LIGHTYELLOW, false);
			break;
		case LKT_ZOMBIE:
			//LKWriteText(hDC, tbear, ncenterx,ncentery, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_LIGHTRED, false);
			LKWriteText(hDC, tbear, ncenterx,yposbear, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_LIGHTRED, false);
			break;
		default:
			//LKWriteText(hDC, tbear, ncenterx,ncentery, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_WHITE, false);
			LKWriteText(hDC, tbear, ncenterx,yposbear, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_WHITE, false);
			break;
	}
  }

}

