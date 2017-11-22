/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKProcess.h"
#include "Waypointparser.h"
#include "Dialogs.h"
#include "Topology.h"
#include "Terrain.h"
#include "Draw/ScreenProjection.h"
#include "LKStyle.h"
#include "NavFunctions.h"
#include "utils/printf.h"
#include "Library/TimeFunctions.h"

const TCHAR *DegreesToText(double brg) {
  if (brg<23||brg>=338) { return MsgToken<1703>(); } // North
  if (brg<68)  { return MsgToken<1704>(); }	// North-East
  if (brg<113) { return MsgToken<1705>(); }	// East
  if (brg<158) { return MsgToken<1706>(); }	// South-East
  if (brg<203) { return MsgToken<1707>(); }	// South
  if (brg<248) { return MsgToken<1708>(); }	// South-West
  if (brg<293) { return MsgToken<1709>(); }	// West
  if (brg<338) { return MsgToken<1710>(); }	// North-West

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

const TCHAR *WhatTimeIsIt(void) {
  static thread_local TCHAR time_temp[48];
  TCHAR tlocal[20];
  TCHAR tutc[20];

  Units::TimeToTextS(tlocal, LocalTime()),
  Units::TimeToText(tutc, GPS_INFO.Time);
  _stprintf(time_temp, _T("h%s (UTC %s)"), tlocal, tutc);
  if (GPS_INFO.NAVWarning || (GPS_INFO.SatellitesUsed == 0))
     _stprintf(time_temp, _T("h%s (NO FIX)"), tlocal);
  else
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
						Units::ToUserDistance(dist),
						Units::GetDistanceName(),
						DegreesToText(brg),
						MsgToken<1711>(),ntype,name); // of
	return ttmp;
  }

  // near
  if (dist>dist_over) {
	_stprintf(ttmp,_T("%s %s <%s>"),MsgToken<1712>(), ntype,name); // Near
	return ttmp;
  }

  _stprintf(ttmp,_T("%s %s <%s>"),MsgToken<1713>(), ntype,name); // Over
  return ttmp;

}


extern Topology* TopoStore[MAXTOPOLOGY];

extern void ResetNearestTopology();

