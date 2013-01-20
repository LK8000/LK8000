/*
  // Size of the box, fixed for each waypoint at this resolution
  unsigned int boxSizeX=0 ,boxSizeY=0;
  int maxtSizeX=0, maxtSizeY=0;
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Multimap.h"
#include "Sideview.h"
#include "LKObjects.h"
#include "RGB.h"


// border margins in boxed text
#define XYMARGIN NIBLSCALE(2) 

// space between two adjacent boxes on the same row
#define BOXINTERVAL NIBLSCALE(2)

// space between row0 and center line
#define CENTERYSPACE NIBLSCALE(1)

// How many boxes on a row, max
#define MAXBSLOT 10

// Size of the box, fixed for each waypoint at this resolution
static unsigned int boxSizeX=0 ,boxSizeY=0;
static int maxtSizeX=0, maxtSizeY=0;
static HFONT line1Font, line2Font;

void MapWindow::DrawVisualGlide(HDC hdc, DiagrammStruct* pDia) {

  unsigned short numboxrows=1;

  #if BUGSTOP
  LKASSERT(Current_Multimap_SizeY<SIZE4);
  #endif
  switch(Current_Multimap_SizeY) {
	case SIZE0:
	case SIZE1:
		numboxrows=3;
		break;
	case SIZE2:
		numboxrows=2;
		break;
	case SIZE3:
		numboxrows=1;
		break;
	case SIZE4:
		return;
	default:
		LKASSERT(0);
		break;
  }

  switch(ScreenSize) {
	case ss800x480:
		line1Font=MapLabelFont;
		line2Font=LK8PanelUnitFont;
		break;
	default:
		line1Font=MapLabelFont;
		line2Font=LK8PanelUnitFont;
		break;
  }


  SIZE textSize;
  TCHAR tmpT[30];
  _tcscpy(tmpT,_T("MMMMMMMMMM"));
  SelectObject(hdc, line1Font);
  GetTextExtentPoint(hdc, tmpT, _tcslen(tmpT), &textSize);
  maxtSizeX=textSize.cx;
  maxtSizeY=textSize.cy;
  boxSizeX=textSize.cx+XYMARGIN+XYMARGIN;
  boxSizeY=(textSize.cy*numboxrows)+XYMARGIN+XYMARGIN;
  //boxSizeY=(textSize.cy*1)+XYMARGIN+XYMARGIN;

  //StartupStore(_T("boxX=%d boxY=%d  \n"),  boxSizeX,boxSizeY);


  RECT vrc;
  vrc.left=0;
  vrc.right=ScreenSizeX;
  vrc.bottom=ScreenSizeY-BottomSize;
  if (Current_Multimap_SizeY==SIZE0) // Full screen?
	vrc.top=0;
  else
	vrc.top=pDia->rc.bottom;
  #if 0
  StartupStore(_T("fxMinMax=%f,%f  fyMinMax=%f,%f rcTLBR=%d,%d %d,%d\n"),
	pDia->fXMin,pDia->fXMax,pDia->fYMin,pDia->fYMax,
	pDia->rc.top, pDia->rc.left, pDia->rc.bottom,pDia->rc.right);
  #endif
  //StartupStore(_T("VG AREA LTRB: %d,%d %d,%d\n"),vrc.left,vrc.top,vrc.right,vrc.bottom);

  HBRUSH oldBrush=(HBRUSH) SelectObject(hdc,GetStockObject(WHITE_BRUSH));
  HPEN   oldPen  =(HPEN)   SelectObject(hdc, GetStockObject(BLACK_PEN));

  HBRUSH brush_back;
  if (!INVERTCOLORS) {
    brush_back = LKBrush_Black;
  } else {
    brush_back = LKBrush_Nlight;
  }

  FillRect(hdc, &vrc, brush_back);

  POINT center, p1, p2;
  center.y=vrc.top+(vrc.bottom-vrc.top)/2;
  center.x=vrc.left+(vrc.right-vrc.left)/2;

  unsigned short numSlotX=(vrc.right-vrc.left)/(boxSizeX+BOXINTERVAL);
  if (numSlotX>MAXBSLOT) numSlotX=MAXBSLOT;

  unsigned short boxInterval=((vrc.right-vrc.left)-(boxSizeX*numSlotX))/(numSlotX+1);

  //StartupStore(_T("numSlotX=%d boxInterval=%d\n"),numSlotX,boxInterval);

  unsigned int t;

  // The horizontal grid
  unsigned int slotCenterX[MAXBSLOT+1];
  for (t=0; t<numSlotX; t++) {
	slotCenterX[t]=(t*boxSizeX) + boxInterval*(t+1)+(boxSizeX/2);
	//StartupStore(_T("slotCenterX[%d]=%d\n"),t,slotCenterX[t]);
  }

  // Vertical coordinates of each up/down subwindow, excluding center line
  int upYtop=vrc.top;
  int upYbottom=center.y-CENTERYSPACE;
  int upSizeY=upYbottom-upYtop;
  int downYtop=center.y+CENTERYSPACE;
  int downYbottom=vrc.bottom;
  int downSizeY=downYbottom-downYtop;

  #if 0
  // Reassign dynamically the vertical scale for each subwindow size
  unsigned int vscale=VSCALE*(100-Current_Multimap_SizeY)/100;
  #else
  // Set the vertical range to 1000 feet, or 300m
  unsigned int vscale;
  if (Units::GetUserAltitudeUnit()==unFeet)
	vscale=(unsigned int)(1000/TOFEET);
  else
	vscale=300;
  #endif



  SetBkMode(hdc,TRANSPARENT);

  RECT trc;
  trc=vrc;

  // Top part of visual rect, target is over us=unreachable=red
  trc.top=vrc.top;
  trc.bottom=center.y-1;
  RenderSky( hdc, trc, RGB_WHITE, RGB(255,150,150) , GC_NO_COLOR_STEPS/2);
  // Bottom part, target is below us=reachable=green
  trc.top=center.y+1;
  trc.bottom=vrc.bottom;
  RenderSky( hdc, trc, RGB(150,255,150) , RGB_WHITE, GC_NO_COLOR_STEPS/2);


  // Draw center line
  p1.x=vrc.left+1; p1.y=center.y;
  p2.x=vrc.right-1; p2.y=center.y;
  SelectObject(hdc, LKPen_Black_N1);
  DrawSolidLine(hdc, p1, p2, vrc);



  SelectObject(hdc, line1Font);
  SelectObject(hdc,LKPen_Black_N0);
  //SelectObject(hdc,LKBrush_LightGreen);

  double altdiff=123;
  int ty,tx;
  int offset=(boxSizeY/2)+CENTERYSPACE;

  // Positive arrival altitude for the waypoint, lower (down) window
  if (altdiff>=0) {
	if (altdiff==0)altdiff=0.5;
	ty=downYtop + (downSizeY/(int)(vscale/altdiff));
	if ((ty-offset)<downYtop) ty=downYtop+offset;
	if ((ty+offset)>downYbottom) ty=downYbottom-offset;
  } else {
	ty=upYbottom + (upSizeY/(int)(vscale/altdiff)); // + because the left part is negative. We are really subtracting
	if ((ty-offset)<upYtop) ty=upYtop+offset;
	if ((ty+offset)>upYbottom) ty=upYbottom-offset;
  }


  for (t=0; t<numSlotX; t++) {
	tx=slotCenterX[t];
	VGTextInBox(hdc,numboxrows,_T("Corni di Canzo"), _T("14.3km  +1523m"),_T("E 21 +1723@mc0"), tx , ty,  RGB_BLACK, LKBrush_LightGreen);
  }




  // Cleanup and return
//_end:
  SelectObject(hdc,oldBrush); 
  SelectObject(hdc,oldPen); 
  return;
}






void MapWindow::VGTextInBox(HDC hDC, short numlines, const TCHAR* wText1, const TCHAR* wText2, const TCHAR *wText3, int x, int y, COLORREF trgb, HBRUSH bbrush) {

  COLORREF oldTextColor=SetTextColor(hDC,trgb);

  SIZE tsize;
  int tx, ty, rowsize;
  SelectObject(hDC, line1Font);
  unsigned int tlen=_tcslen(wText1);
  GetTextExtentPoint(hDC, wText1, tlen, &tsize);
  rowsize=tsize.cy;

  // Fit as many characters in the available boxed space
  if (tsize.cx>maxtSizeX) {
	LKASSERT(tlen>0);
	for (short t=tlen-1; t>0; t--) {
		GetTextExtentPoint(hDC, wText1, t, &tsize);
		if (tsize.cx<=maxtSizeX) {
			tlen=t;
			break;
		}
	}
  }

  short vy=y+(boxSizeY/2);
  SelectObject(hDC,bbrush);
  Rectangle(hDC, 
	x-(boxSizeX/2),
	y-(boxSizeY/2),
	x+(boxSizeX/2),
	vy);

  tx = x-(tsize.cx/2);
  ty = y-(vy-y);

  ExtTextOut(hDC, tx, ty, ETO_OPAQUE, NULL, wText1, tlen, NULL);

  if (numlines==1) goto _end;

  SelectObject(hDC, line2Font);
  tlen=_tcslen(wText2);
  GetTextExtentPoint(hDC, wText2, tlen, &tsize);
  tx = x-(tsize.cx/2);
  ty += rowsize;
  ExtTextOut(hDC, tx, ty, ETO_OPAQUE, NULL, wText2, tlen, NULL);

  if (numlines==2) goto _end;

  tlen=_tcslen(wText3);
  GetTextExtentPoint(hDC, wText3, tlen, &tsize);
  tx = x-(tsize.cx/2);
  ty += rowsize;
  ExtTextOut(hDC, tx, ty, ETO_OPAQUE, NULL, wText3, tlen, NULL);

_end:
  SetTextColor(hDC,oldTextColor);
  return;
}

