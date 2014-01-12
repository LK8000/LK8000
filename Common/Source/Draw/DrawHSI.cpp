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

void MapWindow::DrawHSI(HDC hDC, const RECT rc) {
	static short centerX;
	static short centerY;
	static short radius; //HSI gauge size radius
	static short innerradius;
	static short labelsRadius;
	static short smallMarkRadius;
	static short cdiRadius;
	static short cdiFullScale;
	static POINT fusA,fusB,winA,winB,taiA,taiB; //coordinates airplane symbol
	//TCHAR Buffer[LKSIZEBUFFERVALUE];

	static struct { //Compass rose marks coordinates matrix: using short's to use less memory
		short extX, extY; //coordinates external mark point
		short intX, intY; //coordinates internal mark point
	} compassMarks[72][10]; //72 compass marks (one every 5 degrees), 10 possible cases
	static POINT hdgMark[4]; //Coordinates of heading marker (red triangle on the top of compass rose)

	static const int bigCDIscale=9260; //Large Course Deviation Indicator scale: 5 nautical miles (9260 m)
	static const int smallCDIscale=557; //Narrow (zoomed) CDI scale: 0.3 nautical miles (557 m)

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

	static bool screenModeNormal=true;

	if(DoInit[MDI_DRAWHSI]) {
		//all the sizes must be recalculated in case of screen resolution change:
		short top=(((rc.bottom-BottomSize-(rc.top + TOPLIMITER)-BOTTOMLIMITER)/PANELROWS)+rc.top+TOPLIMITER)-(rc.top + TOPLIMITER);
		centerX=(rc.right-rc.left)/2;
		centerY=((rc.bottom-BottomSize-top)/2)+top-NIBLSCALE(12);
		radius = NIBLSCALE(80);
		innerradius = radius - NIBLSCALE(10);
		labelsRadius = radius -NIBLSCALE(20);
		smallMarkRadius = radius - NIBLSCALE(6);
		cdiRadius = radius - NIBLSCALE(35);
		cdiFullScale = radius -NIBLSCALE(30);

		//For the positions of all 72 compass rose marks there are 10 possible cases to be pre-calculated:
		for(int alpha=0;alpha<10;alpha++) for(int i=0, deg=0, isBig=1; i<72; i++, deg+=5, isBig=!isBig) {
			compassMarks[i][alpha].extX=centerX+radius*fastsine(deg+alpha);
			compassMarks[i][alpha].extY=centerY-radius*fastcosine(deg+alpha);
			compassMarks[i][alpha].intX=centerX+(isBig?innerradius:smallMarkRadius)*fastsine(deg+alpha);
			compassMarks[i][alpha].intY=centerY-(isBig?innerradius:smallMarkRadius)*fastcosine(deg+alpha);
		}

		//Initialize the true heading direction marker (triangle)
		hdgMark[0].x=centerX;
		hdgMark[0].y=centerY-radius+NIBLSCALE(3);
		hdgMark[1].x=centerX-NIBLSCALE(2);
		hdgMark[1].y=centerY-radius-NIBLSCALE(4);
		hdgMark[2].x=centerX+NIBLSCALE(2);
		hdgMark[2].y=hdgMark[1].y;
		hdgMark[3].x=hdgMark[0].x;
		hdgMark[3].y=hdgMark[0].y;

		//Initialize coordinates for airplane symbol
		fusA.x=centerX;
		fusA.y=centerY-NIBLSCALE(7);
		fusB.x=centerX;
		fusB.y=centerY+NIBLSCALE(7);
		winA.x=centerX-NIBLSCALE(8);
		winA.y=centerY-NIBLSCALE(3);
		winB.x=centerX+NIBLSCALE(8);
		winB.y=centerY-NIBLSCALE(3);
		taiA.x=centerX-NIBLSCALE(3);
		taiA.y=centerY+NIBLSCALE(4);
		taiB.x=centerX+NIBLSCALE(3);
		taiB.y=centerY+NIBLSCALE(4);

		DoInit[MDI_DRAWHSI]=false;
	}

	screenModeNormal=INVERTCOLORS; //to check if colors are inverted

	HPEN hpOld = (HPEN) SelectObject(hDC, LKPen_Black_N1);
	HBRUSH hbOld = (HBRUSH) SelectObject(hDC, LKBrush_Black);

	//get the track bearing
	int angle = 360-round(DrawInfo.TrackBearing);

	//Draw the compass rose markers
	int alpha=angle%10;
	POINT external,internal;
	for(int i=0,isBig=1;i<72;i++,isBig=!isBig) {
		external.x=compassMarks[i][alpha].extX;
		external.y=compassMarks[i][alpha].extY;
		internal.x=compassMarks[i][alpha].intX;
		internal.y=compassMarks[i][alpha].intY;
		_DrawLine(hDC, PS_ENDCAP_SQUARE, isBig?NIBLSCALE(1):1,external,internal,screenModeNormal?RGB_WHITE:RGB_BLACK,rc);
	}

	//Put the labels on compass rose
	SelectObject(hDC, LK8TitleFont);
	for(int i=0;i<12;i++) {
		int deg=i*30+angle;
		int x=centerX+labelsRadius*fastsine(deg);
		int y=centerY-labelsRadius*fastcosine(deg);
		LKWriteText(hDC,label[i],x,y,0, WTMODE_NORMAL,WTALIGN_CENTER,RGB_WHITE,false);
	}

	//Draw true heading mark on the top of the compass rose
	SelectObject(hDC, LKPen_Red_N1);
	SelectObject(hDC, LKBrush_Red);
	Polygon(hDC,hdgMark,4);

	if(ValidTaskPoint(ActiveWayPoint)) {
		if(Task[ActiveWayPoint].Index>=0) { //Draw CDI only if there is a task/route active
			//TODO: show them on HSI screen:
			//DerivedDrawInfo.LegCrossTrackError
			//DerivedDrawInfo.LegActualTrueCourse

			double rotation=DerivedDrawInfo.LegActualTrueCourse-DrawInfo.TrackBearing;
			double sin=fastsine(rotation);
			double cos=fastcosine(rotation);

			//This is the upper side of the course direction arrow
			long labelsRadiusXsin=labelsRadius*sin;
			long labelsRadiusXcos=labelsRadius*cos;
			long cdiRadiusXsin=cdiRadius*sin;
			long cdiRadiusXcos=cdiRadius*cos;
			POINT up, down;
			up.x=centerX+labelsRadiusXsin;
			up.y=centerY-labelsRadiusXcos;
			down.x=centerX+cdiRadiusXsin;
			down.y=centerY-cdiRadiusXcos;
			_DrawLine(hDC, PS_ENDCAP_SQUARE, NIBLSCALE(2),up,down,RGB_GREEN,rc);
			long innerradiusXsin=innerradius*sin;
			long innerradiusXcos=innerradius*cos;
			long arrowXsin=NIBLSCALE(4)*sin;
			long arrowXcos=NIBLSCALE(4)*cos;
			long topX=centerX+innerradiusXsin;
			long topY=centerY-innerradiusXcos;
			POINT triangle[4] = {
					{topX,topY}, //top
					{centerX+labelsRadiusXsin+arrowXcos,centerY-labelsRadiusXcos+arrowXsin}, //right
					{centerX+labelsRadiusXsin-arrowXcos,centerY-labelsRadiusXcos-arrowXsin}, //left
					{topX,topY}}; //top
			SelectObject(hDC, LKPen_Green_N1);
			SelectObject(hDC, LKBrush_Green);
			Polygon(hDC,triangle,4);

			//This is the opposite side of the course direction arrow
			up.x=centerX-innerradiusXsin;
			up.y=centerY+innerradiusXcos;
			down.x=centerX-cdiRadiusXsin;
			down.y=centerY+cdiRadiusXcos;
			_DrawLine(hDC, PS_ENDCAP_SQUARE, NIBLSCALE(2),up,down,RGB_GREEN,rc);

			//Course Deviation Indicator
			double xtd=DerivedDrawInfo.LegCrossTrackError;
			int dev; //deviation in pixel
			COLORREF cdiColor=screenModeNormal?RGB_YELLOW:RGB_DARKYELLOW;
			SelectObject(hDC, screenModeNormal?LKPen_White_N1:LKPen_Black_N1);
			SelectObject(hDC, screenModeNormal?LKBrush_White:LKBrush_Black);
			if(abs(xtd)<smallCDIscale) { //use small scale of 0.3 NM
				long tick=cdiFullScale/3; //every mark represents 0.1 NM
				for(int i=1;i<3;i++) {
					long tickXiXsin=tick*i*sin;
					long tickXiXcos=tick*i*cos;
					Circle(hDC,centerX+tickXiXcos,centerY+tickXiXsin,NIBLSCALE(1),rc,false,false);
					Circle(hDC,centerX-tickXiXcos,centerY-tickXiXsin,NIBLSCALE(1),rc,false,false);
				}
				dev=-(int)(round((cdiFullScale*xtd)/smallCDIscale));
			} else { // use big scale of 5 NM
				long tick=cdiFullScale/5; //every mark represents 1 NM
				for(int i=1;i<5;i++) {
					long tickXiXsin=tick*i*sin;
					long tickXiXcos=tick*i*cos;
					Circle(hDC,centerX+tickXiXcos,centerY+tickXiXsin,NIBLSCALE(1),rc,false,false);
					Circle(hDC,centerX-tickXiXcos,centerY-tickXiXsin,NIBLSCALE(1),rc,false,false);
				}
				if(xtd>bigCDIscale) {
					dev=-cdiFullScale;
					cdiColor=RGB_RED;
				} else if(xtd<-bigCDIscale) {
					dev=cdiFullScale;
					cdiColor=RGB_RED;
				} else dev=-round((cdiFullScale*xtd)/bigCDIscale);
			}
			long devXsin=dev*sin;
			long devXcos=dev*cos;
			up.x=centerX+cdiRadiusXsin+devXcos;
			up.y=centerY-cdiRadiusXcos+devXsin;
			down.x=centerX-cdiRadiusXsin+devXcos;
			down.y=centerY+cdiRadiusXcos+devXsin;
			_DrawLine(hDC, PS_ENDCAP_SQUARE, NIBLSCALE(2),up,down,cdiColor,rc);
		}
	}

	//Draw airplane symbol
	_DrawLine(hDC, PS_ENDCAP_SQUARE, NIBLSCALE(2),fusA,fusB,RGB_ORANGE,rc);
	_DrawLine(hDC, PS_ENDCAP_SQUARE, NIBLSCALE(2),winA,winB,RGB_ORANGE,rc);
	_DrawLine(hDC, PS_ENDCAP_SQUARE, NIBLSCALE(2),taiA,taiB,RGB_ORANGE,rc);

	SelectObject(hDC, hbOld);
	SelectObject(hDC, hpOld);
}
