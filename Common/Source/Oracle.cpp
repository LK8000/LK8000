/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKProcess.h"
#include "Waypointparser.h"
#include "dlgTools.h"


const TCHAR *DegreesToText(double brg) {
  if (brg<23||brg>=338) { return gettext(_T("_@M1703_")); } // North
  if (brg<68)  { return gettext(_T("_@M1704_")); }	// North-East
  if (brg<113) { return gettext(_T("_@M1705_")); }	// East
  if (brg<158) { return gettext(_T("_@M1706_")); }	// South-East
  if (brg<203) { return gettext(_T("_@M1707_")); }	// South
  if (brg<248) { return gettext(_T("_@M1708_")); }	// South-West
  if (brg<293) { return gettext(_T("_@M1709_")); }	// West
  if (brg<338) { return gettext(_T("_@M1710_")); }	// North-West

  return(_T("??"));

}

#if 0 // this is confusing , no translation, unused
TCHAR *AltDiffToText(double youralt, double wpalt) {
  static TCHAR sAdiff[20];

  int altdiff=(int) (youralt - wpalt);
  if (altdiff >=0)
	_tcscpy(sAdiff,_T("over"));
  else
	_tcscpy(sAdiff,_T("below"));

 return (sAdiff);

}
#endif

TCHAR *WhatTimeIsIt(void) {
  static TCHAR time_temp[60];
  TCHAR tlocal[20];
  TCHAR tutc[20];

  Units::TimeToTextS(tlocal, (int)TimeLocal((int)(GPS_INFO.Time))),
  Units::TimeToText(tutc, (int)GPS_INFO.Time);
  _stprintf(time_temp, _T("h%s (UTC %s)"), tlocal, tutc);

  return (time_temp);
}

//
// Modes
// 0 big city
//
TCHAR *OracleFormatDistance(const TCHAR *name,const TCHAR *ntype,const double dist,const double brg,const short mode) {

  static TCHAR ttmp[150];
  double dist_over=0, dist_near=0;
  
  switch(mode) {
	case 0:		// big city
	default:
          dist_over=1500;
          dist_near=3000;
          break;
  }

  // 5km west of  the city <abcd>
  if (dist>dist_near) {
	_stprintf(ttmp,_T("%.0f %s %s %s %s <%s>"), 
		dist*DISTANCEMODIFY, Units::GetDistanceName(), DegreesToText(brg), gettext(_T("_@M1711_")),ntype,name); // of
	return ttmp;
  }

  // near 
  if (dist>dist_over) {
	_stprintf(ttmp,_T("%s %s <%s>"),gettext(_T("_@M1712_")), ntype,name); // Near
	return ttmp;
  }

  _stprintf(ttmp,_T("%s %s <%s>"),gettext(_T("_@M1713_")), ntype,name); // Over
  return ttmp;

}




