/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

bool MapWindow::GliderCenter=false;



void MapWindow::CalculateOrientationNormal(void) {
  double trackbearing = DrawInfo.TrackBearing;

  if( ( MapWindow::mode.autoNorthUP() ||  DisplayOrientation == NORTHUP) ||
      ((DisplayOrientation == NORTHTRACK) &&(!mode.Is(Mode::MODE_CIRCLING)))
	|| (DisplayOrientation == NORTHSMART) ||
	( ((DisplayOrientation == NORTHCIRCLE) ||(DisplayOrientation==TARGETCIRCLE)) && (mode.Is(Mode::MODE_CIRCLING)) ) )
  {
		if (mode.Is(Mode::MODE_CIRCLING))
			GliderCenter=true;
		else
			GliderCenter=false;

		if (DisplayOrientation == TARGETCIRCLE) {
			DisplayAngle = DerivedDrawInfo.WaypointBearing;
			DisplayAircraftAngle = trackbearing-DisplayAngle;
		} else {
			DisplayAngle = 0.0;
			DisplayAircraftAngle = trackbearing;
		}

  } else {

	// normal, glider forward
	GliderCenter = false;
	DisplayAngle = trackbearing;
	DisplayAircraftAngle = 0.0;
  }

	if ( ! MapWindow::mode.autoNorthUP() && DisplayOrientation == TARGETUP) {
		DisplayAngle = DerivedDrawInfo.WaypointBearing;
		DisplayAircraftAngle = trackbearing-DisplayAngle;
	}
  DisplayAngle = AngleLimit360(DisplayAngle);
  DisplayAircraftAngle = AngleLimit360(DisplayAircraftAngle);

}



void MapWindow::CalculateOrientationTargetPan(void) {
  // Target pan mode, show target up when looking at current task point,
  // otherwise north up.
  GliderCenter = true;
  if ((ActiveTaskPoint==TargetPanIndex)
      &&(DisplayOrientation != NORTHUP)
      &&(DisplayOrientation != NORTHSMART) // 100419
      &&(DisplayOrientation != NORTHTRACK)
      &&(!MapWindow::mode.autoNorthUP())) {
    // target-up
    DisplayAngle = DerivedDrawInfo.WaypointBearing;
    DisplayAircraftAngle = DrawInfo.TrackBearing-DisplayAngle;
  }
  else {
    // North up
    DisplayAngle = 0.0;
    DisplayAircraftAngle = DrawInfo.TrackBearing;
  }
}



void MapWindow::CalculateOrigin(const RECT& rc, POINT *Orig) {

  if (mode.Is(Mode::MODE_PAN)) {
	// North up
	DisplayAngle = 0.0;
	DisplayAircraftAngle = DrawInfo.TrackBearing;
	GliderCenter = true;
  } else {
	if (mode.Is(Mode::MODE_TARGET_PAN)) {
		CalculateOrientationTargetPan();
	} else {
		CalculateOrientationNormal();
	}
  }

  if(mode.Is(Mode::MODE_TARGET_PAN)) {
    if (ScreenLandscape) {
      Orig->x = (rc.left + rc.right - targetPanSize)/2;
      Orig->y = (rc.bottom + rc.top)/2;
    }
    else {
      Orig->x = (rc.left + rc.right)/2;
      Orig->y = (rc.bottom + rc.top + targetPanSize)/2;
    }
  }
  else if(mode.Is(Mode::MODE_PAN) || mode.Is(Mode::MODE_CIRCLING)) {
	Orig->x = (rc.left + rc.right)/2;
	Orig->y = (rc.bottom + rc.top)/2;
  } else {
	// automagic northup smart
	if (DisplayOrientation == NORTHSMART) {
		double trackbearing = DrawInfo.TrackBearing;
		int middleXY,spanxy;
		if (ScreenLandscape) {
			middleXY=((rc.bottom-BottomSize)+rc.top)/2;
			spanxy=NIBLSCALE(50);
			Orig->y= middleXY + (int)(spanxy*fastcosine(trackbearing));
			// This was moving too much the map!
			// spanx=NIBLSCALE(40);
			// Orig->x= middleX - (int)(spanx*fastsine(trackbearing));
			Orig->x = (rc.left + rc.right)/2;
		} else {
			middleXY=(rc.left+rc.right)/2;
			spanxy=NIBLSCALE(50);
			Orig->x= middleXY - (int)(spanxy*fastsine(trackbearing));
			Orig->y = ((rc.bottom-BottomSize) + rc.top)/2;
		}
	} else {
		// 100924 if we are in north up autorient, position the glider in middle screen
		if (zoom.Scale() > AutoOrientScale) {
			Orig->x = (rc.left + rc.right)/2;
			Orig->y=((rc.bottom-BottomSize)+rc.top)/2;
		} else {
			// else do it normally using configuration
			Orig->x = ((rc.right - rc.left )*GliderScreenPositionX/100)+rc.left;
			Orig->y = ((rc.top - rc.bottom )*GliderScreenPositionY/100)+rc.bottom;
		}
	}
  }
}



// change dynamically the map orientation mode
// set true flag for resetting DisplayOrientation mode and return
void MapWindow::SetAutoOrientation() {

  // 1.4 because of correction if mapscale reported on screen in MapWindow2
  if (MapWindow::zoom.Scale() > AutoOrientScale) {
    MapWindow::mode.setAutoNorthUP(true);
  } else {
	  MapWindow::mode.setAutoNorthUP(false);
  }
}
