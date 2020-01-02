/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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
#include "LKStyle.h"
#include "Asset.hpp"

extern int InfoPageTopLineSeparator;


void MapWindow::DrawHSI(LKSurface& Surface, const RECT& rc, bool& usingQFU, bool& approach, bool& landing) {
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
    static short VertSpeedX, VertSpeedLabelY, VertSpeedValueY, VertSpeedUnitY; //coordinates of vertical speed indication
    static short gssRightMarkX, gssLeftBigMarkX, gssLeftSmallMarkX, gssMarkerApexX, gssMarkerBaseX, gssLabelX; //Glide Slope bar coordinates
    static short gssScaleInPixelX2,gssStart,gssEnd,gssIncrement,gssIncrementX2; //Glide Slope bar pixel dimensions
    static short gssOOSupMarkerUpY,gssOOSupMarkerMidY,gssOOSupMarkerDwY,gssOOSdwMarkerUpY,gssOOSdwMarkerMidY,gssOOSdwMarkerDwY; //GS marker out of scale coordinates
    static short VSIscaleInPixel,VSIleftBorder,VSIrightBorder,VSItopBorder,VSIbottomBorder,vsiIncrement; //Vertical Situation Indicator bar coordinates
    static short VSIrightScaleBorder,VSIairplaneSymLeft,VSIairplaneSymRight,VSIairplaneSymTail,VSIlabelX,VSIlabelUpY,VSIlabelDwY;
    static short VSImarkerBaseX,vsiOOSdwMarkerUp,vsiOOSdwMarkerMid,vsiOOSdwMarkerDw,vsiOOSupMarkerUp,vsiOOSupMarkerMid,vsiOOSupMarkerDw;

    static struct { //Compass rose marks coordinates matrix: using short's to use less memory
        short extX, extY; //coordinates external mark point
        short intX, intY; //coordinates internal mark point
    } compassMarks[72][10]; //72 compass marks (one every 5 degrees), 10 possible cases
    static POINT hdgMark[4]; //Coordinates of heading marker (red triangle on the top of compass rose)

    static const double fiveNauticalMiles=(5.0*NAUTICALMILESTOMETRES); //Large Course Deviation Indicator scale: 5 nautical miles (9260 m)
    static const double smallCDIscale=(0.3*NAUTICALMILESTOMETRES); //Narrow (zoomed) CDI scale: 0.3 nautical miles (557 m)

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

    usingQFU = false;
    approach = false;
    landing = false;

    if(DoInit[MDI_DRAWHSI]) { //All the dimensions must be recalculated in case of screen resolution change
        centerX=(rc.right+rc.left)/2;
        centerY=(rc.bottom-BottomSize-InfoPageTopLineSeparator-TOPLIMITER)/2  +InfoPageTopLineSeparator+TOPLIMITER;
        if (ScreenLandscape)
            radius=centerY-InfoPageTopLineSeparator-NIBLSCALE(8);
        else
            radius=NIBLSCALE(70)-(rc.left+(ScreenSizeX-rc.right))/2;

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

        //Initialize position of Vertical speed indication
        if (ScreenSize==ss800x480 || ScreenSize==ss480x272) VertSpeedX=centerX+radius+NIBLSCALE(100);
        else VertSpeedX=rc.right-RIGHTLIMITER;

        VertSpeedLabelY=centerY-NIBLSCALE(28);
        VertSpeedValueY=centerY-NIBLSCALE(19);
        VertSpeedUnitY=centerY+NIBLSCALE(9);

        //Initialize coordinates for Glide Slope bar
        gssRightMarkX=centerX+radius+NIBLSCALE(ScreenLandscape?18:8);
        gssLeftBigMarkX=centerX+radius+NIBLSCALE(ScreenLandscape?11:3);
        gssLeftSmallMarkX=centerX+radius+NIBLSCALE(ScreenLandscape?13:5);
        gssMarkerApexX=centerX+radius+NIBLSCALE(ScreenLandscape?15:5);
        gssMarkerBaseX=centerX+radius+NIBLSCALE(ScreenLandscape?21:11);
        gssLabelX=centerX+radius+(ScreenLandscape?(NIBLSCALE(6)):(-NIBLSCALE(1)));
        gssScaleInPixelX2=NIBLSCALE(72)*2;
        gssStart=centerY-NIBLSCALE(72);
        gssEnd=centerY+NIBLSCALE(72);
        gssIncrement=NIBLSCALE(12);
        gssIncrementX2=gssIncrement*2;
        gssOOSupMarkerUpY=gssStart-NIBLSCALE(7);
        gssOOSupMarkerMidY=gssStart-NIBLSCALE(2);
        gssOOSupMarkerDwY=gssStart+NIBLSCALE(3);
        gssOOSdwMarkerUpY=gssStart+gssScaleInPixelX2-NIBLSCALE(3);
        gssOOSdwMarkerMidY=gssOOSdwMarkerUpY+NIBLSCALE(5);
        gssOOSdwMarkerDwY=gssOOSdwMarkerMidY+NIBLSCALE(5);

        //Initialize coordinates for Vertical Situation Indicator bar
        VSIscaleInPixel=ScreenLandscape?NIBLSCALE(70):NIBLSCALE(45);
        VSIleftBorder=centerX+radius+NIBLSCALE((ScreenSize==ss800x480 || ScreenSize==ss480x272)?8:3);
        VSIrightBorder=centerX+radius+NIBLSCALE((ScreenSize==ss800x480 || ScreenSize==ss480x272)?13:6);
        VSItopBorder=centerY-VSIscaleInPixel;
        VSIbottomBorder=centerY+VSIscaleInPixel;
        vsiIncrement=(short)round(VSIscaleInPixel/10);
        VSIrightScaleBorder=VSIrightBorder-1;
        VSIairplaneSymLeft=VSIleftBorder-3;
        VSIairplaneSymRight=VSIrightBorder+2;
        VSIairplaneSymTail=centerY-NIBLSCALE(2);
        VSIlabelX=VSIleftBorder+(ScreenLandscape?(NIBLSCALE(2)):(-NIBLSCALE(5)));
        VSIlabelUpY=VSItopBorder-NIBLSCALE(5);
        VSIlabelDwY=VSIbottomBorder+NIBLSCALE(5);
        VSImarkerBaseX=VSIrightBorder+NIBLSCALE((ScreenSize==ss800x480 || ScreenSize==ss480x272)?5:2);
        vsiOOSdwMarkerUp=VSIbottomBorder-NIBLSCALE(1);
        vsiOOSdwMarkerMid=VSIbottomBorder+NIBLSCALE(2);
        vsiOOSdwMarkerDw=VSIbottomBorder+NIBLSCALE(5);
        vsiOOSupMarkerUp=VSIbottomBorder-NIBLSCALE(5);
        vsiOOSupMarkerMid=VSIbottomBorder-NIBLSCALE(2);
        vsiOOSupMarkerDw=VSIbottomBorder+NIBLSCALE(1);

        DoInit[MDI_DRAWHSI]=false;
    }

    const auto hpOld = Surface.SelectObject(LKPen_Black_N1);
    const auto hbOld = Surface.SelectObject(LKBrush_Black);

    //get the track bearing
    int angle = 360-(int)round(DrawInfo.TrackBearing);

    //Draw the compass rose markers
    int alpha=angle%10;
    BUGSTOP_LKASSERT(alpha>=0 && alpha<10);
    if (angle<0) angle=0; // recovered
    POINT external,internal;
    for(int i=0,isBig=1;i<72;i++,isBig=!isBig) {
        external.x=compassMarks[i][alpha].extX;
        external.y=compassMarks[i][alpha].extY;
        internal.x=compassMarks[i][alpha].intX;
        internal.y=compassMarks[i][alpha].intY;
        Surface.DrawLine(PEN_SOLID,isBig?NIBLSCALE(1):1,external,internal,INVERTCOLORS?RGB_WHITE:RGB_BLACK,rc);
    }

    //Put the labels on compass rose
    Surface.SelectObject(LK8TitleFont);
    for(int i=0;i<12;i++) {
        int deg=i*30+angle;
        LKWriteText(Surface,label[i],centerX+(int)(labelsRadius*fastsine(deg)),centerY-(int)(labelsRadius*fastcosine(deg)),WTMODE_NORMAL,WTALIGN_CENTER,RGB_WHITE,false);
    }

    //Draw true heading mark on the top of the compass rose
    if (!IsDithered()) {
        Surface.SelectObject(LKPen_Red_N1);
        Surface.SelectObject(LKBrush_Red);
    } else {
        Surface.SelectObject(!INVERTCOLORS ? LKPen_Black_N1 : LKPen_White_N1);
        Surface.SelectObject(!INVERTCOLORS ? LKBrush_Black : LKBrush_White);
    }
    Surface.Polygon(hdgMark,4);

    //Print the current track indication
    TCHAR Buffer[LKSIZEBUFFERVALUE];
    Surface.SelectObject(LK8InfoSmallFont);
    _stprintf(Buffer, TEXT("%03d%s"),(int)round(DrawInfo.TrackBearing),MsgToken<2179>());
    if (!IsDithered()) {
        LKWriteText(Surface, Buffer, posTRKx, posTRKy, WTMODE_NORMAL, WTALIGN_CENTER, RGB_RED, false);
    } else {
        LKWriteText(Surface, Buffer, posTRKx, posTRKy, WTMODE_NORMAL, WTALIGN_CENTER, RGB_WHITE, false);
    }

    //Copies of the data needed from Task and WayPointList
    bool validActiveWP=false, validPreviousWP=false;
    int currentWP=0, finalWP=0, RunwayLen=0, QFU=0;
    double WPaltitude=0, prevWPaltitude=0, WPleg=0;
    short WPstyle;

    //Critical section: copy the needed data from Task and WayPoint list
    LockTaskData(); // protect Task & WayPointList
    if(ValidTaskPoint(ActiveTaskPoint)) { //if valid task
        if(Task[ActiveTaskPoint].Index>=0) { //if valid WP
            validActiveWP=true;
            currentWP=ActiveTaskPoint;
            finalWP=getFinalWaypoint();
            WPstyle=WayPointList[Task[ActiveTaskPoint].Index].Style;
            RunwayLen=WayPointList[Task[ActiveTaskPoint].Index].RunwayLen;
            WPaltitude= WayPointList[Task[ActiveTaskPoint].Index].Altitude;
            QFU=WayPointList[Task[ActiveTaskPoint].Index].RunwayDir; //get runaway orientation
            if(ActiveTaskPoint>0) { //if we have a previous WP
                if(Task[ActiveTaskPoint-1].Index>=0) {
                    validPreviousWP=true;
                    WPleg=Task[ActiveTaskPoint].Leg;
                    prevWPaltitude=WayPointList[Task[ActiveTaskPoint-1].Index].Altitude;
                }
            }
        }
    }
    UnlockTaskData();

    if(validActiveWP) { //Draw course direction and CDI only if there is a task/route active
        double course = DerivedDrawInfo.LegActualTrueCourse;
        double deviation = DerivedDrawInfo.LegCrossTrackError;
        if(finalWP==currentWP) { //if we are flying to the final destination
            approach=true;
            const double varioFtMin=DerivedDrawInfo.Vario*TOFEETPERMINUTE; //Convert vertical speed to Ft/min

            //Print vertical speed in Ft/min
            _tcscpy(Buffer,MsgToken<784>()); //"Vario"
            Surface.SelectObject(LK8PanelSmallFont);
            if (!IsDithered()) {
            	LKWriteText(Surface,Buffer,VertSpeedX,VertSpeedLabelY,WTMODE_NORMAL,WTALIGN_RIGHT,RGB_LIGHTGREEN,false);
            } else {
                LKWriteText(Surface, Buffer, VertSpeedX, VertSpeedLabelY, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);
            }
            _stprintf(Buffer, TEXT("%+.0f"),varioFtMin); //print the value
            if(!ScreenLandscape || (ScreenSize!=ss800x480 && ScreenSize!=ss480x272)) Surface.SelectObject(LK8PanelMediumFont);
            else Surface.SelectObject(LK8PanelBigFont);
            LKWriteText(Surface,Buffer,VertSpeedX,VertSpeedValueY,WTMODE_NORMAL,WTALIGN_RIGHT,RGB_WHITE,false);
            _stprintf(Buffer,TEXT("FPM")); //measure unit
            Surface.SelectObject(LK8PanelUnitFont);
            LKWriteText(Surface,Buffer,VertSpeedX,VertSpeedUnitY,WTMODE_NORMAL,WTALIGN_RIGHT,RGB_WHITE,false);
            if(DerivedDrawInfo.WaypointDistance<fiveNauticalMiles && WPstyle>=STYLE_AIRFIELDGRASS && WPstyle<=STYLE_AIRFIELDSOLID) { //if we are close to the destination airport
                if(DerivedDrawInfo.WaypointDistance<1500) landing=true; //if at less than 1.5 Km don't show glide slope bar
                else { //Build glide slope bar
                    //Calculate glide slope inclination to reach the destination runaway
                    const double halfRunaway=RunwayLen/2;
                    double distanceToRunaway=0;
                    if(DerivedDrawInfo.WaypointDistance>halfRunaway) distanceToRunaway=DerivedDrawInfo.WaypointDistance-halfRunaway;
                    double heightOnRunaway=DrawInfo.Altitude-WPaltitude;
                    if(heightOnRunaway<0) heightOnRunaway=0;
                    const double glideSlope=RAD_TO_DEG*atan2(heightOnRunaway,distanceToRunaway);
                    //double actualDescentAngle=0;
                    //if(DerivedDrawInfo.Vario<0) actualDescentAngle=RAD_TO_DEG*atan2(-DerivedDrawInfo.Vario,DrawInfo.Speed);

                    //Draw glide slope scale
                    external.x=gssRightMarkX;
                    for(int i=0,isBig=1;i<=12;i++,isBig=!isBig) {
                        internal.y=external.y=gssStart+gssIncrement*i;
                        internal.x=isBig?gssLeftBigMarkX:gssLeftSmallMarkX;
                        if (!IsDithered()) {
                            Surface.DrawLine(PEN_SOLID, isBig ? NIBLSCALE(1) : 1, internal, external, INVERTCOLORS ? RGB_LIGHTGREY : RGB_BLACK, rc);
                        } else {
                            Surface.DrawLine(PEN_SOLID, isBig ? NIBLSCALE(1) : 1, internal, external, INVERTCOLORS ? RGB_WHITE : RGB_BLACK, rc);
                        }
                    }

                    //Draw glide slope marker
                    POINT triangle[4];
                    triangle[0].x=triangle[3].x=gssMarkerApexX;
                    triangle[1].x=triangle[2].x=gssMarkerBaseX;
                    bool isOutOfScale=true;
                    if(glideSlope<=0) {
                        triangle[1].y=gssOOSupMarkerUpY;
                        triangle[0].y=triangle[3].y=gssOOSupMarkerMidY;
                        triangle[2].y=gssOOSupMarkerDwY;
                    } else if(glideSlope>6) {
                        triangle[1].y=gssOOSdwMarkerUpY;
                        triangle[0].y=triangle[3].y=gssOOSdwMarkerMidY;
                        triangle[2].y=gssOOSdwMarkerDwY;
                    } else { // 0 < glideSlope <= 6
                        triangle[0].y=triangle[3].y=gssStart+(int)round((glideSlope*gssScaleInPixelX2)/6);
                        triangle[1].y=triangle[0].y-NIBLSCALE(5);
                        triangle[2].y=triangle[0].y+NIBLSCALE(5);
                        isOutOfScale=false;
                    }
                    if(isOutOfScale) {
                        if (!IsDithered()) {
                            Surface.SelectObject(LKPen_Red_N1);
                            Surface.SelectObject(LKBrush_Red);
                        } else {
                            Surface.SelectObject(INVERTCOLORS ? LKPen_White_N1 : LKPen_Black_N1);
                            Surface.SelectObject(INVERTCOLORS ? LKBrush_White : LKBrush_Black);
                        }
                    } else if(INVERTCOLORS) {
                        Surface.SelectObject(LKPen_White_N1);
                        Surface.SelectObject(LKBrush_White);
                    } else {
                        Surface.SelectObject(LKPen_Black_N1);
                        Surface.SelectObject(LKBrush_Black);
                    }
                    Surface.Polygon(triangle,4);

                    //Put the labels on glide slope scale
                    Surface.SelectObject(LK8PanelSmallFont);
                    if (ScreenLandscape) {
                        for(int i=0;i<=6;i++) {
                            _stprintf(Buffer, TEXT("%d"),i);
                            if (!IsDithered()) {
                                LKWriteText(Surface,
                                            Buffer,
                                            gssLabelX,
                                            gssStart + gssIncrementX2 * i,
                                            WTMODE_NORMAL,
                                            WTALIGN_CENTER,
                                            isOutOfScale ? RGB_LIGHTRED : RGB_WHITE,
                                            false);
                            } else {
                                LKWriteText(Surface,
                                            Buffer,
                                            gssLabelX,
                                            gssStart + gssIncrementX2 * i,
                                            WTMODE_NORMAL,
                                            WTALIGN_CENTER,
                                            isOutOfScale ? RGB_WHITE : RGB_WHITE,
                                            false);
                            }
                        }
                    } else {
                        if (!IsDithered()) {
                            _stprintf(Buffer, TEXT("0"));
                            LKWriteText(Surface,
                                        Buffer,
                                        gssLabelX,
                                        gssStart,
                                        WTMODE_NORMAL,
                                        WTALIGN_CENTER,
                                        isOutOfScale ? RGB_LIGHTRED : RGB_WHITE,
                                        false);
                            _stprintf(Buffer, TEXT("6"));
                            LKWriteText(Surface,
                                        Buffer,
                                        gssLabelX,
                                        gssEnd,
                                        WTMODE_NORMAL,
                                        WTALIGN_CENTER,
                                        isOutOfScale ? RGB_LIGHTRED : RGB_WHITE,
                                        false);
                        } else {
                            _stprintf(Buffer, TEXT("0"));
                            LKWriteText(Surface,
                                        Buffer,
                                        gssLabelX,
                                        gssStart,
                                        WTMODE_NORMAL,
                                        WTALIGN_CENTER,
                                        isOutOfScale ? RGB_WHITE : RGB_WHITE,
                                        false);
                            _stprintf(Buffer, TEXT("6"));
                            LKWriteText(Surface,
                                        Buffer,
                                        gssLabelX,
                                        gssEnd,
                                        WTMODE_NORMAL,
                                        WTALIGN_CENTER,
                                        isOutOfScale ? RGB_WHITE : RGB_WHITE,
                                        false);
                        }
                    }
                } //end of glide slope bar
            }

            //Determine if to give HSI indication respect destination runaway (QFU)
            if(finalWP==0 || DerivedDrawInfo.WaypointDistance<fiveNauticalMiles) {//if direct GOTO or below 5 NM
                if(QFU>0 && QFU<=360) { //valid QFU
                    usingQFU=true;
                    course=QFU;
                    deviation=DerivedDrawInfo.WaypointDistance*fastsine(AngleDifference(course,DerivedDrawInfo.WaypointBearing)); //flat cross track error
                }
            }
        }

        //Print the desired course
        Surface.SelectObject(LK8InfoSmallFont);
        _stprintf(Buffer, TEXT("%03d%s"),(int)round(course),MsgToken<2179>());
        if (!IsDithered()) {
            LKWriteText(Surface, Buffer, posDTKx, posTRKy, WTMODE_NORMAL, WTALIGN_CENTER, RGB_GREEN, false);
        } else {
            LKWriteText(Surface, Buffer, posDTKx, posTRKy, WTMODE_NORMAL, WTALIGN_CENTER, RGB_WHITE, false);
        }

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
        if (!IsDithered()) {
            Surface.DrawLine(PEN_SOLID, NIBLSCALE(2), up, down, RGB_GREEN, rc);
        } else {
            Surface.DrawLine(PEN_SOLID, NIBLSCALE(2), up, down, INVERTCOLORS ? RGB_WHITE : RGB_BLACK, rc);
        }
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
        if (!IsDithered()) {
            Surface.SelectObject(LKPen_Green_N1);
            Surface.SelectObject(LKBrush_Green);
        } else {
            Surface.SelectObject(INVERTCOLORS ? LKPen_White_N1 : LKPen_Black_N1);
            Surface.SelectObject(INVERTCOLORS ? LKBrush_White : LKBrush_Black);
        }
        Surface.Polygon(triangle,4);

        //This is the opposite side of the course direction arrow
        up.x=centerX-innerradiusXsin;
        up.y=centerY+innerradiusXcos;
        down.x=centerX-cdiRadiusXsin;
        down.y=centerY+cdiRadiusXcos;
        if (!IsDithered()) {
            Surface.DrawLine(PEN_SOLID, NIBLSCALE(2), up, down, RGB_GREEN, rc);
        } else {
            Surface.DrawLine(PEN_SOLID, NIBLSCALE(2), up, down, INVERTCOLORS ? RGB_WHITE : RGB_BLACK, rc);
        }

        //Course Deviation Indicator
        if(currentWP>0 || usingQFU) { //we are flying on a predefined routeline or we have the info for landing: draw CDI
            int dev; //deviation in pixel
            LKColor cdiColor;
            if (!IsDithered()) {
              cdiColor = INVERTCOLORS ? RGB_YELLOW : RGB_DARKYELLOW; //color of CDI
            } else {
              cdiColor = INVERTCOLORS ? RGB_WHITE : RGB_BLACK; //color of CDI
            }
            Surface.SelectObject(INVERTCOLORS?LKPen_White_N1:LKPen_Black_N1); //color of CDI scale
            Surface.SelectObject(INVERTCOLORS?LKBrush_White:LKBrush_Black);
            if(abs((int)deviation)<smallCDIscale) { //use small scale of 0.3 NM
                for(int i=1;i<=3;i++) {
                    long tickXiXsin=(long)(smallScaleTick*i*sin);
                    long tickXiXcos=(long)(smallScaleTick*i*cos);
                    Surface.DrawCircle(centerX+tickXiXcos,centerY+tickXiXsin,NIBLSCALE(1),false);
                    Surface.DrawCircle(centerX-tickXiXcos,centerY-tickXiXsin,NIBLSCALE(1),false);
                }
                dev=-(int)(round((cdiFullScale*deviation)/smallCDIscale));
            } else { // use big scale of 5 NM
                for(int i=1;i<=5;i++) {
                    long tickXiXsin=(long)(bigScaleTick*i*sin);
                    long tickXiXcos=(long)(bigScaleTick*i*cos);
                    Surface.DrawCircle(centerX+tickXiXcos,centerY+tickXiXsin,NIBLSCALE(1),false);
                    Surface.DrawCircle(centerX-tickXiXcos,centerY-tickXiXsin,NIBLSCALE(1),false);
                }
                if(deviation>fiveNauticalMiles) { //The larger CDI scale is of 5 nautical miles
                    dev=-cdiFullScale;
                    if (!IsDithered()) {
                      cdiColor = RGB_RED;
                    } else {
                      cdiColor = INVERTCOLORS ? RGB_WHITE : RGB_BLACK;
                    }
                } else if(deviation<-fiveNauticalMiles) {
                    dev=cdiFullScale;
                    if (!IsDithered()) {
                        cdiColor = RGB_RED;
                    } else {
                        cdiColor = INVERTCOLORS ? RGB_WHITE : RGB_BLACK;
                    }
                } else dev=-(int)round((cdiFullScale*deviation)/fiveNauticalMiles);
            }
            const long devXsin=(long)(dev*sin);
            const long devXcos=(long)(dev*cos);
            up.x=centerX+cdiRadiusXsin+devXcos;
            up.y=centerY-cdiRadiusXcos+devXsin;
            down.x=centerX-cdiRadiusXsin+devXcos;
            down.y=centerY+cdiRadiusXcos+devXsin;
            Surface.DrawLine(PEN_SOLID,NIBLSCALE(2),up,down,cdiColor,rc);

            //Print the actual cross track error
            Surface.SelectObject(LK8InfoSmallFont);
            double xtk=fabs(deviation); //here is in meters
            if(DistanceUnit_Config==2) { //Km
                if(xtk>1000) {
                     xtk*=TOKILOMETER;
                     if(xtk<=99.9) _stprintf(Buffer, TEXT("%.1f km"),xtk);
                     else _stprintf(Buffer,TEXT("%d km"),(int)round(xtk));
                 } else _stprintf(Buffer,TEXT("%d m"),(int)round(xtk));
            } else { //Miles or Nautical miles
                xtk*=DISTANCEMODIFY;
                if(xtk<1) _stprintf(Buffer, TEXT("%.2f %s"),xtk,Units::GetDistanceName());
                else if(xtk<=99.9) _stprintf(Buffer, TEXT("%.1f %s"),xtk,Units::GetDistanceName());
                else _stprintf(Buffer,TEXT("%d %s"),(int)round(xtk),Units::GetDistanceName());
            }
            if (!IsDithered()) {
                LKWriteText(Surface, Buffer, posXTKx, posXTKy + NIBLSCALE(2), WTMODE_NORMAL, WTALIGN_CENTER, cdiColor, false);
            } else {
                LKWriteText(Surface, Buffer, posXTKx, posXTKy + NIBLSCALE(2), WTMODE_NORMAL, WTALIGN_CENTER, RGB_WHITE, false);
            }

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
            LKColor colorPen,colorBrush;
            if (!IsDithered()) {
                colorPen = RGB_MAGENTA;
                colorBrush = RGB_MAGENTA;
            } else {
                colorPen = INVERTCOLORS?RGB_WHITE:RGB_BLACK;
                colorBrush = INVERTCOLORS?RGB_WHITE:RGB_BLACK;
            }
            LKPen PenViola(PEN_SOLID,NIBLSCALE(1),colorPen);
            LKBrush BrushViola(colorBrush);
            const auto OldPen = Surface.SelectObject(PenViola);
            const auto OldBrush = Surface.SelectObject(BrushViola);
            Surface.Polygon(triangle,4);
            Surface.SelectObject(OldPen);
            Surface.SelectObject(OldBrush);

            //Print the actual bearing to next WayPoint
            Surface.SelectObject(LK8InfoSmallFont);
            _stprintf(Buffer, TEXT("%03d%s"),(int)round(DerivedDrawInfo.WaypointBearing),MsgToken<2179>());
            if (!IsDithered()) {
                LKWriteText(Surface, Buffer, posDTKx, posBRGy + NIBLSCALE(2), WTMODE_NORMAL, WTALIGN_CENTER, RGB_MAGENTA, false);
            } else {
                LKWriteText(Surface, Buffer, posDTKx, posBRGy + NIBLSCALE(2), WTMODE_NORMAL, WTALIGN_CENTER, RGB_WHITE, false);
            }
        } else { //flying to the departure or a direct GOTO without information for landing: don't draw CDI
            //Draw anyway the CDI scale in grey (disabled)
            Surface.SelectObject(LKPen_Grey_N1); //color of CDI scale
            Surface.SelectObject(LKBrush_Grey);
            for(int i=1;i<=3;i++) {
                const int tickXi=smallScaleTick*i;
                const int tickXiXsin=(int)round(tickXi*sin);
                const int tickXiXcos=(int)round(tickXi*cos);
                Surface.DrawCircle(centerX+tickXiXcos,centerY+tickXiXsin,NIBLSCALE(1),false);
                Surface.DrawCircle(centerX-tickXiXcos,centerY-tickXiXsin,NIBLSCALE(1),false);
            }

            //draw CDI in the center as part of the course direction arrow (same color)
            up.x=centerX+cdiRadiusXsin;
            up.y=centerY-cdiRadiusXcos;
            down.x=centerX-cdiRadiusXsin;
            down.y=centerY+cdiRadiusXcos;
            if (!IsDithered()) {
                Surface.DrawLine(PEN_SOLID, NIBLSCALE(2), up, down, RGB_GREEN, rc);
            } else {
                Surface.DrawLine(PEN_SOLID, NIBLSCALE(2), up, down, INVERTCOLORS ? RGB_WHITE : RGB_BLACK, rc);
            }
        }
    }

    //Draw airplane symbol in the center of HSI
    if (!IsDithered()) {
        Surface.DrawLine(PEN_SOLID, NIBLSCALE(2), fusA, fusB, RGB_ORANGE, rc);
        Surface.DrawLine(PEN_SOLID, NIBLSCALE(2), winA, winB, RGB_ORANGE, rc);
        Surface.DrawLine(PEN_SOLID, NIBLSCALE(2), taiA, taiB, RGB_ORANGE, rc);
    } else {
        Surface.DrawLine(PEN_SOLID, NIBLSCALE(2), fusA, fusB, INVERTCOLORS ? RGB_WHITE : RGB_BLACK, rc);
        Surface.DrawLine(PEN_SOLID, NIBLSCALE(2), winA, winB, INVERTCOLORS ? RGB_WHITE : RGB_BLACK, rc);
        Surface.DrawLine(PEN_SOLID, NIBLSCALE(2), taiA, taiB, INVERTCOLORS ? RGB_WHITE : RGB_BLACK, rc);
    }

    //Draw VSI: Vertical Situation Indicator...
    if(!usingQFU && !landing && validPreviousWP) { //... only if not using glide slope and not lading
        //Calculate expected altitude in route
        double expectedAlt=WPaltitude;
        if(WPleg>0 && WPaltitude!=prevWPaltitude) expectedAlt=(WPaltitude-prevWPaltitude)/WPleg*DerivedDrawInfo.LegDistanceCovered+prevWPaltitude;
        expectedAlt*=TOFEET;
        const double diff=(DrawInfo.Altitude*TOFEET)-expectedAlt; //difference with current altitude

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
        const double pixelPerFeet=VSIscaleInPixel/scale;

        //Draw VSI background
        if(DerivedDrawInfo.TerrainValid) {
            const double altAGLft=DerivedDrawInfo.AltitudeAGL*TOFEET;
            int groundLevel=VSIscaleInPixel;
            if(fabs(altAGLft)<scale) groundLevel=(int)round(altAGLft*pixelPerFeet);
            else if(altAGLft<0) groundLevel=-VSIscaleInPixel;
            if(groundLevel>-VSIscaleInPixel) { //sky part
                LKPen PenSky(PEN_SOLID,NIBLSCALE(1),LKColor(0,153,153));
                LKBrush BrushSky(LKColor(0,153,153));
                const auto oldPen = Surface.SelectObject(PenSky);
                Surface.SelectObject(BrushSky);
                Surface.Rectangle(VSIleftBorder,centerY+groundLevel,VSIrightBorder,VSItopBorder);
                Surface.SelectObject(oldPen);
            }
            if(groundLevel<VSIscaleInPixel) { //ground part
                LKPen PenGround(PEN_SOLID,NIBLSCALE(1),LKColor(204,102,0));
                LKBrush BrushGround(LKColor(204,102,0));
                const auto oldPen = Surface.SelectObject(PenGround);
                Surface.SelectObject(BrushGround);
                Surface.Rectangle(VSIleftBorder,VSIbottomBorder,VSIrightBorder,centerY+groundLevel+1);
                Surface.SelectObject(oldPen);
            }
        }

        //Draw VSI scale
        internal.x=VSIleftBorder;
        external.x=VSIrightScaleBorder;
        for(int i=1,isBig=0;i<=10;i++,isBig=!isBig) {
            internal.y=external.y=centerY-vsiIncrement*i; //upper part
            Surface.DrawLine(PEN_SOLID,isBig?NIBLSCALE(1):1,internal,external,INVERTCOLORS?RGB_WHITE:RGB_BLACK,rc);
            internal.y=external.y=centerY+vsiIncrement*i; //lower part
            Surface.DrawLine(PEN_SOLID,isBig?NIBLSCALE(1):1,internal,external,INVERTCOLORS?RGB_WHITE:RGB_BLACK,rc);
        }

        //Draw airplane symbol at the center of VSI scale
        internal.x=VSIairplaneSymLeft;
        external.x=VSIairplaneSymRight;
        internal.y=external.y=centerY;
        Surface.DrawLine(PEN_SOLID,NIBLSCALE(1),internal,external,INVERTCOLORS?RGB_WHITE:RGB_BLACK,rc);
        internal.x=external.x=internal.x+(external.x-internal.x)/2;
        external.y=VSIairplaneSymTail;
        Surface.DrawLine(PEN_SOLID,ScreenThinSize,internal,external,INVERTCOLORS?RGB_WHITE:RGB_BLACK,rc);
        Surface.SelectObject(INVERTCOLORS?LKPen_White_N1:LKPen_Black_N1);
        Surface.DrawCircle(internal.x,centerY,NIBLSCALE(1),false);

        //Print +scale and -scale at top and bottom of VSI
        Surface.SelectObject(LK8SmallFont);
        if(scale<10000) _stprintf(Buffer, TEXT("+%.0fft"),scale);
        else _stprintf(Buffer, TEXT("+%.0ff"),scale);
        if (!IsDithered()) {
            LKWriteText(Surface, Buffer, VSIlabelX, VSIlabelUpY, WTMODE_NORMAL, WTALIGN_CENTER, outOfScale ? RGB_LIGHTRED : RGB_WHITE, false);
            if (scale < 10000) _stprintf(Buffer, TEXT("-%.0fft"), scale);
            else
                _stprintf(Buffer, TEXT("-%.0ff"), scale);
            LKWriteText(Surface, Buffer, VSIlabelX, VSIlabelDwY, WTMODE_NORMAL, WTALIGN_CENTER, outOfScale ? RGB_LIGHTRED : RGB_WHITE, false);
        } else {
            LKWriteText(Surface, Buffer, VSIlabelX, VSIlabelUpY, WTMODE_NORMAL, WTALIGN_CENTER, outOfScale ? RGB_WHITE : RGB_WHITE, false);
            if (scale < 10000) _stprintf(Buffer, TEXT("-%.0fft"), scale);
            else
                _stprintf(Buffer, TEXT("-%.0ff"), scale);
            LKWriteText(Surface, Buffer, VSIlabelX, VSIlabelDwY, WTMODE_NORMAL, WTALIGN_CENTER, outOfScale ? RGB_WHITE : RGB_WHITE, false);
        }

        //Draw expected in route altitude marker
        POINT triangle[4];
        triangle[0].x=triangle[3].x=VSIleftBorder;
        triangle[1].x=triangle[2].x=VSImarkerBaseX;
        if(outOfScale) {
            if(diff>0) { //down
                triangle[1].y=vsiOOSdwMarkerUp;
                triangle[0].y=triangle[3].y=vsiOOSdwMarkerMid;
                triangle[2].y=vsiOOSdwMarkerDw;
            } else { //up
                triangle[1].y=vsiOOSupMarkerUp;
                triangle[0].y=triangle[3].y=vsiOOSupMarkerMid;
                triangle[2].y=vsiOOSupMarkerDw;
            }
        } else {
            triangle[0].y=triangle[3].y=centerY+(int)round(pixelPerFeet*diff);
            triangle[1].y=triangle[0].y-NIBLSCALE(3);
            triangle[2].y=triangle[0].y+NIBLSCALE(3);
        }
        if(outOfScale) {
          if (!IsDithered()) {
            Surface.SelectObject(LKPen_Red_N1);
            Surface.SelectObject(LKBrush_Red);
          } else {
            Surface.SelectObject(INVERTCOLORS ? LKPen_White_N1 : LKPen_Black_N1);
            Surface.SelectObject(INVERTCOLORS ? LKBrush_White : LKBrush_Black);
          }
        } else if(INVERTCOLORS) {
            Surface.SelectObject(LKPen_White_N1);
            Surface.SelectObject(LKBrush_White);
        } else {
            Surface.SelectObject(LKPen_Black_N1);
            Surface.SelectObject(LKBrush_Black);
        }
        Surface.Polygon(triangle,4);
    }

    Surface.SelectObject(hbOld);
    Surface.SelectObject(hpOld);
}