// This is called by the Draw thread 
// This is still a draft, to find out what's best to tell and how.
// It should be recoded once the rules are clear.
void WhereAmI(void) {

  TCHAR toracle[500];
  TCHAR ttmp[100];
  double dist,wpdist,brg;
  NearestTopoItem *item=NULL;
  bool found=false, over=false, saynear=false, needmorewp=false, secondwpdone=false;

  #if TESTBENCH
  if (NearestBigCity.Valid)
	StartupStore(_T("... NEAREST BIG CITY <%s>  at %.0f km brg=%.0f\n"),
		NearestBigCity.Name,NearestBigCity.Distance/1000,NearestBigCity.Bearing);
  if (NearestCity.Valid)
	StartupStore(_T("... NEAREST CITY <%s>  at %.0f km brg=%.0f\n"),
		NearestCity.Name,NearestCity.Distance/1000,NearestCity.Bearing);
  if (NearestSmallCity.Valid)
	StartupStore(_T("... NEAREST TOWN <%s>  at %.0f km brg=%.0f\n"),
		NearestSmallCity.Name,NearestSmallCity.Distance/1000,NearestSmallCity.Bearing);
  if (NearestWaterArea.Valid)
	StartupStore(_T("... NEAREST WATER AREA <%s>  at %.0f km brg=%.0f\n"),
		NearestWaterArea.Name,NearestWaterArea.Distance/1000,NearestWaterArea.Bearing);
  #endif


  _stprintf(toracle,_T("%s\n\n"), gettext(_T("_@M1724_"))); // YOUR POSITION:

  if (NearestBigCity.Valid) {
	_tcscat(toracle, OracleFormatDistance(NearestBigCity.Name,gettext(_T("_@M1714_")), 
		NearestBigCity.Distance, NearestBigCity.Bearing,0));	// the city
	_tcscat(toracle, _T("\n"));
  }


  if (NearestCity.Valid && NearestSmallCity.Valid) {
	
	if ( (NearestCity.Distance - NearestSmallCity.Distance) <=3000) 
		item=&NearestCity;
	else
		item=&NearestSmallCity;
  } else {
	if (NearestCity.Valid)
		item=&NearestCity;
	else
		if (NearestSmallCity.Valid)
			item=&NearestSmallCity;
  }

  if (item) {
	 dist=item->Distance;
	 brg=item->Bearing;

 	 if (dist>1500) {
		//
		// 2km South of city
		//
		_stprintf(ttmp,_T("%.0f %s %s %s "), dist*DISTANCEMODIFY, Units::GetDistanceName(), DegreesToText(brg),
			gettext(_T("_@M1715_")));	// of the city
		_tcscat(toracle,ttmp);

	  } else {
		//
		//  Over city
		//
  		_stprintf(ttmp,_T("%s "),gettext(_T("_@M1716_"))); // Over the city
 		 _tcscat(toracle,ttmp);
		over=true;
	  }
	  _stprintf(ttmp,_T("<%s>"), item->Name);
	  _tcscat(toracle,ttmp);

	  found=true;
  }

  // Careful, some wide water areas have the center far away from us even if we are over them.
  // We can only check for 2-5km distances max.
  if (NearestWaterArea.Valid) {
  	if (found) {
		if (NearestWaterArea.Distance<2000) {
			if (over) {
				//
				// Over city and lake
				//
	 			_stprintf(ttmp,_T(" %s %s"), gettext(_T("_@M1717_")),NearestWaterArea.Name); // and
	 			_tcscat(toracle,ttmp);
				saynear=true;
			} else {
				//
				// 2km South of city 
				// over lake
				//
	 			_stprintf(ttmp,_T("\n%s %s"), gettext(_T("_@M1718_")),NearestWaterArea.Name); // over
	 			_tcscat(toracle,ttmp);
				saynear=true;
			}
		} else {
			if (NearestWaterArea.Distance<6000) {
				if (over) {
					//
					// Over city 
					// near lake
					//
	 				_stprintf(ttmp,_T("\n%s %s"), gettext(_T("_@M1719_")),NearestWaterArea.Name); // near to
	 				_tcscat(toracle,ttmp);
				} else {
					//
					// 2km South of city
					// near lake
					//
	 				_stprintf(ttmp,_T("\n%s %s"), gettext(_T("_@M1718_")),NearestWaterArea.Name); // over
	 				_tcscat(toracle,ttmp);
				}
			}
			// else no mention to water area, even if it is the only item. Not accurate!
		}
	} else {
		if (NearestWaterArea.Distance>2000) {
			brg=NearestWaterArea.Bearing;
			//
			// 2km North of lake
			// 
			_stprintf(ttmp,_T("%.0f %s %s "), NearestWaterArea.Distance*DISTANCEMODIFY, Units::GetDistanceName(), DegreesToText(brg));
			_tcscat(toracle,ttmp);
		 	_stprintf(ttmp,_T("%s <%s>"), gettext(_T("_@M1711_")),NearestWaterArea.Name); // of
 			_tcscat(toracle,ttmp);
		} else {
			//
			// Over lake
			// 
 			_stprintf(ttmp,_T("%s <%s>"), gettext(_T("_@M1718_")),NearestWaterArea.Name); // over
 			_tcscat(toracle,ttmp);
			over=true;
		}
		found=true;
	}
  }

  _tcscat(toracle,_T("\n"));

  int j=FindNearestFarVisibleWayPoint(GPS_INFO.Longitude,GPS_INFO.Latitude,70000,WPT_UNKNOWN);
  if (!ValidNotResWayPoint(j)) goto _end;

  found=true;

_dowp:

  DistanceBearing( 
	WayPointList[j].Latitude,WayPointList[j].Longitude, 
	GPS_INFO.Latitude, GPS_INFO.Longitude, 
	&wpdist,&brg);

  TCHAR wptype[100];
  switch(WayPointList[j].Style) {
	case 2:
	case 4:
 		_stprintf(wptype,_T("%s "), gettext(_T("_@M1720_")));	// the airfield of
		break;
	case 3:
 		_stprintf(wptype,_T("%s "), gettext(_T("_@M1721_")));	// the field of
		needmorewp=true;
		break;
	case 5:
 		_stprintf(wptype,_T("%s "), gettext(_T("_@M1722_")));	// the airport of
		break;
	default:
		_tcscpy(wptype,_T(""));
		needmorewp=true;
		break;
  }

  if ( (_tcslen(wptype)==0) && WayPointCalc[j].IsLandable) {
	if (WayPointCalc[j].IsAirport)  {
 		 _stprintf(wptype,_T("%s "), gettext(_T("_@M1720_")));	// the airfield of
		needmorewp=false;
	} else {
 		 _stprintf(wptype,_T("%s "), gettext(_T("_@M1721_")));	// the field of
		needmorewp=true;
	}
  } else {
	if (_tcslen(wptype)==0 ) {
		_tcscpy(wptype,_T(""));
		needmorewp=true;
	}
  }

  // nn km south
  if (wpdist>2000) {
	//
	// 2km South of city 
	// and/over lake
	// 4 km SW of waypoint
	_stprintf(ttmp,_T("\n%.0f %s %s "), wpdist*DISTANCEMODIFY, Units::GetDistanceName(), DegreesToText(brg));
	_tcscat(toracle,ttmp);

 	 _stprintf(ttmp,_T("%s %s<%s>"), gettext(_T("_@M1711_")),wptype,WayPointList[j].Name); // of
 	 _tcscat(toracle,ttmp);

  } else {
	if (found) {
		if (over) {
			if (saynear) {
				//
				// 2km South of city 
				// over lake
				// near waypoint
				// ----
				// Over city and lake
				// near waypoint

	 			_stprintf(ttmp,_T("\n%s %s<%s>"), gettext(_T("_@M1719_")),wptype,WayPointList[j].Name); // near to
	 			_tcscat(toracle,ttmp);
			} else {
				// Over city 
				// near lake and waypoint

	 			_stprintf(ttmp,_T(" %s %s<%s>"), gettext(_T("_@M1717_")),wptype,WayPointList[j].Name); // and
	 			_tcscat(toracle,ttmp);
			}
		} else {
 			_stprintf(ttmp,_T("\n%s %s<%s>"), gettext(_T("_@M1719_")),wptype,WayPointList[j].Name); // near to
 			_tcscat(toracle,ttmp);
		}
	} else {
		//
		// Near waypoint (because "over" could be wrong, we have altitudes in wp!)
		// 
 		_stprintf(ttmp,_T("%s %s<%s>"), gettext(_T("_@M1723_")),wptype,WayPointList[j].Name); // Near to
 		_tcscat(toracle,ttmp);
	}
  }

  if (!needmorewp) goto _end;
  if (secondwpdone) goto _end;

  j=FindNearestFarVisibleWayPoint(GPS_INFO.Longitude,GPS_INFO.Latitude,70000,WPT_AIRPORT);
  if (!ValidNotResWayPoint(j)) goto _end;
  secondwpdone=true;
  goto _dowp;


_end:
#ifdef ULLIS_PRIVATE_FEATURES
	_stprintf(ttmp,_T("\n\nin %i ft (MSL)"),  (int)( GPS_INFO.Altitude*TOFEET)); // in height
	_tcscat(toracle,ttmp);
#endif
  // VERY SORRY - YOUR POSITION IS UNKNOWN!
  if (!found) _stprintf(toracle,_T("\n\n%s\n\n%s"), gettext(_T("_@M1725_")),gettext(_T("_@M1726_")));

  CharUpper(toracle);
  MessageBoxX(toracle, gettext(_T("_@M1690_")), mbOk, true);

}

