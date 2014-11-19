/*
 * THIS CODE IS INCLUDED IN LKDRAWTRAIL if LONGTRAIL option is selected
 * This is temporary, until the option is permanent and the file is included
 * in the Makefile. 
 */


double MapWindow::LKDrawLongTrail( LKSurface& Surface, const POINT& Orig, const RECT& rc)
{
  if(TrailActive!=3) return -1; // only when full trail is selected
  if (iLongSnailNext<2) return -1; // no reason to draw a single point

  // Do not draw long trail in thermal view mode, because the snail trail is shrinked and drifted!
  if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) return -1;

  int i;
  LONG_SNAIL_POINT P1;
  int  nearby;


  // Keep track of what's drawn

  bool this_visible = true;
  bool last_visible = false;
  POINT point_lastdrawn;
  point_lastdrawn.x = 0;
  point_lastdrawn.y = 0;

  const int deg = DEG_TO_INT(AngleLimit360(DisplayAngle));
  const int cost = ICOSTABLE[deg];
  const int sint = ISINETABLE[deg];
  const int xxs = Orig_Screen.x*1024-512;
  const int yys = Orig_Screen.y*1024+512;
  const double mDrawScale = zoom.DrawScale();
  const double mPanLongitude = PanLongitude;
  const double mPanLatitude = PanLatitude;

  // pixel manhattan distance
  // It is the sum of x and y differences between previous and next point on screen, in pixels.
  // below this distance, no painting
  nearby=14;

    const auto oldPen = Surface.SelectObject(hSnailPens[3]); // blue color

    // draw from oldest to newest point
    // the "+1" is the latest point in the snail trail, to connect the two trails
    for(i=0; i<iLongSnailNext+1; i++) {
        P1 = LongSnailTrail[i];

        if (P1.Longitude==0) {
	    continue;
        }

        if (!P1.FarVisible) {
            last_visible = false;
            continue;
        }

        this_visible =   ((P1.Longitude> screenbounds_latlon.minx) &&
		     (P1.Longitude< screenbounds_latlon.maxx) &&
		     (P1.Latitude> screenbounds_latlon.miny) &&
		     (P1.Latitude< screenbounds_latlon.maxy)) ;

        if (!this_visible && !last_visible) {
            last_visible = false;
            continue;
        }

	int Y = Real2Int((mPanLatitude-P1.Latitude)*mDrawScale);
	int X = Real2Int((mPanLongitude-P1.Longitude)*fastcosine(P1.Latitude)*mDrawScale);
	P1.Screen.x = (xxs-X*cost + Y*sint)/1024;
	P1.Screen.y = (Y*cost + X*sint + yys)/1024;


        // skip closer points usiong manhattan distance
        if (last_visible && this_visible ) {
	    if ( (abs(P1.Screen.y-point_lastdrawn.y) + abs(P1.Screen.x-point_lastdrawn.x))<=nearby ) {
		continue;
	    }
        }

        if (last_visible) { // draw set cursor at P1
            Surface.DrawSolidLine(P1.Screen, point_lastdrawn, rc);
        }
        point_lastdrawn = P1.Screen;
        last_visible = this_visible;

    } // big for loop
    Surface.SelectObject(oldPen);

#if 0
  // TODO we may draw to oldest point in the snailtrail, instead.. if really we want to do it
  // Otherwise we have 1 minute gap between snail and long trail
  if (last_visible) {
    Surface.DrawSolidLine(Orig, point_lastdrawn, rc);
  }
#endif

  return 1;
}

