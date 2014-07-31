/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"
#include "LKObjects.h"
#include "DoInits.h"



#define NUMVBRICKS 32	// MUST be an even number
#define BOXTHICK   2	// caution, not used as deemed
#define PIXELSEPARATE 1
#define POSCOLOR 6


// Available only in fullscreen landscape mode
void MapWindow::LKDrawVario(HDC hDC, RECT rc) {

  HPEN		oldPen;
  HBRUSH	oldBrush;

  static RECT	vrc, mrc, hrc, htrc, hbrc;
  static RECT	brc[NUMVBRICKS];
  static HPEN	blackThickPen;
  static HPEN	blackThinPen, whiteThinPen;
  static HBRUSH blackBrush, whiteBrush;
  static HBRUSH greenBrush, darkyellowBrush, orangeBrush, redBrush;
  static HBRUSH lakeBrush, blueBrush, indigoBrush;
  static HBRUSH *positiveBrush[NUMVBRICKS/2];
  static HBRUSH *negativeBrush[NUMVBRICKS/2];

  static short startInitCounter=0;
  static bool dogaugeinit=true;
  static double max_positiveGload;
  static double max_negativeGload;

  if (DoInit[MDI_DRAWVARIO]) {

  int boxthick;
  int hpixelseparate;
  int vpixelseparate;

  startInitCounter=0;
  dogaugeinit=true;

  // initial fullscale G loads for 2D driving.
  // These values are then rescaled (only increased) automatically.
  max_positiveGload=0.1;
  max_negativeGload=-0.1;

  // A dirty hack for an impossible division solution
  // lowres devices should have 8 bricks, and not 16 as asked by users!
  switch (ScreenSize) {
	case ss320x240:
	case ss400x240:
		hpixelseparate=0;
		vpixelseparate=-1;
		boxthick=0;
		break;
	case ss480x272:
	case ss480x234:
		hpixelseparate=0;
		vpixelseparate=-2;
		boxthick=0;
		break;
	default:
		hpixelseparate=PIXELSEPARATE;
		vpixelseparate=PIXELSEPARATE;
		boxthick=IBLSCALE(BOXTHICK);
		break;
  }

  int variowidth=LKVarioSize;


  blackThickPen =  LKPen_Black_N2;	// BOXTHICK
  whiteThinPen =   LKPen_White_N0;
  blackThinPen =   LKPen_Black_N0;
  blackBrush = LKBrush_Black;
  whiteBrush = LKBrush_White;
  greenBrush = LKBrush_Green;
  darkyellowBrush = LKBrush_DarkYellow2;
  orangeBrush = LKBrush_Orange;
  redBrush = LKBrush_Red;
  lakeBrush = LKBrush_Lake;
  blueBrush = LKBrush_Blue;
  indigoBrush = LKBrush_Indigo;

  // set default background in case of missing values
  for (int i=0; i<(NUMVBRICKS/2); i++ )
	positiveBrush[i]= &blackBrush;
  for (int i=0; i<(NUMVBRICKS/2); i++ )
	negativeBrush[i]= &blackBrush;

  positiveBrush[15]=&greenBrush;
  positiveBrush[14]=&greenBrush;
  positiveBrush[13]=&greenBrush;
  positiveBrush[12]=&greenBrush;
  positiveBrush[11]=&darkyellowBrush;
  positiveBrush[10]=&darkyellowBrush;
  positiveBrush[9]=&darkyellowBrush;
  positiveBrush[8]=&darkyellowBrush;
  positiveBrush[7]=&orangeBrush;
  positiveBrush[6]=&orangeBrush;
  positiveBrush[5]=&orangeBrush;
  positiveBrush[4]=&orangeBrush;
  positiveBrush[3]=&redBrush;
  positiveBrush[2]=&redBrush;
  positiveBrush[1]=&redBrush;
  positiveBrush[0]=&redBrush;

  negativeBrush[0]=&lakeBrush;
  negativeBrush[1]=&lakeBrush;
  negativeBrush[2]=&lakeBrush;
  negativeBrush[3]=&lakeBrush;
  negativeBrush[4]=&blueBrush;
  negativeBrush[5]=&blueBrush;
  negativeBrush[6]=&blueBrush;
  negativeBrush[7]=&blueBrush;
  negativeBrush[8]=&indigoBrush;
  negativeBrush[9]=&indigoBrush;
  negativeBrush[10]=&indigoBrush;
  negativeBrush[11]=&indigoBrush;
  negativeBrush[12]=&blackBrush;
  negativeBrush[13]=&blackBrush;
  negativeBrush[14]=&blackBrush;
  negativeBrush[15]=&blackBrush;


  // vario paint area
  vrc.left=rc.left+NIBLSCALE(1);
  vrc.top=rc.top+NIBLSCALE(1);
  vrc.right=vrc.left+variowidth;
  vrc.bottom=rc.bottom - BottomSize - NIBLSCALE(1);;

  // meter area
  mrc.left=vrc.left+boxthick-hpixelseparate;
  mrc.top=vrc.top+boxthick-vpixelseparate;;
  mrc.right=vrc.right-boxthick;
  mrc.bottom=vrc.bottom-boxthick;

  int vmiddle=((mrc.bottom-mrc.top)/2)+mrc.top;

  // half vario separator for positive and negative values
  hrc.top = vrc.top+ vmiddle-NIBLSCALE(1);
  hrc.bottom = vrc.top+ vmiddle+NIBLSCALE(1);
  hrc.left= vrc.left;
  // MUST MATCH MapWindow DrawLook8000 leftmargin!!
  hrc.right=vrc.right+NIBLSCALE(2);

  // half top meter area
  htrc.left=mrc.left;
  htrc.right=mrc.right;
  htrc.top=mrc.top;

  switch (ScreenSize) {
	case ss320x240:
	case ss480x234:
	case ss480x272:
		htrc.bottom=hrc.top -vpixelseparate;
		hbrc.top=hrc.bottom+vpixelseparate;
		break;
	default:
		htrc.bottom=hrc.top -vpixelseparate-1;
		hbrc.top=hrc.bottom+vpixelseparate+1;
		break;
  }

  // half bottom meter area
  hbrc.left=mrc.left;
  hbrc.right=mrc.right;
  hbrc.bottom=mrc.bottom;

  // pixel height of each brick
  int bricksize=(htrc.bottom - htrc.top - ((vpixelseparate) * ((NUMVBRICKS/2)-1)  )) / (NUMVBRICKS/2);
#if (WINDOWSPC>0)
  if (ScreenSize==ss720x408) bricksize=13;
#endif
  if (ScreenSize==ss480x272) bricksize=9;
  if (ScreenSize==ss480x234) bricksize=8;
 
  // Pre-calculate brick positions for half top
  for (int i=0; i<(NUMVBRICKS/2); i++) {
	brc[i].top= htrc.top + (bricksize*i)+(i*(vpixelseparate));
	// make the last one rounded since bricksize could be slighlty smaller due to division round
	if (i==((NUMVBRICKS/2)-1))
		brc[i].bottom= htrc.bottom;
	else
		brc[i].bottom= brc[i].top+bricksize;
	brc[i].left= htrc.left;
	brc[i].right= htrc.right;
  }
  // Pre-calculate brick positions for half bottom
  for (int i=((NUMVBRICKS/2)-1); i>=0; i--) {
	brc[ (NUMVBRICKS/2)+ i].bottom= hbrc.bottom - (bricksize*(  ((NUMVBRICKS/2)-1)-i)  ) - 
		(  (((NUMVBRICKS/2)-1)-i) * vpixelseparate   );
	if ( i == 0 )
		brc[ (NUMVBRICKS/2)+ i].top = hbrc.top;
	else
		brc[ (NUMVBRICKS/2)+ i].top = brc[ (NUMVBRICKS/2)+i].bottom - bricksize;
	brc[ (NUMVBRICKS/2)+ i].left = hbrc.left;
	brc[ (NUMVBRICKS/2)+ i].right = hbrc.right;
  }


	DoInit[MDI_DRAWVARIO]=false;
  } // END of INIT




  // draw external box
  if (BgMapColor>POSCOLOR) 
	oldPen=(HPEN)SelectObject(hDC,whiteThinPen);
  else
	oldPen=(HPEN)SelectObject(hDC,blackThickPen);

  if (LKVarioBar>vBarVarioGR) {
	oldBrush=(HBRUSH)SelectObject(hDC,GetStockObject(NULL_BRUSH));
  } else {
	oldBrush=(HBRUSH)SelectObject(hDC,hInvBackgroundBrush[BgMapColor]);
  	Rectangle(hDC,vrc.left, vrc.top, vrc.right, vrc.bottom);
  }


  // draw middle separator for 0 scale indicator
  if (BgMapColor>POSCOLOR) 
  	FillRect(hDC,&hrc, whiteBrush);
  else
  	FillRect(hDC,&hrc, blackBrush);

  if (BgMapColor>POSCOLOR)
	  SelectObject(hDC,whiteThinPen);
  else
	  SelectObject(hDC,blackThinPen);

  double value=0;

  if (ISCAR && DrawInfo.Speed>0) {
	// Heading is setting Gload, but Heading is not calculated while steady!
	// For this case, we force value to 0.
	//
	// Since we use currently a scale 0-6 for vario, we can use 0-2 for cars.
	// This accounts for an acceleration topscale of 0-100kmh in 13.9 seconds.
	// Not a big acceleration, but very good for normal car usage.
	// We make this concept dynamical, and different for positive and negative accelerations.
	// Because negative accelerations are much higher, on a car. Of course!
	//
	if (DerivedDrawInfo.Gload>0) {
		if (DerivedDrawInfo.Gload>max_positiveGload) {
			max_positiveGload=DerivedDrawInfo.Gload;
			StartupStore(_T("..... NEW MAXPOSITIVE G=%f\n"),max_positiveGload);
		}
		LKASSERT(max_positiveGload>0);
		value = (DerivedDrawInfo.Gload/max_positiveGload)*6;
		//StartupStore(_T("Speed=%f G=%f max=%f val=%f\n"),DrawInfo.Speed, DerivedDrawInfo.Gload, max_positiveGload,value);
	}
	if (DerivedDrawInfo.Gload<0) {
		if (DerivedDrawInfo.Gload<max_negativeGload) {
			max_negativeGload=DerivedDrawInfo.Gload;
			StartupStore(_T("..... NEW MAXNEGATIVE G=%f\n"),max_negativeGload);
		}
		LKASSERT(max_negativeGload<0);
		value = (DerivedDrawInfo.Gload/max_negativeGload)*-6;
		//StartupStore(_T("Speed=%f G=%f max=%f val=%f\n"),DrawInfo.Speed, DerivedDrawInfo.Gload, max_negativeGload,value);
	}

	goto _aftercar;
  }

  if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING) || LKVarioVal==vValVarioVario) {
	if (DrawInfo.VarioAvailable) {
		// UHM. I think we are not painting values correctly for knots &c.
		//value = LIFTMODIFY*DrawInfo.Vario;
		value = DrawInfo.Vario;
	} else {
		value = DerivedDrawInfo.Vario;
	}
  } else {
	switch(LKVarioVal) {
		case vValVarioNetto:
			value = DerivedDrawInfo.NettoVario;
			break;
		case vValVarioSoll:
			double ias;
			if (DrawInfo.AirspeedAvailable && DrawInfo.VarioAvailable)
				ias=DrawInfo.IndicatedAirspeed;
			else
				ias=DerivedDrawInfo.IndicatedAirspeedEstimated;

			value = DerivedDrawInfo.VOpt - ias;
			// m/s 0-nnn autolimit to 20m/s full scale (72kmh diff)
			if (value>20) value=20;
			if (value<-20) value=-20;
			value/=3.3333;	// 0-20  -> 0-6
			value *= -1; // if up, push down
			break;

		default:
			value = DerivedDrawInfo.NettoVario;
			break;
	}
  }

