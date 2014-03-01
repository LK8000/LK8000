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

#ifndef __MINGW32__
#define DEG "\xB0"
#else
#define DEG "°"
#endif

HSIreturnStruct MapWindow::DrawHSI(HDC hDC, const RECT rc) {
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

    static const int fiveNauticalMiles=(5.0*NAUTICALMILESTOMETRES); //Large Course Deviation Indicator scale: 5 nautical miles (9260 m)
    static const int smallCDIscale=(0.3*NAUTICALMILESTOMETRES); //Narrow (zoomed) CDI scale: 0.3 nautical miles (557 m)

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

    HSIreturnStruct returnStruct = {false, false, false};

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
    _stprintf(Buffer, TEXT("%03d")TEXT(DEG),(int)round(DrawInfo.TrackBearing));
    LKWriteText(hDC,Buffer,posTRKx,posTRKy,0,WTMODE_NORMAL,WTALIGN_CENTER,RGB_RED,false);

    LockTaskData(); // protect Task & WayPointList
    if(ValidTaskPoint(ActiveWayPoint)) {
        if(Task[ActiveWayPoint].Index>=0) { //Draw course direction and CDI only if there is a task/route active
            double course = DerivedDrawInfo.LegActualTrueCourse;
            double deviation = DerivedDrawInfo.LegCrossTrackError;
            int finalWaypoint=getFinalWaypoint();
            if(finalWaypoint==ActiveWayPoint) { //if we are flying to the final destination
                returnStruct.approach=true;
                const double varioFtMin=DerivedDrawInfo.Vario*TOFEETPERMINUTE; //Convert vertical speed to Ft/min

                //Print vertical speed in Ft/min
                int xpos=78;
                if(ScreenSize==ss800x480 || ScreenSize==ss480x272) xpos=100;
                _stprintf(Buffer,gettext(TEXT("_@M784_"))); //"Vario"
                SelectObject(hDC, LK8PanelSmallFont);
                LKWriteText(hDC,Buffer,centerX+radius+NIBLSCALE(ScreenLandscape?xpos:47),centerY-NIBLSCALE(28),0, WTMODE_NORMAL,WTALIGN_RIGHT,RGB_LIGHTGREEN,false);
                _stprintf(Buffer, TEXT("%+.0f"),varioFtMin); //print the value
                if(!ScreenLandscape || (ScreenSize!=ss800x480 && ScreenSize!=ss480x272)) SelectObject(hDC, LK8PanelMediumFont);
                else SelectObject(hDC, LK8PanelBigFont);
                LKWriteText(hDC,Buffer,centerX+radius+NIBLSCALE(ScreenLandscape?xpos:47),centerY-NIBLSCALE(19),0, WTMODE_NORMAL,WTALIGN_RIGHT,RGB_WHITE,false);
                _stprintf(Buffer,TEXT("FPM")); //measure unit
                SelectObject(hDC, LK8PanelUnitFont);
                LKWriteText(hDC,Buffer,centerX+radius+NIBLSCALE(ScreenLandscape?xpos:47),centerY+NIBLSCALE(9),0, WTMODE_NORMAL,WTALIGN_RIGHT,RGB_WHITE,false);

                if(DerivedDrawInfo.WaypointDistance<fiveNauticalMiles) { //if we are close to the destination
                    if(DerivedDrawInfo.WaypointDistance<1500) returnStruct.landing=true; //if at less than 1.5 Km don't show glide slope bar
                    else { //Build glide slope bar
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
                        int startX=0;
                        if(ScreenLandscape) startX=8;
                        int gssStart=centerY-NIBLSCALE(72);
                        int gssIncrement=NIBLSCALE(12);
                        if(ScreenLandscape) external.x=centerX+radius+NIBLSCALE(startX+10);
                        else external.x=centerX+radius+NIBLSCALE(startX+8);
                        for(int i=0,isBig=1;i<=12;i++,isBig=!isBig) {
                            internal.y=external.y=gssStart+gssIncrement*i;
                            internal.x=centerX+radius+(isBig?NIBLSCALE(startX+3):NIBLSCALE(startX+5));
                            _DrawLine(hDC,PS_SOLID,isBig?NIBLSCALE(1):1,internal,external,INVERTCOLORS?RGB_LIGHTGREY:RGB_BLACK,rc);
                        }

                        //Draw glide slope marker
                        POINT triangle[4];
                        if(ScreenLandscape) {
                            triangle[0].x=triangle[3].x=centerX+radius+NIBLSCALE(startX+7);
                            triangle[1].x=triangle[2].x=centerX+radius+NIBLSCALE(startX+13);
                        } else {
                            triangle[0].x=triangle[3].x=centerX+radius+NIBLSCALE(startX+5);
                            triangle[1].x=triangle[2].x=centerX+radius+NIBLSCALE(startX+11);
                        }
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
                        if (ScreenLandscape) {
                            for(int i=0;i<=6;i++) {
                                _stprintf(Buffer, TEXT("%d"),i);
                                LKWriteText(hDC,Buffer,centerX+radius+NIBLSCALE(6),gssStart+gssIncrement*i,0, WTMODE_NORMAL,WTALIGN_CENTER,isOutOfScale?RGB_LIGHTRED:RGB_WHITE,false);
                            }
                        } else {
                            _stprintf(Buffer, TEXT("0"));
                            LKWriteText(hDC,Buffer,centerX+radius-NIBLSCALE(1),gssStart,0, WTMODE_NORMAL,WTALIGN_CENTER,isOutOfScale?RGB_LIGHTRED:RGB_WHITE,false);
                            _stprintf(Buffer, TEXT("6"));
                            LKWriteText(hDC,Buffer,centerX+radius-NIBLSCALE(1),gssStart+gssIncrement*6,0, WTMODE_NORMAL,WTALIGN_CENTER,isOutOfScale?RGB_LIGHTRED:RGB_WHITE,false);
                        }
                    } //end of glide slope bar
                }

                //Determine if to give HSI indication respect destination runaway (QFU)
                if(finalWaypoint==0 || DerivedDrawInfo.WaypointDistance<fiveNauticalMiles) {//if direct GOTO or below 5 NM
                    int QFU=WayPointList[Task[ActiveWayPoint].Index].RunwayDir; //get runaway orientation
                    if(QFU>0 && QFU<=360) { //valid QFU
                        returnStruct.usingQFU=true;
                        course=QFU;
                        deviation=DerivedDrawInfo.WaypointDistance*fastsine(AngleDifference(course,DerivedDrawInfo.WaypointBearing)); //flat cross track error
                    }
                }
            }

            //Print the desired course
            SelectObject(hDC, LK8InfoSmallFont);
            _stprintf(Buffer, TEXT("%03d")TEXT(DEG),(int)round(course));
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
            if(ActiveWayPoint>0 || returnStruct.usingQFU) { //we are flying on a predefined routeline or we have the info for landing: draw CDI
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
                _stprintf(Buffer, TEXT("%03d")TEXT(DEG),(int)round(DerivedDrawInfo.WaypointBearing));
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
    UnlockTaskData();

    //Draw airplane symbol in the center of HSI
    _DrawLine(hDC,PS_SOLID,NIBLSCALE(2),fusA,fusB,RGB_ORANGE,rc);
    _DrawLine(hDC,PS_SOLID,NIBLSCALE(2),winA,winB,RGB_ORANGE,rc);
    _DrawLine(hDC,PS_SOLID,NIBLSCALE(2),taiA,taiB,RGB_ORANGE,rc);

    //Draw VSI: Vertical Situation Indicator
    if(!returnStruct.usingQFU && !returnStruct.landing) { //if not using glide slope and not lading
        LockTaskData(); // protect Task & WayPointList
        if(ValidTaskPoint(ActiveWayPoint)) { //if valid task
            if(ActiveWayPoint>0) { //if we have a previous WP
                if(Task[ActiveWayPoint].Index>=0 && Task[ActiveWayPoint-1].Index>=0) { //if valid waypoints

                    //Calculate expected altitude in route
                    double expectedAlt=WayPointList[Task[ActiveWayPoint].Index].Altitude;
                    if(Task[ActiveWayPoint].Leg>0 || Task[ActiveWayPoint].Leg!=Task[ActiveWayPoint].Leg) {
                        expectedAlt=(WayPointList[Task[ActiveWayPoint].Index].Altitude-WayPointList[Task[ActiveWayPoint-1].Index].Altitude) / Task[ActiveWayPoint].Leg
                                * DerivedDrawInfo.LegDistanceCovered + WayPointList[Task[ActiveWayPoint-1].Index].Altitude;
                    }
                    expectedAlt*=TOFEET;
                    double diff=(DrawInfo.Altitude*TOFEET)-expectedAlt; //difference with current altitude

                    //Find out a proper scale to display the difference on the VSI bar
                    double scale=fabs(diff);
                    bool outOfScale=false;
                    if(scale<100) scale=100;
                    else if(scale<500) scale=500;
                    else if(scale<1000) scale=1000;
                    else if(scale<5000) scale=5000;
                    else if(scale<10000) scale=10000;
                    else {
                        if(scale>=20000) outOfScale=true;
                        scale=20000;
                    }
                    int scaleInPixel=NIBLSCALE(70);
                    if(!ScreenLandscape) scaleInPixel=NIBLSCALE(45);
                    double pixelPerFeet=scaleInPixel/scale;

                    //Draw VSI background
                    int leftBorder=centerX+radius+NIBLSCALE(3);
                    int rightBorder=centerX+radius+NIBLSCALE(6);
                    if(ScreenSize==ss800x480 || ScreenSize==ss480x272) {
                        leftBorder+=NIBLSCALE(5);
                        rightBorder+=NIBLSCALE(7);
                    }
                    int topBorder=centerY-scaleInPixel;
                    int bottomBorder=centerY+scaleInPixel;
                    if(DerivedDrawInfo.TerrainValid) {
                        const double altAGLft=DerivedDrawInfo.AltitudeAGL*TOFEET;
                        int groundLevel=scaleInPixel;
                        if(fabs(altAGLft)<scale) groundLevel=(int)round(altAGLft*pixelPerFeet);
                        else if(altAGLft<0) groundLevel=-scaleInPixel;
                        if(groundLevel>-scaleInPixel) { //sky part
                            HPEN PenSky=CreatePen(PS_SOLID,NIBLSCALE(1),RGB(0,153,153));
                            HBRUSH BrushSky=CreateSolidBrush(COLORREF RGB(0,153,153));
                            SelectObject(hDC,PenSky);
                            SelectObject(hDC,BrushSky);
                            Rectangle(hDC,leftBorder,centerY+groundLevel,rightBorder,topBorder);
                            if(PenSky) DeleteObject(PenSky);
                            if(BrushSky) DeleteObject(BrushSky);
                        }
                        if(groundLevel<scaleInPixel) { //ground part
                            HPEN PenGround=CreatePen(PS_SOLID,NIBLSCALE(1),RGB(204,102,0));
                            HBRUSH BrushGround=CreateSolidBrush(COLORREF RGB(204,102,0));
                            SelectObject(hDC,PenGround);
                            SelectObject(hDC,BrushGround);
                            Rectangle(hDC,leftBorder,bottomBorder,rightBorder,centerY+groundLevel+1);
                            if(PenGround) DeleteObject(PenGround);
                            if(BrushGround) DeleteObject(BrushGround);
                        }
                    }

                    //Draw VSI scale
                    int vsiIncrement=(int)round(scaleInPixel/10);
                    internal.x=leftBorder;
                    external.x=rightBorder-1;
                    for(int i=1,isBig=0;i<=10;i++,isBig=!isBig) {
                        internal.y=external.y=centerY-vsiIncrement*i; //upper part
                        _DrawLine(hDC,PS_SOLID,isBig?NIBLSCALE(1):1,internal,external,INVERTCOLORS?RGB_WHITE:RGB_BLACK,rc);
                        internal.y=external.y=centerY+vsiIncrement*i; //lower part
                        _DrawLine(hDC,PS_SOLID,isBig?NIBLSCALE(1):1,internal,external,INVERTCOLORS?RGB_WHITE:RGB_BLACK,rc);
                    }

                    //Draw airplane symbol at the center of VSI scale
                    internal.x=leftBorder-3;
                    external.x=rightBorder+2;
                    internal.y=external.y=centerY;
                    _DrawLine(hDC,PS_SOLID,NIBLSCALE(1),internal,external,INVERTCOLORS?RGB_WHITE:RGB_BLACK,rc);
                    internal.x=external.x=internal.x+(external.x-internal.x)/2;
                    external.y=centerY-NIBLSCALE(2);
                    _DrawLine(hDC,PS_SOLID,1,internal,external,INVERTCOLORS?RGB_WHITE:RGB_BLACK,rc);
                    SelectObject(hDC,INVERTCOLORS?LKPen_White_N1:LKPen_Black_N1);
                    Circle(hDC,internal.x,centerY,NIBLSCALE(1),rc,false,false);

                    //Print +scale and -scale at top and bottom of VSI
                    SelectObject(hDC, LK8SmallFont);
                    int xPos=leftBorder+NIBLSCALE(2);
                    if(!ScreenLandscape) xPos=leftBorder-NIBLSCALE(5);
                    if(scale<10000) _stprintf(Buffer, TEXT("+%.0fft"),scale);
                    else _stprintf(Buffer, TEXT("+%.0ff"),scale);
                    LKWriteText(hDC,Buffer,xPos,topBorder-NIBLSCALE(5),0, WTMODE_NORMAL,WTALIGN_CENTER,outOfScale?RGB_LIGHTRED:RGB_WHITE,false);
                    if(scale<10000) _stprintf(Buffer, TEXT("-%.0fft"),scale);
                    else _stprintf(Buffer, TEXT("-%.0ff"),scale);
                    LKWriteText(hDC,Buffer,xPos,bottomBorder+NIBLSCALE(5),0, WTMODE_NORMAL,WTALIGN_CENTER,outOfScale?RGB_LIGHTRED:RGB_WHITE,false);

                    //Draw expected in route altitude marker
                    POINT triangle[4];
                    triangle[0].x=triangle[3].x=leftBorder;
                    triangle[1].x=triangle[2].x=rightBorder+NIBLSCALE(1);
                    if(outOfScale) {
                        if(diff>0) { //up
                            triangle[1].y=bottomBorder-NIBLSCALE(1);
                            triangle[0].y=triangle[3].y=bottomBorder+NIBLSCALE(2);
                            triangle[2].y=bottomBorder+NIBLSCALE(5);
                        } else { //down
                            triangle[1].y=bottomBorder-NIBLSCALE(5);
                            triangle[0].y=triangle[3].y=bottomBorder-NIBLSCALE(2);
                            triangle[2].y=bottomBorder+NIBLSCALE(1);
                        }
                    } else {
                        triangle[0].y=triangle[3].y=centerY+(int)round(pixelPerFeet*diff);
                        triangle[1].y=triangle[0].y-NIBLSCALE(3);
                        triangle[2].y=triangle[0].y+NIBLSCALE(3);
                    }
                    if(outOfScale) {
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
                }
            }
        }
        UnlockTaskData();
    }

    SelectObject(hDC, hbOld);
    SelectObject(hDC, hpOld);
    return returnStruct;
}
