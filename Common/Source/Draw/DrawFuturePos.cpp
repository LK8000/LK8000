/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   DrawFuturePos.cpp
 * Author: Alberto Realis-Luc
 * Description: Draw projection of expected position on heading line in 2, 5 and 10 minutes at current speed, like SkyDemon
 *
 * Created on May 11, 2014
 */


#include "externs.h"
#include "RGB.h"

void MapWindow::DrawFuturePos(LKSurface& Surface, const POINT& Orig, const RECT& rc, bool headUpLine) {
    if(DrawInfo.Speed < 13.88) return; //Don't continue if ground speed < 50 Km/h
    const double trackBearing = headUpLine ? DisplayAircraftAngle+(DerivedDrawInfo.Heading-DrawInfo.TrackBearing) : DrawInfo.TrackBearing;
    const double dist2min=120*DrawInfo.Speed*zoom.ResScaleOverDistanceModify(); // 2 min
    if(dist2min>=NIBLSCALE(3)) { //proceed only if the distance is not too small on the map
        const double dist5min=300*DrawInfo.Speed*zoom.ResScaleOverDistanceModify(); // 5 min
        const double dist10min=600*DrawInfo.Speed*zoom.ResScaleOverDistanceModify(); //10 min
        
        // Reduce the rectangle for a better effect
        const RECT ClipRect = (RECT){rc.left+NIBLSCALE(5), rc.top+NIBLSCALE(5), rc.right-NIBLSCALE(5), rc.bottom-NIBLSCALE(5) };

        POINT p1,p2;

        if( !MapWindow::mode.autoNorthUP() && !headUpLine && (DisplayOrientation==TRACKUP || DisplayOrientation==NORTHCIRCLE || DisplayOrientation==TARGETCIRCLE || DisplayOrientation==TARGETUP)) { //Track up map view
            p1.x=Orig.x-NIBLSCALE(4);
            p2.x=Orig.x+NIBLSCALE(4);
            p1.y=p2.y=Orig.y-(int)round(dist2min);
            Surface.DrawLine(PEN_SOLID,NIBLSCALE(1),p1,p2,BlackScreen?RGB_INVDRAW:RGB_BLACK,ClipRect);
            p1.y=p2.y=Orig.y-(int)round(dist5min);
            Surface.DrawLine(PEN_SOLID,NIBLSCALE(1),p1,p2,BlackScreen?RGB_INVDRAW:RGB_BLACK,ClipRect);
            p1.y=p2.y=Orig.y-(int)round(dist10min);
            Surface.DrawLine(PEN_SOLID,NIBLSCALE(1),p1,p2,BlackScreen?RGB_INVDRAW:RGB_BLACK,ClipRect);
        } else { //North up map view
            const double sin=fastsine(trackBearing);
            const double cos=fastcosine(trackBearing);
            double distXsin=dist2min*sin;
            double distXcos=dist2min*cos;
            const double tickXsin=NIBLSCALE(4)*sin;
            const double tickXcos=NIBLSCALE(4)*cos;
            p1.x=Orig.x+(int)round(distXsin-tickXcos);
            p1.y=Orig.y-(int)round(distXcos+tickXsin);
            p2.x=Orig.x+(int)round(distXsin+tickXcos);
            p2.y=Orig.y-(int)round(distXcos-tickXsin);
            Surface.DrawLine(PEN_SOLID,NIBLSCALE(1),p1,p2,BlackScreen?RGB_INVDRAW:RGB_BLACK,ClipRect);
            distXsin=dist5min*sin;
            distXcos=dist5min*cos;
            p1.x=Orig.x+(int)round(distXsin-tickXcos);
            p1.y=Orig.y-(int)round(distXcos+tickXsin);
            p2.x=Orig.x+(int)round(distXsin+tickXcos);
            p2.y=Orig.y-(int)round(distXcos-tickXsin);
            Surface.DrawLine(PEN_SOLID,NIBLSCALE(1),p1,p2,BlackScreen?RGB_INVDRAW:RGB_BLACK,ClipRect);
            distXsin=dist10min*sin;
            distXcos=dist10min*cos;
            p1.x=Orig.x+(int)round(distXsin-tickXcos);
            p1.y=Orig.y-(int)round(distXcos+tickXsin);
            p2.x=Orig.x+(int)round(distXsin+tickXcos);
            p2.y=Orig.y-(int)round(distXcos-tickXsin);
            Surface.DrawLine(PEN_SOLID,NIBLSCALE(1),p1,p2,BlackScreen?RGB_INVDRAW:RGB_BLACK,ClipRect);
        }
    }
}

