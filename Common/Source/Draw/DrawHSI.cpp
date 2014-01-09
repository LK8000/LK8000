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
	static short top = (((rc.bottom-BottomSize-(rc.top + TOPLIMITER)-BOTTOMLIMITER)/PANELROWS)+rc.top+TOPLIMITER)-(rc.top + TOPLIMITER);
	static short centerX=(rc.right-rc.left)/2;
	static short centerY=((rc.bottom-BottomSize-top)/2)+top-NIBLSCALE(10);
	static int radius = NIBLSCALE(80); // gauge size radius
	static int innerradius = radius - NIBLSCALE(10);
	static int labelsRadius = radius -NIBLSCALE(20);
	static int smallMarkRadius = radius - NIBLSCALE(6);
	static int cdiRadius = radius - NIBLSCALE(35);
	static int cdiFullScale = radius -NIBLSCALE(30);
	//TCHAR Buffer[LKSIZEBUFFERVALUE];
	static compassMark compassMarks[72][10];

	//From Wikipedia about Course Deviation Indicator:
	//When used with a GPS it shows actual distance left or right of the programmed courseline.
	//Sensitivity is usually programmable or automatically switched, but 5 nautical miles (9.3 km) deviation at full scale is typical for en route operations.
	//Approach and terminal operations have a higher sensitivity up to frequently .3 nautical miles (0.56 km) at full scale.
	static const int bigCDIscale=9260; //5 NM
	static const int smallCDIscale=557; //0.3 NM

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

	static HPEN hpMarks = LKPen_White_N1;
	static HBRUSH hbMarks = LKBrush_White;
	static HPEN hpDir = LKPen_Green_N1;
	static HBRUSH hbDir = LKBrush_Green;
	static COLORREF cdiColor=RGB_YELLOW;
	static COLORREF cdiOutOfScaleColor=RGB_RED;
	static COLORREF markColor=RGB_WHITE;
	static COLORREF dirColor=RGB_GREEN;
	static COLORREF airplaneSymbolColor=RGB_ORANGE;
	static bool prevScreenModeNormal=true;

	if(DoInit[MDI_DRAWHSI]) {
		//all the sizes must be recalculated in case of screen resolution change:
		top=(((rc.bottom-BottomSize-(rc.top + TOPLIMITER)-BOTTOMLIMITER)/PANELROWS)+rc.top+TOPLIMITER)-(rc.top + TOPLIMITER);
		centerX=(rc.right-rc.left)/2;
		centerY=((rc.bottom-BottomSize-top)/2)+top-NIBLSCALE(10);
		radius = NIBLSCALE(80);
		innerradius = radius - NIBLSCALE(10);
		labelsRadius = radius -NIBLSCALE(20);
		smallMarkRadius = radius - NIBLSCALE(6);
		cdiRadius = radius - NIBLSCALE(35);
		cdiFullScale = radius -NIBLSCALE(30);

		//For the positions of all 72 compass rose marks there are 10 possible cases to be pre-calculated:
		for(int alpha=0;alpha<10;alpha++) for(int i=0, deg=0, isBig=1; i<72; i++, deg+=5, isBig=!isBig) {
			compassMarks[i][alpha].extX=centerX+(short)(radius*fastsine(deg+alpha));
			compassMarks[i][alpha].extY=centerY-(short)(radius*fastcosine(deg+alpha));
			compassMarks[i][alpha].intX=centerX+(short)((isBig?innerradius:smallMarkRadius)*fastsine(deg+alpha));
			compassMarks[i][alpha].intY=centerY-(short)((isBig?innerradius:smallMarkRadius)*fastcosine(deg+alpha));
		}
		DoInit[MDI_DRAWHSI]=false;
	}

	if(INVERTCOLORS!=prevScreenModeNormal) {//if screen mode changed
		prevScreenModeNormal=!prevScreenModeNormal;
		if(prevScreenModeNormal) { //if true set normal colors
			cdiColor=RGB_YELLOW;
			cdiOutOfScaleColor=RGB_RED;
			markColor=RGB_WHITE;
			dirColor=RGB_GREEN;
			airplaneSymbolColor=RGB_ORANGE;
			hpMarks = LKPen_White_N1;
			hbMarks = LKBrush_White;
			hpDir = LKPen_Green_N1;
			hbDir = LKBrush_Green;
		} else { //otherwise the text colors are inverted so adjust the colors properly!
			cdiColor=RGB_BLACK;
			cdiOutOfScaleColor=RGB_GREY;
			markColor=RGB_BLACK;
			dirColor=RGB_BLACK;
			airplaneSymbolColor=RGB_GREY;
			hpMarks = LKPen_Grey_N1;
			hbMarks = LKBrush_Grey;
			hpDir = LKPen_Black_N1;
			hbDir = LKBrush_Black;
		}
	}

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
		_DrawLine(hDC, PS_ENDCAP_SQUARE, isBig?NIBLSCALE(1):1,external,internal,markColor,rc);
	}

	//Put the labels on compass rose
	SelectObject(hDC, LK8TitleFont);
	for(int i=0;i<12;i++) {
		int deg=i*30+angle;
		int x=centerX+(int)(labelsRadius*fastsine(deg));
		int y=centerY-(int)(labelsRadius*fastcosine(deg));
		LKWriteText(hDC,label[i],x,y,0, WTMODE_NORMAL,WTALIGN_CENTER,RGB_WHITE,false);
	}

	if(ValidTaskPoint(ActiveWayPoint)) {
		if(Task[ActiveWayPoint].Index>=0) { //Draw CDI only if there is a task/route active
			//TODO: show them on HSI screen:
			//DerivedDrawInfo.LegCrossTrackError
			//DerivedDrawInfo.LegActualTrueCourse

			double rotation=DerivedDrawInfo.LegActualTrueCourse-DrawInfo.TrackBearing;
			POINT up, down;

			//TODO: optimize calculations...

			//This is the upper side of the course direction arrow
			up.x=centerX+(long)(labelsRadius*fastsine(rotation));
			up.y=centerY-(long)(labelsRadius*fastcosine(rotation));
			down.x=centerX+(long)(cdiRadius*fastsine(rotation));
			down.y=centerY-(long)(cdiRadius*fastcosine(rotation));
			_DrawLine(hDC, PS_ENDCAP_SQUARE, NIBLSCALE(2),up,down,dirColor,rc);
			long topX=centerX+(long)(innerradius*fastsine(rotation));
			long topY=centerY-(long)(innerradius*fastcosine(rotation));
			long rightX=centerX+(long)(labelsRadius*fastsine(rotation))+(long)(NIBLSCALE(4)*fastcosine(rotation));
			long rightY=centerY-(long)(labelsRadius*fastcosine(rotation))+(long)(NIBLSCALE(4)*fastsine(rotation));
			long leftX=centerX+(long)(labelsRadius*fastsine(rotation))-(long)(NIBLSCALE(4)*fastcosine(rotation));
			long leftY=centerY-(long)(labelsRadius*fastcosine(rotation))-(long)(NIBLSCALE(4)*fastsine(rotation));
			SelectObject(hDC, hpDir);
			SelectObject(hDC, hbDir);
			POINT triangle[4] = {{topX,topY},{rightX,rightY},{leftX,leftY},{topX,topY}};
			Polygon(hDC,triangle,4);

			//This is the lower side of the direction arrow
			up.x=centerX+(long)(innerradius*fastsine(-rotation));
			up.y=centerY+(long)(innerradius*fastcosine(-rotation));
			down.x=centerX+(long)(cdiRadius*fastsine(-rotation));
			down.y=centerY+(long)(cdiRadius*fastcosine(-rotation));
			_DrawLine(hDC, PS_ENDCAP_SQUARE, NIBLSCALE(2),up,down,dirColor,rc);

			//CDI
			double xtd=DerivedDrawInfo.LegCrossTrackError;
			int dev; //deviation in pixel
			COLORREF color=cdiColor;
			SelectObject(hDC, hpMarks);
			SelectObject(hDC, hbMarks);
			if(abs(xtd)<smallCDIscale) { //use small scale of 0.3 NM
				long tick=cdiFullScale/3; //every mark represents 0.1 NM
				for(int i=1;i<3;i++) {
					long x=centerX+(long)(tick*i*fastcosine(rotation));
					long y=centerY+(long)(tick*i*fastsine(rotation));
					Circle(hDC,x,y,NIBLSCALE(1),rc,false,false);
					x=centerX-(long)(tick*i*fastcosine(rotation));
					y=centerY-(long)(tick*i*fastsine(rotation));
					Circle(hDC,x,y,NIBLSCALE(1),rc,false,false);
				}
				dev=-(int)(round((cdiFullScale*xtd)/smallCDIscale));
			} else { // use big scale of 5 NM
				long tick=cdiFullScale/5; //every mark represents 1 NM
				for(int i=1;i<5;i++) {
					long x=centerX+(long)(tick*i*fastcosine(rotation));
					long y=centerY+(long)(tick*i*fastsine(rotation));
					Circle(hDC,x,y,NIBLSCALE(1),rc,false,false);
					x=centerX-(long)(tick*i*fastcosine(rotation));
					y=centerY-(long)(tick*i*fastsine(rotation));
					Circle(hDC,x,y,NIBLSCALE(1),rc,false,false);
				}
				if(xtd>bigCDIscale) {
					dev=-cdiFullScale;
					color=cdiOutOfScaleColor;
				} else if(xtd<-bigCDIscale) {
					dev=cdiFullScale;
					color=cdiOutOfScaleColor;
				} else dev=-(int)(round((cdiFullScale*xtd)/bigCDIscale));
			}
			up.x=centerX+(long)(cdiRadius*fastsine(rotation))+(long)(dev*fastcosine(rotation));
			up.y=centerY-(long)(cdiRadius*fastcosine(rotation))+(long)(dev*fastsine(rotation));
			down.x=centerX+(long)(cdiRadius*fastsine(-rotation))+(long)(dev*fastcosine(-rotation));
			down.y=centerY+(long)(cdiRadius*fastcosine(-rotation))-(long)(dev*fastsine(-rotation));;
			_DrawLine(hDC, PS_ENDCAP_SQUARE, NIBLSCALE(2),up,down,color,rc);
		}
	}

	//Draw airplane symbol
	POINT a,b;
	a.x=centerX;
	a.y=centerY-NIBLSCALE(7);
	b.x=centerX;
	b.y=centerY+NIBLSCALE(7);
	_DrawLine(hDC, PS_ENDCAP_SQUARE, NIBLSCALE(2),a,b,airplaneSymbolColor,rc);
	a.x=centerX-NIBLSCALE(8);
	a.y=centerY-NIBLSCALE(3);
	b.x=centerX+NIBLSCALE(8);
	b.y=centerY-NIBLSCALE(3);
	_DrawLine(hDC, PS_ENDCAP_SQUARE, NIBLSCALE(2),a,b,airplaneSymbolColor,rc);
	a.x=centerX-NIBLSCALE(3);
	a.y=centerY+NIBLSCALE(4);
	b.x=centerX+NIBLSCALE(3);
	b.y=centerY+NIBLSCALE(4);
	_DrawLine(hDC, PS_ENDCAP_SQUARE, NIBLSCALE(2),a,b,airplaneSymbolColor,rc);

	SelectObject(hDC, hbOld);
	SelectObject(hDC, hpOld);
}
