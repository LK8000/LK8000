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


#include "externs.h"
#include "LKMapWindow.h"
#include "LKObjects.h"
#include "RGB.h"
#include "DoInits.h"

bool MapWindow::DrawHSI(HDC hDC, const RECT rc, bool *glideSlopeBarEnabled) {
	static short centerX, centerY; //center coordinates of HSI gauge
	static short radius; //HSI gauge size radius
	static short innerradius; //internal radius of big marks on the compass rose
	static short labelsRadius; //radius where the directions labels are drawn
	static short smallMarkRadius; //internal radius of small marks on the compass rose
	static short cdiRadius; //radius of Course Deviation Indicator line
	static short cdiFullScale; //size in pixel for all one side CDI scale
	static short smallScaleTick; //interval between marks on small CDI scale
	static short bigScaleTick; //interval between marks on big CDI scale
	static POINT fusA,fusB,winA,winB,taiA,taiB; //coordinates for airplane symbol
	static short posTRKx, posTRKy; //coordinates of current track textual information
	static short posDTKx; //X coordinate of desired track textual information
	static short posBRGy; //Y coordinate of current bearing to next waypoint
	static short posXTKx, posXTKy; //coordinates of cross track error textual information

	static struct { //Compass rose marks coordinates matrix: using short's to use less memory
		short extX, extY; //coordinates external mark point
		short intX, intY; //coordinates internal mark point
	} compassMarks[72][10]; //72 compass marks (one every 5 degrees), 10 possible cases
	static POINT hdgMark[4]; //Coordinates of heading marker (red triangle on the top of compass rose)

	static const int fiveNauticalMiles=9260; //Large Course Deviation Indicator scale: 5 nautical miles (9260 m)
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

	if(DoInit[MDI_DRAWHSI]) { //All the dimensions must be recalculated in case of screen resolution change
		if(ScreenLandscape) radius=NIBLSCALE(80);
		else radius=NIBLSCALE(70);
		short top=(((rc.bottom-BottomSize-(rc.top + TOPLIMITER)-BOTTOMLIMITER)/PANELROWS)+rc.top+TOPLIMITER)-(rc.top + TOPLIMITER);
		centerX=(rc.right-rc.left)/2;
		centerY=((rc.bottom-BottomSize-top)/2)+top-NIBLSCALE(12);
		innerradius=radius-NIBLSCALE(10);
		labelsRadius=radius-NIBLSCALE(20);
		smallMarkRadius=radius-NIBLSCALE(6);
		cdiRadius=radius-NIBLSCALE(35);
		cdiFullScale=radius-NIBLSCALE(30);

		//For the positions of all 72 compass rose marks there are 10 possible cases to be pre-calculated:
		for(int alpha=0;alpha<10;alpha++) for(int i=0, deg=0, isBig=1; i<72; i++, deg+=5, isBig=!isBig) {
			compassMarks[i][alpha].extX=centerX+(short)(radius*fastsine(deg+alpha));
			compassMarks[i][alpha].extY=centerY-(short)(radius*fastcosine(deg+alpha));
			compassMarks[i][alpha].intX=centerX+(short)((isBig?innerradius:smallMarkRadius)*fastsine(deg+alpha));
			compassMarks[i][alpha].intY=centerY-(short)((isBig?innerradius:smallMarkRadius)*fastcosine(deg+alpha));
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

		//Initialize the sizes of the marks on the two CDI scales
		smallScaleTick=(short)round(cdiFullScale/3); //every mark represents 0.1 NM
		bigScaleTick=(short)round(cdiFullScale/5); //every mark represents 1 NM

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

		//Initialize coordinates of HSI textual informations
		posTRKx=centerX-radius+NIBLSCALE(20);
		posTRKy=centerY-radius;
		posDTKx=centerX+radius-NIBLSCALE(15);
		posBRGy=centerY-radius+NIBLSCALE(11);
		posXTKx=centerX+radius-NIBLSCALE(25);
		posXTKy=centerY+radius-NIBLSCALE(5);

		DoInit[MDI_DRAWHSI]=false;
	}

	*glideSlopeBarEnabled=false;
	HPEN hpOld = (HPEN) SelectObject(hDC, LKPen_Black_N1);
	HBRUSH hbOld = (HBRUSH) SelectObject(hDC, LKBrush_Black);

	//get the track bearing
	int angle = 360-(int)round(DrawInfo.TrackBearing);

	//Draw the compass rose markers
	int alpha=angle%10;
	#if BUGSTOP
	LKASSERT(alpha>=0 && alpha<10);
	#endif
	if (angle<0) angle=0; // recovered
	POINT external,internal;
	for(int i=0,isBig=1;i<72;i++,isBig=!isBig) {
		external.x=compassMarks[i][alpha].extX;
		external.y=compassMarks[i][alpha].extY;
		internal.x=compassMarks[i][alpha].intX;
		internal.y=compassMarks[i][alpha].intY;
		_DrawLine(hDC,PS_SOLID,isBig?NIBLSCALE(1):1,external,internal,INVERTCOLORS?RGB_WHITE:RGB_BLACK,rc);
	}

	//Put the labels on compass rose
	SelectObject(hDC, LK8TitleFont);
	for(int i=0;i<12;i++) {
		int deg=i*30+angle;
		LKWriteText(hDC,label[i],centerX+(int)(labelsRadius*fastsine(deg)),centerY-(int)(labelsRadius*fastcosine(deg)),0, WTMODE_NORMAL,WTALIGN_CENTER,RGB_WHITE,false);
	}

	//Draw true heading mark on the top of the compass rose
	SelectObject(hDC, LKPen_Red_N1);
	SelectObject(hDC, LKBrush_Red);
	Polygon(hDC,hdgMark,4);

	//Print the current track indication
	TCHAR Buffer[LKSIZEBUFFERVALUE];
	SelectObject(hDC, LK8InfoSmallFont);
	#ifndef __MINGW32__
		_stprintf(Buffer, TEXT("%03d\xB0"),(int)round(DrawInfo.TrackBearing));
	#else
		_stprintf(Buffer, TEXT("%03d°"),(int)round(DrawInfo.TrackBearing));
	#endif
	LKWriteText(hDC,Buffer,posTRKx,posTRKy,0,WTMODE_NORMAL,WTALIGN_CENTER,RGB_RED,false);

	bool emulatedILS=false;
	if(ValidTaskPoint(ActiveWayPoint)) {
		if(Task[ActiveWayPoint].Index>=0) { //Draw course direction and CDI only if there is a task/route active
			double course = DerivedDrawInfo.LegActualTrueCourse;
			double deviation = DerivedDrawInfo.LegCrossTrackError;
			int finalWaypoint=getFinalWaypoint();

			if(finalWaypoint==ActiveWayPoint) { //if we are flying to the final destination
				*glideSlopeBarEnabled=true;

				//Calculate glide slope inclination to reach the destination runaway
				double halfRunaway=WayPointList[Task[ActiveWayPoint].Index].RunwayLen/2;
				double distanceToRunaway=0;
				if(DerivedDrawInfo.WaypointDistance>halfRunaway) distanceToRunaway=DerivedDrawInfo.WaypointDistance-halfRunaway;
				double heightOnRunaway=DrawInfo.Altitude-WayPointList[Task[ActiveWayPoint].Index].Altitude;
				if(heightOnRunaway<0) heightOnRunaway=0;
				double glideSlope=RAD_TO_DEG*atan2(heightOnRunaway,distanceToRunaway);
				//double actualDescentAngle=0;
				//if(DerivedDrawInfo.Vario<0) actualDescentAngle=RAD_TO_DEG*atan2(-DerivedDrawInfo.Vario,DrawInfo.Speed);

				//Draw glide slope scale
				int gssStart=centerY-NIBLSCALE(72);
				int gssIncrement=NIBLSCALE(12);
				external.x=centerX+radius+NIBLSCALE(21);
				for(int i=0,isBig=1;i<=12;i++,isBig=!isBig) {
					internal.y=external.y=gssStart+gssIncrement*i;
					internal.x=centerX+radius+(isBig?NIBLSCALE(14):NIBLSCALE(16));
					_DrawLine(hDC,PS_SOLID,isBig?NIBLSCALE(1):1,internal,external,INVERTCOLORS?RGB_LIGHTGREY:RGB_BLACK,rc);
				}

				//Draw the blue marker representing the required glide slope inclination to reach the destination
				//if(requiredInclination<=0) {
				//	internal.y=external.y=gssStart-NIBLSCALE(5);
				//	_DrawLine(hDC,PS_SOLID,NIBLSCALE(2),internal,external,RGB_GREY,rc);
				//} else if(requiredInclination>6) {
				//	internal.y=external.y=gssStart+NIBLSCALE(72)*2+NIBLSCALE(5);
				//	_DrawLine(hDC,PS_SOLID,NIBLSCALE(2),internal,external,RGB_GREY,rc);
				//} else { // 0 < reqiredInclination <= 6
				//	internal.y=external.y=gssStart+(int)round((requiredInclination*NIBLSCALE(72)*2)/6);
				//	_DrawLine(hDC,PS_SOLID,NIBLSCALE(2),internal,external,RGB_GREEN,rc);
				//}

				//Draw glide slope marker
				POINT triangle[4];
				triangle[0].x=triangle[3].x=centerX+radius+NIBLSCALE(18);
				triangle[1].x=triangle[2].x=centerX+radius+NIBLSCALE(24);
				bool isOutOfScale=true;
				if(glideSlope<=0) {
					triangle[1].y=gssStart-NIBLSCALE(7);
					triangle[0].y=triangle[3].y=gssStart-NIBLSCALE(2);
					triangle[2].y=gssStart+NIBLSCALE(3);
				} else if(glideSlope>6) {
					triangle[1].y=gssStart+NIBLSCALE(72)*2-NIBLSCALE(3);
					triangle[0].y=triangle[3].y=triangle[1].y+NIBLSCALE(5);
					triangle[2].y=triangle[0].y+NIBLSCALE(5);
				} else { // 0 < glideSlope <= 6
					triangle[0].y=triangle[3].y=gssStart+(int)round((glideSlope*NIBLSCALE(72)*2)/6);
					triangle[1].y=triangle[0].y-NIBLSCALE(5);
					triangle[2].y=triangle[0].y+NIBLSCALE(5);
					isOutOfScale=false;
				}
				if(isOutOfScale) {
					SelectObject(hDC, LKPen_Red_N1);
					SelectObject(hDC, LKBrush_Red);
				} else if(INVERTCOLORS) {
					SelectObject(hDC, LKPen_White_N1);
					SelectObject(hDC, LKBrush_White);
				} else {
					SelectObject(hDC, LKPen_Black_N1);
					SelectObject(hDC, LKBrush_Black);
				}
				Polygon(hDC,triangle,4);

				//Put the labels on glide slope scale
				SelectObject(hDC, LK8PanelSmallFont);
				gssIncrement*=2;
				for(int i=0;i<=6;i++) {
					_stprintf(Buffer, TEXT("%d"),i);
					LKWriteText(hDC,Buffer,centerX+radius+NIBLSCALE(8),gssStart+gssIncrement*i,0, WTMODE_NORMAL,WTALIGN_CENTER,isOutOfScale?RGB_LIGHTRED:RGB_WHITE,false);
				}

				//Print glide slope value
				//if(!isOutOfScale) {
				//	SelectObject(hDC,LK8PanelSmallFont);
				//	#ifndef __MINGW32__
				//	_stprintf(Buffer, TEXT("%.1f\xB0"),glideSlope);
				//	#else
				//	_stprintf(Buffer, TEXT("%.1f°"),glideSlope);
				//	#endif
				//	LKWriteText(hDC,Buffer,centerX+radius+NIBLSCALE(38),triangle[0].y,0, WTMODE_NORMAL,WTALIGN_CENTER,RGB_WHITE,false);
				//}

				//Determine if to give HSI indication respect destination runaway (QFU)
				if(finalWaypoint==0 || DerivedDrawInfo.WaypointDistance<fiveNauticalMiles) {//if direct GOTO or below 5 NM
					int QFU=WayPointList[Task[ActiveWayPoint].Index].RunwayDir; //get runaway orientation
					if(QFU>0 && QFU<=360) { //valid QFU
						emulatedILS=true;
						course=QFU;
						deviation=DerivedDrawInfo.WaypointDistance*fastsine(AngleDifference(course,DerivedDrawInfo.WaypointBearing)); //flat cross track error
					}
				}
			}

			//Print the desired course
			SelectObject(hDC, LK8InfoSmallFont);
			#ifndef __MINGW32__
				_stprintf(Buffer, TEXT("%03d\xB0"),(int)round(course));
			#else
				_stprintf(Buffer, TEXT("%03d°"),(int)round(course));
			#endif
			LKWriteText(hDC,Buffer,posDTKx,posTRKy,0,WTMODE_NORMAL,WTALIGN_CENTER,RGB_GREEN,false);

			//Calculate rotation angle
			double rotation=course-DrawInfo.TrackBearing;
			double sin=fastsine(rotation);
			double cos=fastcosine(rotation);

			//This is the upper side of the course direction arrow
			long labelsRadiusXsin=(long)(labelsRadius*sin);
			long labelsRadiusXcos=(long)(labelsRadius*cos);
			long cdiRadiusXsin=(long)(cdiRadius*sin);
			long cdiRadiusXcos=(long)(cdiRadius*cos);
			POINT up, down;
			up.x=centerX+labelsRadiusXsin;
			up.y=centerY-labelsRadiusXcos;
			down.x=centerX+cdiRadiusXsin;
			down.y=centerY-cdiRadiusXcos;
			_DrawLine(hDC,PS_SOLID,NIBLSCALE(2),up,down,RGB_GREEN,rc);
			long innerradiusXsin=(long)(innerradius*sin);
			long innerradiusXcos=(long)(innerradius*cos);
			long arrowXsin=(long)(NIBLSCALE(4)*sin);
			long arrowXcos=(long)(NIBLSCALE(4)*cos);
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
			_DrawLine(hDC,PS_SOLID,NIBLSCALE(2),up,down,RGB_GREEN,rc);

			//Course Deviation Indicator
			if(ActiveWayPoint>0 || emulatedILS) { //we are flying on a predefined routeline or we have the info for landing: draw CDI
				int dev; //deviation in pixel
				COLORREF cdiColor=INVERTCOLORS?RGB_YELLOW:RGB_DARKYELLOW; //color of CDI
				SelectObject(hDC,INVERTCOLORS?LKPen_White_N1:LKPen_Black_N1); //color of CDI scale
				SelectObject(hDC,INVERTCOLORS?LKBrush_White:LKBrush_Black);
				if(abs((int)deviation)<smallCDIscale) { //use small scale of 0.3 NM
					for(int i=1;i<=3;i++) {
						long tickXiXsin=(long)(smallScaleTick*i*sin);
						long tickXiXcos=(long)(smallScaleTick*i*cos);
						Circle(hDC,centerX+tickXiXcos,centerY+tickXiXsin,NIBLSCALE(1),rc,false,false);
						Circle(hDC,centerX-tickXiXcos,centerY-tickXiXsin,NIBLSCALE(1),rc,false,false);
					}
					dev=-(int)(round((cdiFullScale*deviation)/smallCDIscale));
				} else { // use big scale of 5 NM
					for(int i=1;i<=5;i++) {
						long tickXiXsin=(long)(bigScaleTick*i*sin);
						long tickXiXcos=(long)(bigScaleTick*i*cos);
						Circle(hDC,centerX+tickXiXcos,centerY+tickXiXsin,NIBLSCALE(1),rc,false,false);
						Circle(hDC,centerX-tickXiXcos,centerY-tickXiXsin,NIBLSCALE(1),rc,false,false);
					}
					if(deviation>fiveNauticalMiles) { //The larger CDI scale is of 5 nautical miles
						dev=-cdiFullScale;
						cdiColor=RGB_RED;
					} else if(deviation<-fiveNauticalMiles) {
						dev=cdiFullScale;
						cdiColor=RGB_RED;
					} else dev=-(int)round((cdiFullScale*deviation)/fiveNauticalMiles);
				}
				long devXsin=(long)(dev*sin);
				long devXcos=(long)(dev*cos);
				up.x=centerX+cdiRadiusXsin+devXcos;
				up.y=centerY-cdiRadiusXcos+devXsin;
				down.x=centerX-cdiRadiusXsin+devXcos;
				down.y=centerY+cdiRadiusXcos+devXsin;
				_DrawLine(hDC,PS_SOLID,NIBLSCALE(2),up,down,cdiColor,rc);

				//Print the actual cross track error
				SelectObject(hDC, LK8InfoSmallFont);
				double xtk=fabs(deviation);
				if(xtk>1000) {
					xtk/=1000;
					if(xtk<=99.9) _stprintf(Buffer, TEXT("%.1f Km"),xtk);
					else _stprintf(Buffer,TEXT("%d Km"),(int)round(xtk));
				} else _stprintf(Buffer,TEXT("%d m"),(int)round(xtk));
				LKWriteText(hDC,Buffer,posXTKx,posXTKy+NIBLSCALE(2),0,WTMODE_NORMAL,WTALIGN_CENTER,cdiColor,false);

				//Draw bearing pointer to next waypoint
				rotation=DerivedDrawInfo.WaypointBearing-DrawInfo.TrackBearing;
				sin=fastsine(rotation);
				cos=fastcosine(rotation);
				arrowXsin=(long)(NIBLSCALE(2)*sin);
				arrowXcos=(long)(NIBLSCALE(2)*cos);
				topX=centerX+(long)((radius-NIBLSCALE(2))*sin);
				topY=centerY-(long)((radius-NIBLSCALE(2))*cos);
				innerradiusXsin=(long)(innerradius*sin);
				innerradiusXcos=(long)(innerradius*cos);
				triangle[0].x=topX;
				triangle[0].y=topY; //top
				triangle[1].x=centerX+innerradiusXsin+arrowXcos;
				triangle[1].y=centerY-innerradiusXcos+arrowXsin; //right
				triangle[2].x=centerX+innerradiusXsin-arrowXcos;
				triangle[2].y=centerY-innerradiusXcos-arrowXsin; //left
				triangle[3].x=topX;
				triangle[3].y=topY;
				HPEN PenViola=CreatePen(PS_SOLID,NIBLSCALE(1),RGB_MAGENTA);
				HBRUSH BrushViola=CreateSolidBrush(COLORREF RGB_MAGENTA);
				SelectObject(hDC,PenViola);
				SelectObject(hDC,BrushViola);
				Polygon(hDC,triangle,4);
				if(PenViola) DeleteObject(PenViola);
				if(BrushViola) DeleteObject(BrushViola);

				//Print the actual bearing to next WayPoint
				SelectObject(hDC, LK8InfoSmallFont);
				#ifndef __MINGW32__
					_stprintf(Buffer, TEXT("%03d\xB0"),(int)round(DerivedDrawInfo.WaypointBearing));
				#else
					_stprintf(Buffer, TEXT("%03d°"),(int)round(DerivedDrawInfo.WaypointBearing));
				#endif
				LKWriteText(hDC,Buffer,posDTKx,posBRGy+NIBLSCALE(2),0,WTMODE_NORMAL,WTALIGN_CENTER,RGB_MAGENTA,false);
			} else { //flying to the departure or a direct GOTO without information for landing: don't draw CDI
				//Draw anyway the CDI scale in grey (disabled)
				SelectObject(hDC,LKPen_Grey_N1); //color of CDI scale
				SelectObject(hDC,LKBrush_Grey);
				for(int i=1;i<=3;i++) {
					long tickXiXsin=(long)(smallScaleTick*i*sin);
					long tickXiXcos=(long)(smallScaleTick*i*cos);
					Circle(hDC,centerX+tickXiXcos,centerY+tickXiXsin,NIBLSCALE(1),rc,false,false);
					Circle(hDC,centerX-tickXiXcos,centerY-tickXiXsin,NIBLSCALE(1),rc,false,false);
				}

				//draw CDI in the center as part of the course direction arrow (same color)
				up.x=centerX+cdiRadiusXsin;
				up.y=centerY-cdiRadiusXcos;
				down.x=centerX-cdiRadiusXsin;
				down.y=centerY+cdiRadiusXcos;
				_DrawLine(hDC,PS_SOLID,NIBLSCALE(2),up,down,RGB_GREEN,rc);
			}
		}
	}

	//Draw airplane symbol
	_DrawLine(hDC,PS_SOLID,NIBLSCALE(2),fusA,fusB,RGB_ORANGE,rc);
	_DrawLine(hDC,PS_SOLID,NIBLSCALE(2),winA,winB,RGB_ORANGE,rc);
	_DrawLine(hDC,PS_SOLID,NIBLSCALE(2),taiA,taiB,RGB_ORANGE,rc);

	SelectObject(hDC, hbOld);
	SelectObject(hDC, hpOld);
	return emulatedILS;
}
