/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   DrawHSI.cpp
 * Author: Alberto Realis-Luc
 *
 * Created on 5 January 2014, 10:43
 */

//INFO: This is how to print the degree symbol:
//#ifndef __MINGW32__
//	//_stprintf(Buffer, TEXT("%2.0f\xB0"), beta);
//#else
//	//_stprintf(Buffer, TEXT("%2.0f°"), beta);
//#endif


#include "externs.h"
#include "LKMapWindow.h"
#include "LKObjects.h"
#include "RGB.h"
#include "DoInits.h"

struct compassMark {
	short extX, extY; //coordinates external point
	short intX, intY; //coordinates internal point
};

void MapWindow::DrawHSI(HDC hDC, const RECT rc) {

	static short top = (((rc.bottom - BottomSize - (rc.top + TOPLIMITER)
			- BOTTOMLIMITER) / PANELROWS) + rc.top + TOPLIMITER)
			- (rc.top + TOPLIMITER);

	HPEN hpBlack;
	HBRUSH hbBlack;
	//HPEN hpWhite;
	//HBRUSH hbWhite;
	//HPEN hpBorder;
	//HBRUSH hbBorder;
	HPEN hpOld;
	HBRUSH hbOld;

	static const int radius = NIBLSCALE(80); // gauge size radius
	static const int innerradius = radius - NIBLSCALE(10);
	static const int labelsRadius = radius -NIBLSCALE(20);
	static const int smallMarkRadius = radius - NIBLSCALE(6);
	//TCHAR Buffer[LKSIZEBUFFERVALUE];
	static struct compassMark compassMarks[72][10];
	static const TCHAR* label[]= { //labels of the compass rose
			TEXT("N"),
			TEXT("03"),
			TEXT("06"),
			TEXT("E"),
			TEXT("12"),
			TEXT("15"),
			TEXT("S"),
			TEXT("21"),
			TEXT("24"),
			TEXT("W"),
			TEXT("30"),
			TEXT("33")
	};

	static const short centerX=(rc.right-rc.left)/2;
	static const short centerY=((rc.bottom-BottomSize-top)/2)+top-NIBLSCALE(10);

	static int angle;

	if(DoInit[MDI_DRAWHSI]) { //for the init there are 10 possible cases that must be precomputed
		for(int alpha=0;alpha<10;alpha++) for(int i=0, deg=0, isBig=1; i<72; i++, deg=deg+5, isBig=!isBig) {
			compassMarks[i][alpha].extX=centerX+(short)(radius*fastsine(deg+alpha));
			compassMarks[i][alpha].extY=centerY-(short)(radius*fastcosine(deg+alpha));
			compassMarks[i][alpha].intX=centerX+(short)((isBig?innerradius:smallMarkRadius)*fastsine(deg+alpha));
			compassMarks[i][alpha].intY=centerY-(short)((isBig?innerradius:smallMarkRadius)*fastcosine(deg+alpha));
		}
		DoInit[MDI_DRAWHSI] = false;
	}

	//hpBlack = LKPen_Grey_N1;
	//hbBlack = LKBrush_Grey;

	hpBlack = LKPen_Black_N1;
	hbBlack = LKBrush_Black;
	//hpWhite = LKPen_White_N1;
	//hbWhite = LKBrush_White; //test this was the original
	//hpBorder = LKPen_Grey_N2;
	//hbBorder = LKBrush_Grey;

	//SelectObject(hDC, hpBorder);
	//SelectObject(hDC, hbBorder);
	//Circle(hDC, Start.x, Start.y, radius + NIBLSCALE(2), rc, false, false);

	//SelectObject(hDC, hpBlack);
	//SelectObject(hDC, hbBlack);

	hpOld = (HPEN) SelectObject(hDC, hpBlack);
	hbOld = (HBRUSH) SelectObject(hDC, hbBlack);
	//Circle(hDC, centerX, centerY, radius, rc, false, true);

	//get the track bearing
	angle = 360-round(DrawInfo.TrackBearing);


	//Draw the markers
	int alpha=angle%10;
	POINT external,internal;
	for(int i=0;i<72;i++) {
		external.x=compassMarks[i][alpha].extX;
		external.y=compassMarks[i][alpha].extY;
		internal.x=compassMarks[i][alpha].intX;
		internal.y=compassMarks[i][alpha].intY;
		_DrawLine(hDC, PS_SOLID, NIBLSCALE(1),external,internal,RGB_WHITE,rc);
	}

	//Put the labels
	SelectObject(hDC, LK8TitleFont);
	for(int i=0;i<12;i++) {
		int deg=i*30+angle;
		int x=centerX+(int)(labelsRadius*fastsine(deg));
		int y=centerY-(int)(labelsRadius*fastcosine(deg));
		LKWriteText(hDC,label[i],x,y,0, WTMODE_NORMAL,WTALIGN_CENTER,RGB_WHITE,false);
	}


	SelectObject(hDC, hbOld);
	SelectObject(hDC, hpOld);
}