void WhereAmI::Run() {
  TestLog(_T("Oracle : start to find position"));

  PeriodClock _time;
  _time.Update();

  ResetNearestTopology();

  LockTerrainDataGraphics();

  const vectorObj center = {
      MapWindow::GetPanLongitude(),
      MapWindow::GetPanLatitude()
  };
  rectObj bounds = {
      center.x, center.y,
      center.x, center.y
  };

  for(int i = 0; i < 10; ++i) {
      double X, Y;
      FindLatitudeLongitude(center.y, center.x, i*360/10, 30*1000, &Y, &X );
      bounds.minx = std::min(bounds.minx, X);
      bounds.maxx = std::max(bounds.maxx, X);
      bounds.miny = std::min(bounds.miny, Y);
      bounds.maxy = std::max(bounds.maxy, Y);
  }


  for (int z=0; z<MAXTOPOLOGY; z++) {
    if (TopoStore[z]) {
      // See also CheckScale for category checks! We should use a function in fact.
      if ( TopoStore[z]->scaleCategory == 10 ||
        (TopoStore[z]->scaleCategory >= 70 && TopoStore[z]->scaleCategory <=110)) {
        TopoStore[z]->SearchNearest(bounds);
      }
    }
  }

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


  _stprintf(toracle,_T("%s\n\n"), MsgToken<1724>()); // YOUR POSITION:

  if (NearestBigCity.Valid) {
	_tcscat(toracle, OracleFormatDistance(NearestBigCity.Name,MsgToken<1714>(),
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
		_stprintf(ttmp,_T("%.0f %s %s %s "),
		    Units::ToUserDistance(dist), Units::GetDistanceName(), DegreesToText(brg),
			MsgToken<1715>());	// of the city
		_tcscat(toracle,ttmp);

	  } else {
		//
		//  Over city
		//
		_stprintf(ttmp,_T("%s "),MsgToken<1716>()); // Over the city
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
				_stprintf(ttmp,_T(" %s %s"), MsgToken<1717>(),NearestWaterArea.Name); // and
				_tcscat(toracle,ttmp);
				saynear=true;
			} else {
				//
				// 2km South of city
				// over lake
				//
				_stprintf(ttmp,_T("\n%s %s"), MsgToken<1718>(),NearestWaterArea.Name); // over
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
					_stprintf(ttmp,_T("\n%s %s"), MsgToken<1719>(),NearestWaterArea.Name); // near to
					_tcscat(toracle,ttmp);
				} else {
					//
					// 2km South of city
					// near lake
					//
					_stprintf(ttmp,_T("\n%s %s"), MsgToken<1718>(),NearestWaterArea.Name); // over
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
			_stprintf(ttmp,_T("%.0f %s %s "), Units::ToUserDistance(NearestWaterArea.Distance), Units::GetDistanceName(), DegreesToText(brg));
			_tcscat(toracle,ttmp);
			_stprintf(ttmp,_T("%s <%s>"), MsgToken<1711>(),NearestWaterArea.Name); // of
			_tcscat(toracle,ttmp);
		} else {
			//
			// Over lake
			//
			_stprintf(ttmp,_T("%s <%s>"), MsgToken<1718>(),NearestWaterArea.Name); // over
			_tcscat(toracle,ttmp);
			over=true;
		}
		found=true;
	}
  }

  _tcscat(toracle,_T("\n"));


  // find nearest turnpoint & nearest Landable
  unsigned idx_nearest_airport = 0;
  unsigned idx_nearest_unknown = 0;
  {
    double dist_airport = std::numeric_limits<double>::max();
    double dist_unknown = std::numeric_limits<double>::max();

    for(unsigned i=NUMRESWP; i<WayPointList.size(); ++i) {

        if(WayPointList[i].Style == STYLE_THERMAL) continue;

        DistanceBearing(GPS_INFO.Latitude, GPS_INFO.Longitude,
                      WayPointList[i].Latitude, WayPointList[i].Longitude,
                      &(WayPointCalc[i].Distance), &(WayPointCalc[i].Bearing));

        if(WayPointCalc[i].Distance > 70000) continue; // To Far

        if(WayPointCalc[i].WpType == WPT_AIRPORT) {
            if(WayPointCalc[i].Distance < dist_airport) {
                dist_airport = WayPointCalc[i].Distance;
                idx_nearest_airport = i;
            }
        }
        if(WayPointCalc[i].Distance < dist_unknown) {
            dist_unknown = WayPointCalc[i].Distance;
            idx_nearest_unknown = i;
        }
    }
  }

  int j = idx_nearest_unknown;
  if (!ValidNotResWayPoint(j)) goto _end;

  found=true;

_dowp:

  DistanceBearing(
	WayPointList[j].Latitude,WayPointList[j].Longitude,
	GPS_INFO.Latitude, GPS_INFO.Longitude,
	&wpdist,&brg);

  TCHAR wptype[100];
  switch(WayPointList[j].Style) {
	case STYLE_AIRFIELDGRASS:
	case STYLE_GLIDERSITE:
		_stprintf(wptype,_T("%s "), MsgToken<1720>());	// the airfield of
		break;
	case STYLE_OUTLANDING:
		_stprintf(wptype,_T("%s "), MsgToken<1721>());	// the field of
		needmorewp=true;
		break;
	case STYLE_AIRFIELDSOLID:
		_stprintf(wptype,_T("%s "), MsgToken<1722>());	// the airport of
		break;
	default:
		_tcscpy(wptype,_T(""));
		needmorewp=true;
		break;
  }

  if ( (_tcslen(wptype)==0) && WayPointCalc[j].IsLandable) {
	if (WayPointCalc[j].IsAirport)  {
		 _stprintf(wptype,_T("%s "), MsgToken<1720>());	// the airfield of
		needmorewp=false;
	} else {
		 _stprintf(wptype,_T("%s "), MsgToken<1721>());	// the field of
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
	_stprintf(ttmp,_T("\n%.0f %s %s "), Units::ToUserDistance(wpdist), Units::GetDistanceName(), DegreesToText(brg));
	_tcscat(toracle,ttmp);

	 lk::snprintf(ttmp,_T("%s %s<%s>"), MsgToken<1711>(),wptype,WayPointList[j].Name); // of
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

				lk::snprintf(ttmp,_T("\n%s %s<%s>"), MsgToken<1719>(),wptype,WayPointList[j].Name); // near to
				_tcscat(toracle,ttmp);
			} else {
				// Over city
				// near lake and waypoint

				lk::snprintf(ttmp,_T(" %s %s<%s>"), MsgToken<1717>(),wptype,WayPointList[j].Name); // and
				_tcscat(toracle,ttmp);
			}
		} else {
			lk::snprintf(ttmp,_T("\n%s %s<%s>"), MsgToken<1719>(),wptype,WayPointList[j].Name); // near to
			_tcscat(toracle,ttmp);
		}
	} else {
		//
		// Near waypoint (because "over" could be wrong, we have altitudes in wp!)
		//
		lk::snprintf(ttmp,_T("%s %s<%s>"), MsgToken<1723>(),wptype,WayPointList[j].Name); // Near to
		_tcscat(toracle,ttmp);
	}
  }

  if (!needmorewp) goto _end;
  if (secondwpdone) goto _end;

  j=idx_nearest_airport;
  if (!ValidNotResWayPoint(j)) goto _end;
  secondwpdone=true;
  goto _dowp;

_end:

  UnlockTerrainDataGraphics();

  // VERY SORRY - YOUR POSITION IS UNKNOWN!
  if (!found) _stprintf(toracle,_T("\n\n%s\n\n%s"), MsgToken<1725>(),MsgToken<1726>());

  CharUpper(toracle);

  #ifdef TESTBENCH
  StartupStore(_T("Oracle : Result found in %d ms") NEWLINE, _time.Elapsed());
  #endif
}
