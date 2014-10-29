/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKInterface.h"
#include "LKObjects.h"


//
// Final Glide Bar, revised for variometer gauge
//
void MapWindow::DrawFinalGlide(LKSurface& Surface, const RECT& rc)
{

  SIZE           TextSize;

  if ((GlideBarMode == (GlideBarMode_t)gbDisabled)) {
	GlideBarOffset=0;
	return;
  }

  POINT GlideBar[6] = { {0,0},{9,-9},{18,0},{18,0},{9,0},{0,0} };
  POINT GlideBar0[6] = { {0,0},{9,-9},{18,0},{18,0},{9,0},{0,0} };
  
  LKPen hpOld;
  LKBrush hbOld;
  
  TCHAR Value[10];
  
  int Offset;
  int Offset0;
  int i;
  int lkVarioOffset=0, minBar, maxBar;

  if (LKVarioBar)
	lkVarioOffset=LKVarioSize+NIBLSCALE(2); //@ 091114

  // 091114
  switch(ScreenSize) {
	case (ScreenSize_t)ss480x234:
	case (ScreenSize_t)ss480x272:
	case (ScreenSize_t)ss720x408:
		minBar=-40;
		maxBar=40;
		break;
	case (ScreenSize_t)ss800x480:
	case (ScreenSize_t)ss400x240:
		minBar=-45;
		maxBar=45;
                break;
	default:
		minBar=-50; // was 60
		maxBar=50;
		break;
  }
  
  LockTaskData();  // protect from external task changes

  bool invalidbar=false;
  #if 101004
  int barindex;
  barindex=GetOvertargetIndex();
  if (barindex>=0) {
  #else
  if (ValidTaskPoint(ActiveWayPoint)){
  #endif

	const int y0 = ( (rc.bottom - rc.top )/2)+rc.top;

	#if 110609
	if ( ValidTaskPoint(1) && OvertargetMode == OVT_TASK && GlideBarMode == (GlideBarMode_t)gbFinish ) {
		// Before the start, there is no task altitude available!
		if (DerivedDrawInfo.ValidStart) {
		    Offset = ((int)DerivedDrawInfo.TaskAltitudeDifference)/8; 
		    Offset0 = ((int)DerivedDrawInfo.TaskAltitudeDifference0)/8; 
		} else {
                    // In this case, print an invalid bar
		    invalidbar=true;
		    Offset=0;
		    Offset0=0;
		}
	} else {
		Offset=(int)WayPointCalc[barindex].AltArriv[AltArrivMode];
		Offset0=Offset;
	}
	#else
	// This is wrong, we are painting values relative to MC and ignoring safetyMC
	if (OvertargetMode != OVT_TASK) { //@ 101004
		Offset=(int)WayPointCalc[barindex].AltArriv[AltArrivMode];
		Offset0=Offset;
	} else {
		// 60 units is size, div by 8 means 60*8 = 480 meters.
		if ( (GlideBarMode == (GlideBarMode_t)gbFinish)) {
			Offset = ((int)DerivedDrawInfo.TaskAltitudeDifference)/8; 
			Offset0 = ((int)DerivedDrawInfo.TaskAltitudeDifference0)/8; 
		} else {
			Offset = ((int)DerivedDrawInfo.NextAltitudeDifference)/8; 
			Offset0 = ((int)DerivedDrawInfo.NextAltitudeDifference0)/8; 
		}
	}
	#endif

	// TODO feature: should be an angle if in final glide mode

	if(Offset > maxBar) Offset = maxBar;
	if(Offset < minBar) Offset = minBar;
	Offset = IBLSCALE(Offset);
	if(Offset<0) {
		GlideBar[1].y = NIBLSCALE(9);
	}
      
	if(Offset0 > maxBar) Offset0 = maxBar;
	if(Offset0 < minBar) Offset0 = minBar;
	Offset0 = IBLSCALE(Offset0);
	if(Offset0<0) {
		GlideBar0[1].y = NIBLSCALE(9);
	}
 
	for(i=0;i<6;i++) {
		GlideBar[i].y += y0;
		// if vario activated
		GlideBar[i].x = IBLSCALE(GlideBar[i].x)+rc.left+lkVarioOffset; //@ 091114
	}
	GlideBar[0].y -= Offset;
	GlideBar[1].y -= Offset;
	GlideBar[2].y -= Offset;

	for(i=0;i<6;i++) {
		GlideBar0[i].y += y0;
		GlideBar0[i].x = IBLSCALE(GlideBar0[i].x)+rc.left+lkVarioOffset; //@ 091114
	}
	GlideBar0[0].y -= Offset0;
	GlideBar0[1].y -= Offset0;
	GlideBar0[2].y -= Offset0;

	if ((Offset<0)&&(Offset0<0)) {
		// both below
		if (Offset0!= Offset) {
			int dy = (GlideBar0[0].y-GlideBar[0].y) +(GlideBar0[0].y-GlideBar0[3].y); dy = max(NIBLSCALE(3), dy);
			GlideBar[3].y = GlideBar0[0].y-dy;
			GlideBar[4].y = GlideBar0[1].y-dy;
			GlideBar[5].y = GlideBar0[2].y-dy;
          
			GlideBar0[0].y = GlideBar[3].y;
			GlideBar0[1].y = GlideBar[4].y;
			GlideBar0[2].y = GlideBar[5].y;
		} else {
			Offset0 = 0;
		}

	} else if ((Offset>0)&&(Offset0>0)) {
		// both above
		GlideBar0[3].y = GlideBar[0].y;
		GlideBar0[4].y = GlideBar[1].y;
		GlideBar0[5].y = GlideBar[2].y;

		if (abs(Offset0-Offset)<NIBLSCALE(4)) {
			Offset= Offset0;
		} 
	}

	// draw actual glide bar

	if (Offset<=0) {
		hpOld = Surface.SelectObject(hpFinalGlideBelow);
		hbOld = Surface.SelectObject(LKBrush_Red);
	} else {
		hpOld = Surface.SelectObject(hpFinalGlideAbove);
		hbOld = Surface.SelectObject(LKBrush_Green);
	}
	Surface.Polygon(GlideBar,6);

	// in case of invalid bar because finish mode with real task but no valid start, we skip
	if (invalidbar) {
	    _tcscpy(Value,_T("---"));
            goto _skipout;
        }

	// draw glide bar at mc 0 and X  only for OVT_TASK 101004
	// we dont have mc0 calc ready for other overtargets, not granted at least
	if (OvertargetMode == OVT_TASK) {
		if (Offset0<=0) {
			Surface.SelectObject(hpFinalGlideBelow);
			Surface.SelectObject(LKBrush_Hollow);
		} else {
			Surface.SelectObject(hpFinalGlideAbove);
			Surface.SelectObject(LKBrush_Hollow);
		}
		if (Offset!=Offset0) {
			Surface.Polygon(GlideBar0,6);
		}


		// Draw an X  on final glide bar if unreachable at current Mc
		if ( (GlideBarMode == (GlideBarMode_t)gbFinish) ) {
			if ((DerivedDrawInfo.TaskTimeToGo>0.9*ERROR_TIME) || 
			((MACCREADY<0.01) && (DerivedDrawInfo.TaskAltitudeDifference<0))) {
				Surface.SelectObject(LKPen_White_N2);
				POINT Cross[4] = { {-5, -5}, { 5,  5}, {-5,  5}, { 5, -5} };
				for (i=0; i<4; i++) {
					Cross[i].x = IBLSCALE(Cross[i].x+9)+lkVarioOffset; //@ 091114
					Cross[i].y = IBLSCALE(Cross[i].y+9)+y0;
				}
				Surface.Polyline(&Cross[0],2);
                Surface.Polyline(&Cross[2],2);
			}
		} else {
			if ((MACCREADY<0.01) && (DerivedDrawInfo.NextAltitudeDifference<0)) {
				Surface.SelectObject(LKPen_White_N2);
				POINT Cross[4] = { {-5, -5}, { 5,  5}, {-5,  5}, { 5, -5} };
				for (i=0; i<4; i++) {
					Cross[i].x = IBLSCALE(Cross[i].x+9)+lkVarioOffset;
					Cross[i].y = IBLSCALE(Cross[i].y+9)+y0;
				}
				Surface.Polyline(&Cross[0],2);
				Surface.Polyline(&Cross[2],2);
			}
		}
	}

	// draw boxed value in the center
		if (OvertargetMode == OVT_TASK ) { //@ 101004
			// A task is made of at least 2 tps, otherwise its a goto
			if (( (GlideBarMode == (GlideBarMode_t)gbFinish) && ValidTaskPoint(1)) ) {
                if(ISPARAGLIDER && DerivedDrawInfo.TaskAltitudeDifference > 0.0) {
                    if ( (ALTITUDEMODIFY*DerivedDrawInfo.TaskAltitudeArrival) <ALTDIFFLIMIT) //@ 091114
                        _stprintf(Value,TEXT(" --- "));
                    else
                        _stprintf(Value,TEXT("%1.0f "), ALTITUDEMODIFY*DerivedDrawInfo.TaskAltitudeArrival);
                } else {
                    if ( (ALTITUDEMODIFY*DerivedDrawInfo.TaskAltitudeDifference) <ALTDIFFLIMIT) //@ 091114
                        _stprintf(Value,TEXT(" --- "));
                    else
                        _stprintf(Value,TEXT("%1.0f "), ALTITUDEMODIFY*DerivedDrawInfo.TaskAltitudeDifference);
                }
			} else {
				if ( (ALTITUDEMODIFY*WayPointCalc[barindex].AltArriv[AltArrivMode]) < ALTDIFFLIMIT)
					_stprintf(Value,TEXT(" --- "));
				else
					_stprintf(Value,TEXT("%1.0f "), ALTITUDEMODIFY*WayPointCalc[barindex].AltArriv[AltArrivMode]);
				/*
				 * Well this was the reason why the glidebar value was out of sync with overlays
				if ( (ALTITUDEMODIFY*DerivedDrawInfo.NextAltitudeDifference) < ALTDIFFLIMIT) //@ 091114
					_stprintf(Value,TEXT(" --- "));
				else
					_stprintf(Value,TEXT("%1.0f "), ALTITUDEMODIFY*DerivedDrawInfo.NextAltitudeDifference);
				*/
			}
		} else {
			if ( (ALTITUDEMODIFY*WayPointCalc[barindex].AltArriv[AltArrivMode]) < ALTDIFFLIMIT)
				_stprintf(Value,TEXT(" --- "));
			else
				_stprintf(Value,TEXT("%1.0f "), ALTITUDEMODIFY*WayPointCalc[barindex].AltArriv[AltArrivMode]);
		}

_skipout: 
		// in case of invalidbar we get here with Offset 0

		if (Offset>=0) {
			Offset = GlideBar[2].y+Offset+NIBLSCALE(5);
		} else {
			if (Offset0>0) {
				Offset = GlideBar0[1].y-NIBLSCALE(15);
			} else {
				Offset = GlideBar[2].y+Offset-NIBLSCALE(15);
			}
		}
		// VENTA10
		Surface.GetTextSize(Value, _tcslen(Value), &TextSize); 
		GlideBarOffset=max(NIBLSCALE(11),(int)TextSize.cx) - NIBLSCALE(2);
 
    TextInBoxMode_t TextInBoxMode = {0};
    TextInBoxMode.Border = true;          //={1|8};
    TextInBoxMode.Reachable = true;
    // boxed numbers are a bit too much on the left, so increase the offset
    TextInBox(Surface, &rc,Value, lkVarioOffset+NIBLSCALE(1), (int)Offset, 0, &TextInBoxMode); //@ 091114

	Surface.SelectObject(hbOld);
	Surface.SelectObject(hpOld);
    } else GlideBarOffset=0; 	// 091125 BUGFIX glidebaroffset is zero when no task point
     {
       UnlockTaskData();
     }
  
}


