/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RGB.h"


//
// The heading track line, like on Garmin units
//
void MapWindow::DrawHeading(HDC hdc, POINT Orig, RECT rc ) {
    if(DrawInfo.NAVWarning) return; // 100214
    if(mode.Is(MapWindow::Mode::MODE_CIRCLING)) return;

    POINT p2;
    double tmp = 200000*zoom.ResScaleOverDistanceModify();
    if (!(DisplayOrientation == TRACKUP || DisplayOrientation == NORTHCIRCLE || DisplayOrientation == TRACKCIRCLE)) {
        p2.y= Orig.y - (int)(tmp*fastcosine(DrawInfo.TrackBearing));
        p2.x= Orig.x + (int)(tmp*fastsine(DrawInfo.TrackBearing));
    } else {
        p2.x=Orig.x;
        p2.y=Orig.y - (int)tmp;
    }

    // Reduce the rectangle for a better effect
    rc.top+=NIBLSCALE(5);
    rc.right-=NIBLSCALE(5);
    rc.bottom-=NIBLSCALE(5);
    rc.left+=NIBLSCALE(5);

    ForcedClipping=true;
    _DrawLine(hdc, PS_SOLID, NIBLSCALE(1), Orig, p2, BlackScreen ? RGB_INVDRAW : RGB_BLACK, rc); // 091109

    // Draw projection of the position in 2, 5 and 10 min at current speed, like SkyDemon
    if(ISGAAIRCRAFT&&DrawInfo.Speed>13.88) { //Only for GA and if ground speed > 50 Km/h
        double dist2min=120*DrawInfo.Speed*zoom.ResScaleOverDistanceModify(); // 2 min
        if(dist2min>=NIBLSCALE(3)) { //proceed only if the distance is not too small on the map
            double dist5min=300*DrawInfo.Speed*zoom.ResScaleOverDistanceModify(); // 5 min
            double dist10min=600*DrawInfo.Speed*zoom.ResScaleOverDistanceModify(); //10 min
            POINT p1;
            if(DisplayOrientation==TRACKUP || DisplayOrientation==NORTHCIRCLE || DisplayOrientation==TRACKCIRCLE) {
                p1.x=Orig.x-NIBLSCALE(4);
                p2.x=Orig.x+NIBLSCALE(4);
                p1.y=p2.y=Orig.y-(int)round(dist2min);
                _DrawLine(hdc,PS_SOLID,NIBLSCALE(1),p1,p2,BlackScreen?RGB_INVDRAW:RGB_BLACK,rc);
                p1.y=p2.y=Orig.y-(int)round(dist5min);
                _DrawLine(hdc,PS_SOLID,NIBLSCALE(1),p1,p2,BlackScreen?RGB_INVDRAW:RGB_BLACK,rc);
                p1.y=p2.y=Orig.y-(int)round(dist10min);
                _DrawLine(hdc,PS_SOLID,NIBLSCALE(1),p1,p2,BlackScreen?RGB_INVDRAW:RGB_BLACK,rc);
            } else { //North up map view
                double sin=fastsine(DrawInfo.TrackBearing);
                double cos=fastcosine(DrawInfo.TrackBearing);
                double distXsin=dist2min*sin;
                double distXcos=dist2min*cos;
                double tickXsin=NIBLSCALE(4)*sin;
                double tickXcos=NIBLSCALE(4)*cos;
                p1.x=Orig.x+(int)round(distXsin-tickXcos);
                p1.y=Orig.y-(int)round(distXcos+tickXsin);
                p2.x=Orig.x+(int)round(distXsin+tickXcos);
                p2.y=Orig.y-(int)round(distXcos-tickXsin);
                _DrawLine(hdc,PS_SOLID,NIBLSCALE(1),p1,p2,BlackScreen?RGB_INVDRAW:RGB_BLACK,rc);
                distXsin=dist5min*sin;
                distXcos=dist5min*cos;
                p1.x=Orig.x+(int)round(distXsin-tickXcos);
                p1.y=Orig.y-(int)round(distXcos+tickXsin);
                p2.x=Orig.x+(int)round(distXsin+tickXcos);
                p2.y=Orig.y-(int)round(distXcos-tickXsin);
                _DrawLine(hdc,PS_SOLID,NIBLSCALE(1),p1,p2,BlackScreen?RGB_INVDRAW:RGB_BLACK,rc);
                distXsin=dist10min*sin;
                distXcos=dist10min*cos;
                p1.x=Orig.x+(int)round(distXsin-tickXcos);
                p1.y=Orig.y-(int)round(distXcos+tickXsin);
                p2.x=Orig.x+(int)round(distXsin+tickXcos);
                p2.y=Orig.y-(int)round(distXcos-tickXsin);
                _DrawLine(hdc,PS_SOLID,NIBLSCALE(1),p1,p2,BlackScreen?RGB_INVDRAW:RGB_BLACK,rc);
            }
        }
    }
    ForcedClipping=false;
}
