/*
 * $Id: LKMapWindow.h,v 1.1 2011/12/21 10:35:29 root Exp root $
 */

#if !defined(LKMAPWINDOW_H)
#define LKMAPWINDOW_H

#include "ScreenGeometry.h"

// Maximum number of characters for waypoint name. Should be dynamically calculated?
// CAUTION a line should be adjusted by hand! Not a reference value!!
#define MAXNLNAME 12


#define PANELCOLUMNS    4
#define PANELROWS       4
#define RIGHTLIMITER    NIBLSCALE(3)
#define LEFTLIMITER     NIBLSCALE(3)
#define TOPLIMITER      NIBLSCALE(1)
#define BOTTOMLIMITER   NIBLSCALE(2)


#endif
