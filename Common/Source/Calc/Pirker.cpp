/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"


//
// Herbert Pirker calculation
//
double PirkerAnalysis(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
                      const double this_bearing,
                      const double GlideSlope) {


//  bool maxfound = false;
//  bool first = true;
  double pirker_mc = 0.0;
  double h_target = GlideSlope;
  double h;
  double dh= 1.0;
  double last_pirker_mc = 5.0;
  double last_dh = -1.0;
  double pirker_mc_zero = 0.0;
 
  (void)Basic;

  while (pirker_mc<10.0) {

    h = GlidePolar::MacCreadyAltitude(pirker_mc, 
                                      1.0, // unit distance
				      this_bearing, 
                                      Calculated->WindSpeed, 
                                      Calculated->WindBearing, 
                                      0, 0, true, 0);

    dh = (h_target-h); 
    // height difference, how much we have compared to 
    // how much we need at that speed.
    //   dh>0, we can afford to speed up

    if (dh==last_dh) {
      // same height, must have hit max speed.
      if (dh>0) {
        return last_pirker_mc;
      } else {
        return 0.0;
      }
    }

    if ((dh<=0)&&(last_dh>0)) {
      if (dh-last_dh < 0) {
	double f = (-last_dh)/(dh-last_dh);
	pirker_mc_zero = last_pirker_mc*(1.0-f)+f*pirker_mc;	
      } else {
	pirker_mc_zero = pirker_mc;
      }
      return pirker_mc_zero;
    }
    last_dh = dh;
    last_pirker_mc = pirker_mc;

    pirker_mc += 0.5;
  }
  if (dh>=0) {
    return pirker_mc;
  }
  return -1.0; // no solution found, unreachable without further climb
}