_aftercar:

  if (dogaugeinit) {

	// this is causing problems on emulators and condor and most of the times when the gps has no valid date
	// so we don't use seconds, but loop counter
	if ( startInitCounter++ >2) {
		dogaugeinit=false;
	} else {
		short j=NUMVBRICKS/2;
		// Demo show all bricks
		for (int i=0; i<j; i++)  {
			SelectObject(hDC,*positiveBrush[i]);
			Rectangle(hDC,brc[i].left, brc[i].top, brc[i].right, brc[i].bottom);
		}
		for (int i=0; i<j; i++)  {
			SelectObject(hDC,*negativeBrush[i]);
			Rectangle(hDC,brc[i+(NUMVBRICKS/2)].left, brc[i+(NUMVBRICKS/2)].top, brc[i+(NUMVBRICKS/2)].right, brc[i+(NUMVBRICKS/2)].bottom);
		}
		
		SelectObject(hDC,oldPen);
		SelectObject(hDC,oldBrush);
		return;

	}
  }

  short lkvariobar=LKVarioBar;
  if (lkvariobar>vBarVarioGR) lkvariobar-=vBarVarioGR;
  short meter=-1;
  if (value>0) {

	if (value>=0.05) meter=15;
	if (value>=0.25) meter=14;
	if (value>=0.50) meter=13;
	if (value>=0.75) meter=12;
	if (value>=1.00) meter=11;
	if (value>=1.25) meter=10;
	if (value>=1.50) meter=9;
	if (value>=1.75) meter=8;
	if (value>=2.00) meter=7;
	if (value>=2.50) meter=6;
	if (value>=3.00) meter=5;
	if (value>=3.50) meter=4;
	if (value>=4.00) meter=3;
	if (value>=4.50) meter=2;
	if (value>=5.00) meter=1;
	if (value>=6.00) meter=0;

	if (meter>=0) {
		for (unsigned short i=15; i>=meter && i<NUMVBRICKS; i--) {
			switch (lkvariobar) {
				case vBarVarioColor:
					SelectObject(hDC,*positiveBrush[i]);
					break;
				case vBarVarioMono:
					if (BgMapColor>POSCOLOR)
						SelectObject(hDC,whiteBrush);
					else
						SelectObject(hDC,blackBrush);
					break;
				case vBarVarioRB:
					SelectObject(hDC,redBrush);
					break;
				case vBarVarioGR:
				default:
					SelectObject(hDC,greenBrush);
					break;
			}
/*
			if (LKVarioBar == vBarVarioColor) 
				SelectObject(hDC,*positiveBrush[i]);
			else {
				if (BgMapColor>POSCOLOR)
					SelectObject(hDC,whiteBrush);
				else
					SelectObject(hDC,blackBrush);
			}
*/
			Rectangle(hDC,brc[i].left, brc[i].top, brc[i].right, brc[i].bottom);
		}
	}
  } else if (value <0) {
	value*=-1;
	if (value>=0.05) meter=0;
	if (value>=0.25) meter=1;
	if (value>=0.50) meter=2;
	if (value>=0.75) meter=3;
	if (value>=1.00) meter=4;
	if (value>=1.25) meter=5;
	if (value>=1.50) meter=6;
	if (value>=1.75) meter=7;
	if (value>=2.00) meter=8;
	if (value>=2.50) meter=9;
	if (value>=3.00) meter=10;
	if (value>=3.50) meter=11;
	if (value>=4.00) meter=12;
	if (value>=4.50) meter=13;
	if (value>=5.00) meter=14;
	if (value>=6.00) meter=15;

	if (meter>=0) {
		for (unsigned short i=0; i<=meter && i<(NUMVBRICKS/2); i++) {
			switch (lkvariobar) {
				case vBarVarioColor:
					SelectObject(hDC,*negativeBrush[i]);
					break;
				case vBarVarioMono:
					if (BgMapColor>POSCOLOR)
						SelectObject(hDC,whiteBrush);
					else
						SelectObject(hDC,blackBrush);
					break;
				case vBarVarioRB:
					SelectObject(hDC,blueBrush);
					break;
				case vBarVarioGR:
				default:
					SelectObject(hDC,redBrush);
					break;
			}
/*
			if (LKVarioBar == vBarVarioColor) 
				SelectObject(hDC,*negativeBrush[i]);
			else {
				if (BgMapColor>POSCOLOR)
					SelectObject(hDC,whiteBrush);
				else
					SelectObject(hDC,blackBrush);
			}
*/
			Rectangle(hDC,brc[i+(NUMVBRICKS/2)].left, brc[i+(NUMVBRICKS/2)].top, brc[i+(NUMVBRICKS/2)].right, brc[i+(NUMVBRICKS/2)].bottom);
		}
	}

  }
	
  // cleanup and return 
  SelectObject(hDC,oldPen);
  SelectObject(hDC,oldBrush);
  return;

}


