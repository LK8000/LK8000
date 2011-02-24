/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

*/

#include "StdAfx.h"
#include "LKAirspace.h"
#include "externs.h"
#include "RasterTerrain.h"
// 
#include <tchar.h>
#include <ctype.h>

#include "wcecompat/ts_string.h"

#ifdef LKAIRSPACE

static const int k_nAreaCount = 13;
static const TCHAR* k_strAreaStart[k_nAreaCount] = {
					_T("R"),  
					_T("Q"), 
					_T("P"), 
					_T("CTR"),
					_T("A"), 
					_T("B"), 
					_T("C"), 
					_T("D"), 
					_T("GP"), 
					_T("W"), 
					_T("E"), 
					_T("F"),
					_T("G")
};
static const int k_nAreaType[k_nAreaCount] = {
					RESTRICT, 
					DANGER, 
					PROHIBITED, 
					CTR,
					CLASSA, 
					CLASSB, 
					CLASSC, 
					CLASSD, 
					NOGLIDER, 
					WAVE, 
					CLASSE, 
					CLASSF,
					CLASSG};


//for Draw()
extern void ClipPolygon(HDC hdc, POINT *ptin, unsigned int n, 
                 RECT rc, bool fill=true);

CAirspaceManager CAirspaceManager::_instance = CAirspaceManager(CAirspaceManager::_instance);


//
// CAIRSPACE CLASEE
//

void CAirspace::Dump() const
{
  //StartupStore(TEXT("CAirspace Dump%s"),NEWLINE);
  StartupStore(TEXT(" Name:%s%s"),_name,NEWLINE);
  StartupStore(TEXT(" Type:%d (%s)%s"),_type,k_strAreaStart[_type], NEWLINE);
  StartupStore(TEXT(" Base.Altitude:%lf%s"),_base.Altitude,NEWLINE);
  StartupStore(TEXT(" Base.FL:%lf%s"),_base.FL,NEWLINE);
  StartupStore(TEXT(" Base.AGL:%lf%s"),_base.AGL,NEWLINE);
  StartupStore(TEXT(" Base.Base:%d%s"),_base.Base,NEWLINE);
  StartupStore(TEXT(" Top.Altitude:%lf%s"),_top.Altitude,NEWLINE);
  StartupStore(TEXT(" Top.FL:%lf%s"),_top.FL,NEWLINE);
  StartupStore(TEXT(" Top.AGL:%lf%s"),_top.AGL,NEWLINE);
  StartupStore(TEXT(" Top.Base:%d%s"),_top.Base,NEWLINE);
  StartupStore(TEXT(" bounds.minx,miny:%lf,%lf%s"),_bounds.minx,_bounds.miny,NEWLINE);
  StartupStore(TEXT(" bounds.maxx,maxy:%lf,%lf%s"),_bounds.maxx,_bounds.maxy,NEWLINE);
   
}

void CAirspace::AirspaceAGLLookup(double av_lat, double av_lon) 
{
  if (((_base.Base == abAGL) || (_top.Base == abAGL))) {
    RasterTerrain::Lock();
    // want most accurate rounding here
    RasterTerrain::SetTerrainRounding(0,0);
    double th = RasterTerrain::GetTerrainHeight(av_lat, av_lon);

	if (th==TERRAIN_INVALID) th=0; //@ 101027 FIX
    
    if (_base.Base == abAGL) {
      if (_base.AGL>=0) {
		_base.Altitude = _base.AGL+th;
      } else {
		// surface, set to zero
		_base.AGL = 0;
		_base.Altitude = 0;
      }
    }
    if (_top.Base == abAGL) {
      if (_top.AGL>=0) {
		_top.Altitude = _top.AGL+th;
      } else {
		// surface, set to zero
		_top.AGL = 0;
		_top.Altitude = 0;
      }
    }
    // 101027 We still use 0 altitude for no terrain, what else can we do..
    RasterTerrain::Unlock();
  }
}

void CAirspace::QnhChangeNotify()
{
  if (_top.Base == abFL) _top.Altitude = AltitudeToQNHAltitude((_top.FL * 100)/TOFEET);
  if (_base.Base == abFL) _base.Altitude = AltitudeToQNHAltitude((_base.FL * 100)/TOFEET);
}

bool CAirspace::GetFarVisible(const rectObj &bounds_active) const
{
  return (msRectOverlap(&_bounds, &bounds_active) == MS_TRUE);
	  // These are redundant here, msRectOverlap returns true in that cases also.
	  //||
	  //(msRectContained(bounds_active, &_bounds) == MS_TRUE) ||
	  //(msRectContained(&_bounds, bounds_active) == MS_TRUE);
}

inline bool CheckInsideLongitude(const double &longitude, const double &lon_min, const double &lon_max)
{
  if (lon_min<=lon_max) {
    // normal case
    return ((longitude>lon_min) && (longitude<lon_max));
  } else {
    // area goes across 180 degree boundary, so lon_min is +ve, lon_max is -ve (flipped)
    return ((longitude>lon_min) || (longitude<lon_max));
  }
}

//
// CAIRSPACE_CIRCLE CLASS
//
CAirspace_Circle::CAirspace_Circle(const double &Center_Latitude, const double &Center_Longitude, const double &Airspace_Radius):
		CAirspace(),
		_latcenter(Center_Latitude),
		_loncenter(Center_Longitude),
		_radius(Airspace_Radius)
{
	CalcBounds();
	AirspaceAGLLookup(Center_Latitude, Center_Longitude); 
}


void CAirspace_Circle::Dump() const
{
  StartupStore(TEXT("CAirspace_Circle Dump, CenterLat:%lf, CenterLon:%lf, Radius:%lf%s"), _latcenter, _loncenter, _radius, NEWLINE);
  CAirspace::Dump();
}


bool CAirspace_Circle::Inside(const double &longitude, const double &latitude) const
{
  double bearing;
  if ((latitude> _bounds.miny) &&
	  (latitude< _bounds.maxy) &&
	  CheckInsideLongitude(longitude, _bounds.minx, _bounds.maxx)
	) {
	if (Range(longitude, latitude, bearing)<0) {
	  return true;
	}
  }
  return false;
}

double CAirspace_Circle::Range(const double &longitude, const double &latitude, double &bearing) const
{
  double distance;
  DistanceBearing(latitude,longitude,
                  _latcenter, 
                  _loncenter,
                  &distance, &bearing);
  return distance - _radius;
}


void CAirspace_Circle::ScanCircleBounds(double bearing)
{
  double lat, lon;
  FindLatitudeLongitude(_latcenter, _loncenter, 
                        bearing, _radius,
                        &lat, &lon);

  _bounds.minx = min(lon, _bounds.minx);
  _bounds.maxx = max(lon, _bounds.maxx);
  _bounds.miny = min(lat, _bounds.miny);
  _bounds.maxy = max(lat, _bounds.maxy);
}


void CAirspace_Circle::CalcBounds() 
{
	_bounds.minx = _loncenter;
    _bounds.maxx = _loncenter;
    _bounds.miny = _latcenter;
    _bounds.maxy = _latcenter;
    ScanCircleBounds(0);
    ScanCircleBounds(90);
    ScanCircleBounds(180);
    ScanCircleBounds(270);

    // JMW detect airspace that wraps across 180
    if ((_bounds.minx< -90) && (_bounds.maxx>90)) {
      double tmp = _bounds.minx;
      _bounds.minx = _bounds.maxx;
      _bounds.maxx = tmp;
    }
}

void CAirspace_Circle::CalculateScreenPosition(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const double &ResMapScaleOverDistanceModify) 
{
  _drawstyle = adsHidden;
  if (iAirspaceMode[_type]%2 == 1) {
    double basealt;
    double topalt;
    if (_base.Base != abAGL) {
      basealt = _base.Altitude;
    } else {
      basealt = _base.AGL + CALCULATED_INFO.TerrainAlt;
    }
    if (_top.Base != abAGL) {
      topalt = _top.Altitude;
    } else {
      topalt = _top.AGL + CALCULATED_INFO.TerrainAlt;
    }
    if(CAirspaceManager::Instance().CheckAirspaceAltitude(basealt, topalt)) {
      if (msRectOverlap(&_bounds, &screenbounds_latlon) 
         // || msRectContained(&screenbounds_latlon, &_bounds) is redundant here, msRectOverlap also returns true on containing!
		 ) {

	if (!_newwarnacknobrush &&
	    !(iAirspaceBrush[_type] == NUMAIRSPACEBRUSHES-1)) {
	  _drawstyle = adsFilled;
	} else {
	  _drawstyle = adsOutline;
	}

        MapWindow::LatLon2Screen(_loncenter, _latcenter, _screencenter);
        _screenradius = iround(_radius * ResMapScaleOverDistanceModify);
      }
    }
  }
}

void CAirspace_Circle::Draw(HDC hDCTemp, const RECT &rc, bool param1) const
{
  Circle(hDCTemp, _screencenter.x, _screencenter.y, _screenradius ,rc, true, param1);
}


//
// CAIRSPACE AREA CLASS
//
void CAirspace_Area::Dump() const
{
  CGeoPointList::const_iterator i;

  StartupStore(TEXT("CAirspace_Area Dump%s"), NEWLINE);
  CAirspace::Dump();
  for (i = _geopoints.begin(); i != _geopoints.end(); ++i) {
	StartupStore(TEXT("  Point lat:%lf, lon:%lf%s"), i->Latitude(), i->Longitude(), NEWLINE);
  }
}

void CAirspace_Area::ScreenClosestPoint(const POINT &p1, const POINT &p2, 
			const POINT &p3, POINT *p4, int offset) const
{

  int v12x, v12y, v13x, v13y;

  v12x = p2.x-p1.x; v12y = p2.y-p1.y;
  v13x = p3.x-p1.x; v13y = p3.y-p1.y;

  int mag12 = isqrt4(v12x*v12x+v12y*v12y);
  if (mag12>1) {
    // projection of v13 along v12 = v12.v13/|v12|
    int proj = (v12x*v13x+v12y*v13y)/mag12;
    // fractional distance
    double f;
    if (offset>0) {
      if (offset*2<mag12) {
	proj = max(0, min(proj, mag12));
	proj = max(offset, min(mag12-offset, proj+offset));
      } else {
	proj = mag12/2;
      }
    } 
    f = min(1.0,max(0.0,(double)proj/mag12));

    // location of 'closest' point 
    p4->x = lround(v12x*f)+p1.x;
    p4->y = lround(v12y*f)+p1.y;
  } else {
    p4->x = p1.x;
    p4->y = p1.y;
  }
}


// this one uses screen coordinates to avoid as many trig functions
// as possible.. it means it is approximate but for our use it is ok.
double CAirspace_Area::ScreenCrossTrackError(double lon1, double lat1,
		     double lon2, double lat2,
		     double lon3, double lat3,
		     double *lon4, double *lat4) const
{
  POINT p1, p2, p3, p4;
  
  MapWindow::LatLon2Screen(lon1, lat1, p1);
  MapWindow::LatLon2Screen(lon2, lat2, p2);
  MapWindow::LatLon2Screen(lon3, lat3, p3);

  ScreenClosestPoint(p1, p2, p3, &p4, 0);

  MapWindow::Screen2LatLon(p4.x, p4.y, *lon4, *lat4);
  
  // compute accurate distance
  double tmpd;
  DistanceBearing(lat3, lon3, *lat4, *lon4, &tmpd, NULL); 
  return tmpd;
}


///////////////////////////////////////////////////

// Copyright 2001, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.

//    a Point is defined by its coordinates {int x, y;}
//===================================================================

// isLeft(): tests if a point is Left|On|Right of an infinite line.
//    Input:  three points P0, P1, and P2
//    Return: >0 for P2 left of the line through P0 and P1
//            =0 for P2 on the line
//            <0 for P2 right of the line
//    See: the January 2001 Algorithm "Area of 2D and 3D Triangles and Polygons"
inline static double
isLeft( const CGeoPoint &P0, const CGeoPoint &P1, const double &longitude, const double &latitude )
{
    return ( (P1.Longitude() - P0.Longitude()) * (latitude - P0.Latitude())
            - (longitude - P0.Longitude()) * (P1.Latitude() - P0.Latitude()) );
}

// wn_PnPoly(): winding number test for a point in a polygon
//      Input:   P = a point,
//               V[] = vertex points of a polygon V[n+1] with V[n]=V[0]
//      Return:  wn = the winding number (=0 only if P is outside V[])
int CAirspace_Area::wn_PnPoly( const double &longitude, const double &latitude ) const
{
  int    wn = 0;    // the winding number counter

  // loop through all edges of the polygon
  CGeoPointList::const_iterator it = _geopoints.begin();
  CGeoPointList::const_iterator itnext = it;
  ++itnext;
  for (int i=0; i<((int)_geopoints.size()-1); ++i, ++it, ++itnext) {
		if (it->Latitude() <= latitude) {         // start y <= P.Latitude
			if (itnext->Latitude() > latitude)      // an upward crossing
				if (isLeft( *it, *itnext, longitude, latitude) > 0)  // P left of edge
					++wn;            // have a valid up intersect
		} else {                       // start y > P.Latitude (no test needed)
			if (itnext->Latitude() <= latitude)     // a downward crossing
				if (isLeft( *it, *itnext, longitude, latitude) < 0)  // P right of edge
					--wn;            // have a valid down intersect
		}
	}
	return wn;
}


bool CAirspace_Area::Inside(const double &longitude, const double &latitude) const
{
  if (_geopoints.size() < 3) return false;
  // first check if point is within bounding box
  if (
	  (latitude> _bounds.miny)&&
	  (latitude< _bounds.maxy)&&
	  CheckInsideLongitude(longitude, _bounds.minx, _bounds.maxx)
	) {
	  // it is within, so now do detailed polygon test
	  if (wn_PnPoly(longitude, latitude) != 0) {
		// we are inside the i'th airspace area
		return true;
	  }
  }
  return false;
}


double CAirspace_Area::Range(const double &longitude, const double &latitude, double &bearing) const
{
  // find nearest distance to line segment
  unsigned int i;
  double dist= 0;
  double nearestdistance = dist;
  double nearestbearing = bearing;
  double lon4, lat4;
  int    wn = 0;    // the winding number counter
  
  CGeoPointList::const_iterator it = _geopoints.begin();
  CGeoPointList::const_iterator itnext = it;
  ++itnext;
  
  for (i=0; i<_geopoints.size()-1; ++i) {
    dist = ScreenCrossTrackError(
				 it->Longitude(),
				 it->Latitude(),
				 itnext->Longitude(),
				 itnext->Latitude(),
				 longitude, latitude,
				 &lon4, &lat4);
	
	if (it->Latitude() <= latitude) {         // start y <= P.Latitude
		if (itnext->Latitude() > latitude)      // an upward crossing
			if (isLeft( *it, *itnext, longitude, latitude) > 0)  // P left of edge
				++wn;            // have a valid up intersect
	} else {                       // start y > P.Latitude (no test needed)
		if (itnext->Latitude() <= latitude)     // a downward crossing
			if (isLeft( *it, *itnext, longitude, latitude) < 0)  // P right of edge
				--wn;            // have a valid down intersect
	}

	if ((dist<nearestdistance)||(i==0)) {
      nearestdistance = dist;
      DistanceBearing(latitude, longitude,
                      lat4, lon4, NULL, 
                      &nearestbearing);
    }
    ++it;
	++itnext;
  }
  bearing = nearestbearing;
  if (wn!=0) return -nearestdistance; else return nearestdistance;
}

void CAirspace_Area::SetPoints(CGeoPointList &Area_Points)
{
	POINT p;
	_geopoints = Area_Points;
	_screenpoints.clear();
	for (unsigned int i=0; i<_geopoints.size(); ++i) _screenpoints.push_back(p);
	CalcBounds();
	AirspaceAGLLookup( (_bounds.miny+_bounds.maxy)/2.0, (_bounds.minx+_bounds.maxx)/2.0 ); 
}

void CAirspace_Area::CalcBounds()
{
  list <CGeoPoint>::iterator it = _geopoints.begin();
  
  _bounds.minx = it->Longitude();
  _bounds.maxx = it->Longitude();
  _bounds.miny = it->Latitude();
  _bounds.maxy = it->Latitude();
  for(it = _geopoints.begin(); it != _geopoints.end(); ++it) {
	_bounds.minx = min(it->Longitude(), _bounds.minx);
	_bounds.maxx = max(it->Longitude(), _bounds.maxx);
	_bounds.miny = min(it->Latitude(), _bounds.miny);
	_bounds.maxy = max(it->Latitude(), _bounds.maxy);
  }

  // JMW detect airspace that wraps across 180
  if ((_bounds.minx< -90) && (_bounds.maxx>90)) {
	double tmp = _bounds.minx;
	_bounds.minx = _bounds.maxx;
	_bounds.maxx = tmp;
	for(it = _geopoints.begin(); it != _geopoints.end(); ++it) {
	  if (it->Longitude()<0) it->Longitude(it->Longitude() + 360);
	}
  }
}

void CAirspace_Area::CalculateScreenPosition(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const double &ResMapScaleOverDistanceModify) 
{
  _drawstyle = adsHidden;
  if (iAirspaceMode[_type]%2 == 1) {
    double basealt;
    double topalt;
    if (_base.Base != abAGL) {
      basealt = _base.Altitude;
    } else {
      basealt = _base.AGL + CALCULATED_INFO.TerrainAlt;
    }
    if (_top.Base != abAGL) {
      topalt = _top.Altitude;
    } else {
      topalt = _top.AGL + CALCULATED_INFO.TerrainAlt;
    }
    if(CAirspaceManager::Instance().CheckAirspaceAltitude(basealt, topalt)) {
      if (msRectOverlap(&_bounds, &screenbounds_latlon) 
         // || msRectContained(&screenbounds_latlon, &_bounds) is redundant here, msRectOverlap also returns true on containing!
		 ) {

	if (!_newwarnacknobrush &&
	    !(iAirspaceBrush[_type] == NUMAIRSPACEBRUSHES-1)) {
	  _drawstyle = adsFilled;
	} else {
	  _drawstyle = adsOutline;
	}
	CGeoPointList::iterator it;
	POINTList::iterator itr;
	for (it = _geopoints.begin(), itr = _screenpoints.begin(); it != _geopoints.end(); ++it, ++itr) {
        MapWindow::LatLon2Screen(it->Longitude(), it->Latitude(), *itr);
	}
      }
    }
  }
}

void CAirspace_Area::Draw(HDC hDCTemp, const RECT &rc, bool param1) const
{
  ClipPolygon(hDCTemp, (POINT*)&(*_screenpoints.begin()), _screenpoints.size(), rc, param1);
}



//
// CAIRSPACEMANAGER CLASS
//

bool CAirspaceManager::StartsWith(const TCHAR *Text, const TCHAR *LookFor) const
{
  while(1) {
    if (!(*LookFor)) return TRUE;
    if (*Text != *LookFor) return FALSE;
    ++Text; ++LookFor;
  }
}

bool CAirspaceManager::CheckAirspaceAltitude(const double &Base, const double &Top) const
{
  double alt;
  if (GPS_INFO.BaroAltitudeAvailable) {
    alt = GPS_INFO.BaroAltitude;
  } else {
    alt = GPS_INFO.Altitude;
  }

  switch (AltitudeMode)
    {
    case ALLON : return TRUE;
		
    case CLIP : 
      if(Base < ClipAltitude)
	return TRUE;
      else
	return FALSE;

    case AUTO:
      if( ( alt > (Base - AltWarningMargin) ) 
	  && ( alt < (Top + AltWarningMargin) ))
	return TRUE;
      else
	return FALSE;

    case ALLBELOW:
      if(  (Base - AltWarningMargin) < alt )
	return  TRUE;
      else
	return FALSE;
    case INSIDE:
      if( ( alt >= (Base) ) && ( alt < (Top) ))
	return TRUE;
      else
        return FALSE;
    case ALLOFF : return FALSE;
    }
  return TRUE;
}

void CAirspaceManager::ReadAltitude(const TCHAR *Text, AIRSPACE_ALT *Alt) const
{
  TCHAR *Stop;
  TCHAR sTmp[128];
  TCHAR *pWClast = NULL;
  TCHAR *pToken;
  bool  fHasUnit=false;

  _tcsncpy(sTmp, Text, sizeof(sTmp)/sizeof(sTmp[0]));
  sTmp[sizeof(sTmp)/sizeof(sTmp[0])-1] = '\0';

  _tcsupr(sTmp);

  pToken = strtok_r(sTmp, (TCHAR*)TEXT(" "), &pWClast);

  Alt->Altitude = 0;
  Alt->FL = 0;
  Alt->AGL = 0;
  Alt->Base = abUndef;

  while((pToken != NULL) && (*pToken != '\0')){

    if (isdigit(*pToken)) {
      double d = (double)StrToDouble(pToken, &Stop);
      if (Alt->Base == abFL){
        Alt->FL = d;
        Alt->Altitude = AltitudeToQNHAltitude((Alt->FL * 100)/TOFEET);
      } else if (Alt->Base == abAGL) {
	Alt->AGL = d;
      } else {
        Alt->Altitude = d;
      }
      if (*Stop != '\0'){
        pToken = Stop;
        continue;
      }

    }

    else if (_tcscmp(pToken, TEXT("GND")) == 0) {
      // JMW support XXXGND as valid, equivalent to XXXAGL
      Alt->Base = abAGL;
      if (Alt->Altitude>0) {
	Alt->AGL = Alt->Altitude;
	Alt->Altitude = 0;
      } else {
	Alt->FL = 0;
	Alt->Altitude = 0;
	Alt->AGL = -1;
	fHasUnit = true;
      }
    }

    else if (_tcscmp(pToken, TEXT("SFC")) == 0) {
      Alt->Base = abAGL;
      Alt->FL = 0;
      Alt->Altitude = 0;
      Alt->AGL = -1;
      fHasUnit = true;
    }

    else if (_tcsstr(pToken, TEXT("FL")) == pToken){ 
      // this parses "FL=150" and "FL150"
      Alt->Base = abFL;
      fHasUnit = true;
      if (pToken[2] != '\0'){// no separator between FL and number
	pToken = &pToken[2];
	continue;
      }
    }

    else if ((_tcscmp(pToken, TEXT("FT")) == 0)
             || (_tcscmp(pToken, TEXT("F")) == 0)){
      Alt->Altitude = Alt->Altitude/TOFEET;
      fHasUnit = true;
    }

    else if (_tcscmp(pToken, TEXT("MSL")) == 0){
      Alt->Base = abMSL;
    }

    else if (_tcscmp(pToken, TEXT("M")) == 0){
      // JMW must scan for MSL before scanning for M
      fHasUnit = true;
    }

    else if (_tcscmp(pToken, TEXT("AGL")) == 0){
      Alt->Base = abAGL;
      Alt->AGL = Alt->Altitude;
      Alt->Altitude = 0;
    }

    else if (_tcscmp(pToken, TEXT("STD")) == 0){
      if (Alt->Base != abUndef) {
        // warning! multiple base tags
      }
      Alt->Base = abFL;
      Alt->FL = (Alt->Altitude * TOFEET) / 100;
      Alt->Altitude = AltitudeToQNHAltitude((Alt->FL * 100)/TOFEET);

    }

    else if (_tcscmp(pToken, TEXT("UNL")) == 0) {
      // JMW added Unlimited (used by WGC2008)
      Alt->Base = abMSL;
      Alt->AGL = -1;
      Alt->Altitude = 50000;
    }

    pToken = strtok_r(NULL, (TCHAR*)TEXT(" \t"), &pWClast);

  }

  if (!fHasUnit && (Alt->Base != abFL)) {
    // ToDo warning! no unit defined use feet or user alt unit
    // Alt->Altitude = Units::ToSysAltitude(Alt->Altitude);
    Alt->Altitude = Alt->Altitude/TOFEET;
    Alt->AGL = Alt->AGL/TOFEET;
  }

  if (Alt->Base == abUndef) {
    // ToDo warning! no base defined use MSL
    Alt->Base = abMSL;
  }

}

bool CAirspaceManager::ReadCoords(TCHAR *Text, double *X, double *Y) const
{
  double Ydeg=0, Ymin=0, Ysec=0;
  double Xdeg=0, Xmin=0, Xsec=0;
  TCHAR *Stop;

  // ToDo, add more error checking and making it more tolerant/robust

  Ydeg = (double)StrToDouble(Text, &Stop);
  if ((Text == Stop) || (*Stop =='\0')) return false;
  Stop++;
  Ymin = (double)StrToDouble(Stop, &Stop);
  if (Ymin<0 || Ymin >=60){
    // ToDo
  }
  if (*Stop =='\0') return false;
  if(*Stop == ':'){
    Stop++;
    if (*Stop =='\0') return false;
    Ysec = (double)StrToDouble(Stop, &Stop);
    if (Ysec<0 || Ysec >=60) {
      // ToDo
    }
  }

  *Y = Ysec/3600 + Ymin/60 + Ydeg;

  if (*Stop == ' ')
    Stop++;

  if (*Stop =='\0') return false;
  if((*Stop == 'S') || (*Stop == 's'))
    {
      *Y = *Y * -1;
    }
  Stop++;
  if (*Stop =='\0') return false;

  Xdeg = (double)StrToDouble(Stop, &Stop);
  Stop++;
  Xmin = (double)StrToDouble(Stop, &Stop);
  if(*Stop == ':'){
    Stop++;
    if (*Stop =='\0') return false;
    Xsec = (double)StrToDouble(Stop, &Stop);
  }

  *X = Xsec/3600 + Xmin/60 + Xdeg;

  if (*Stop == ' ')
    Stop++;
  if (*Stop =='\0') return false;
  if((*Stop == 'W') || (*Stop == 'w'))
    {
      *X = *X * -1;
    }

  if (*X<-180) {
    *X+= 360;
  }
  if (*X>180) {
    *X-= 360;
  }

  return true;
}


bool CAirspaceManager::CalculateArc(TCHAR *Text, CGeoPointList *_geopoints, double &CenterX, const double &CenterY, const int &Rotation) const
{
  double StartLat, StartLon;
  double EndLat, EndLon;
  double StartBearing;
  double EndBearing;
  double Radius;
  TCHAR *Comma = NULL;
  CGeoPoint newpoint(0,0);
  double lat,lon;

  ReadCoords(Text,&StartLon , &StartLat);
	
  Comma = _tcschr(Text,',');
  if(!Comma)
    return false;

  ReadCoords(&Comma[1],&EndLon , &EndLat);

  DistanceBearing(CenterY, CenterX, StartLat, StartLon, 
                  &Radius, &StartBearing);
  DistanceBearing(CenterY, CenterX, EndLat, EndLon, 
                  NULL, &EndBearing);
  newpoint.Latitude(StartLat);
  newpoint.Longitude(StartLon);
  _geopoints->push_back(newpoint);
  
  while(fabs(EndBearing-StartBearing) > 7.5)
  {
	  StartBearing += Rotation *5 ;
	  if(StartBearing > 360) StartBearing -= 360;
	  if(StartBearing < 0) StartBearing += 360;
	  FindLatitudeLongitude(CenterY, CenterX, StartBearing, Radius, &lat, &lon);
	  newpoint.Latitude(lat);
	  newpoint.Longitude(lon);
	  _geopoints->push_back(newpoint);
  }
  newpoint.Latitude(EndLat);
  newpoint.Longitude(EndLon);
  _geopoints->push_back(newpoint);
  return true;
}

bool CAirspaceManager::CalculateSector(TCHAR *Text, CGeoPointList *_geopoints, double &CenterX, const double &CenterY, const int &Rotation) const
{
  double Radius;
  double StartBearing;
  double EndBearing;
  TCHAR *Stop;
  CGeoPoint newpoint(0,0);
  double lat=0,lon=0;

  Radius = NAUTICALMILESTOMETRES * (double)StrToDouble(Text, &Stop);
  StartBearing = (double)StrToDouble(&Stop[1], &Stop);
  EndBearing = (double)StrToDouble(&Stop[1], &Stop);

  while(fabs(EndBearing-StartBearing) > 7.5)
  {
    if(StartBearing >= 360) StartBearing -= 360;
    if(StartBearing < 0) StartBearing += 360;

	FindLatitudeLongitude(CenterY, CenterX, StartBearing, Radius, &lat, &lon);

	newpoint.Latitude(lat);
	newpoint.Longitude(lon);
	_geopoints->push_back(newpoint);
    
	StartBearing += Rotation *5 ;
  }
  FindLatitudeLongitude(CenterY, CenterX, EndBearing, Radius, &lat, &lon);
  newpoint.Latitude(lat);
  newpoint.Longitude(lon);
  _geopoints->push_back(newpoint);
  return true;
}


// Reading and parsing OpenAir airspace file
void CAirspaceManager::FillAirspacesFromOpenAir(ZZIP_FILE *fp)
{
  TCHAR	*Comment;
  int		nSize;
  TCHAR Text[READLINE_LENGTH];
  TCHAR sTmp[READLINE_LENGTH+100];
  TCHAR *p;
  int linecount=0;
  int parsing_state = 0;
  CAirspace *newairspace = NULL;
  // Variables to store airspace parameters
  TCHAR Name[NAME_SIZE+1];
  CGeoPointList points;
  double Radius = 0;
  double Latitude = 0;
  double Longitude = 0;
  int Type = 0;
  AIRSPACE_ALT Base;
  AIRSPACE_ALT Top;
  int Rotation = 1;
  double CenterX = 0;
  double CenterY = 0;
  double lat=0,lon=0;

  
  StartupStore(TEXT(". Reading airspace file%s"),NEWLINE);

  while (ReadString(fp, READLINE_LENGTH, Text)){
	++linecount;
	p = Text;
	//Skip whitespaces
	while (*p!=0 && isspace(*p)) p++;
	if (*p==0) continue;
	//Skip comment lines
	if (*p=='*') continue;
	_tcsupr(p);
	// Strip comments and newline chars from end of line
	Comment = _tcschr(p, _T('*'));
	if(Comment != NULL)
	{
		*Comment = _T('\0');		// Truncate line
		nSize = Comment - p;		// Reset size
		if (nSize < 3)
		  continue;				// Ensure newline removal won't fail
	}
    nSize = _tcsclen(p);
	if(p[nSize-1] == _T('\n')) p[--nSize] = _T('\0');
	if(p[nSize-1] == _T('\r')) p[--nSize] = _T('\0');

//	StartupStore(TEXT(".  %s%s"),p,NEWLINE);

	switch (*p) {
	  case _T('A'):
		p++; // skip A
		switch (*p) {
		  case _T('C'):	//AC
			p++; // skip C
			if (parsing_state==10) { // New airspace begin, store the old one, reset parser
			  if (Radius>0) {
				// Last one was a circle
				newairspace = new CAirspace_Circle(Longitude, Latitude, Radius);
			  } else {
				  // Last one was an area
				  // Skip it if we dont have minimum 3 points
				  if (points.size()<3) {
				  }
				  newairspace = new CAirspace_Area;
	  			  newairspace->SetPoints(points);
				}
			  newairspace->Init(Name, Type, Base, Top);

			  if (1) {
				CCriticalSection::CGuard guard(_csairspaces);
				_airspaces.push_back(newairspace);
			  }
			  
			  Name[0]='\0';
			  Radius = 0;
			  Longitude = 0;
			  Latitude = 0;
			  points.clear();
			  Type = 0;
			  Base.Base = abUndef;
			  Top.Base = abUndef;
			  newairspace = NULL;
			  parsing_state = 0;
			}
			if (parsing_state==0) {	// New AC
			  p++; //Skip space
			  Rotation = +1;
			  for (int i = 0; i < k_nAreaCount; ++i) {
				if (StartsWith(p, k_strAreaStart[i])) {
					Type = k_nAreaType[i];
					parsing_state = 10;
					break;
				}
			  }
			}  
			break;

		  case _T('N'): //AN - Airspace name
			p++; p++;
			if (parsing_state == 10) _tcsncpy(Name, p, NAME_SIZE);
            break;
            
          case _T('L'): //AL - base altitude
			p++; p++;
			if (parsing_state == 10) ReadAltitude(p, &Base);
            break;
            
          case _T('H'): //AH - top altitude
			p++; p++;
			if (parsing_state == 10) ReadAltitude(p, &Top);
            break;
            
          case _T('T'): // ignore airspace labels
            // TODO: adding airspace labels
            continue;
			
		  default:
			wsprintf(sTmp, TEXT("Parse error at line %d\r\n\"%s\"\r\nLine skipped."), linecount, p );
			// LKTOKEN  _@M68_ = "Airspace" 
			if (MessageBoxX(NULL, sTmp, gettext(TEXT("_@M68_")), MB_OKCANCEL) == IDCANCEL) return;
			break;
		} //sw
		break;
	  
      case _T('D'):
		p++;
		switch (*p) {
          case _T('A'): //DA - Sector
			if (!CalculateSector(p, &points, CenterX, CenterY, Rotation)) {
			  wsprintf(sTmp, TEXT("Parse error at line %d\r\n\"%s\"\r\nLine skipped."), linecount, p );
			  // LKTOKEN  _@M68_ = "Airspace" 
			  if (MessageBoxX(NULL, sTmp, gettext(TEXT("_@M68_")), MB_OKCANCEL) == IDCANCEL) return;
			}
            break;
            
          case _T('B'): //DB - Arc
			p++; p++; // skip B and space
			if (!CalculateArc(p, &points, CenterX, CenterY, Rotation)) {
			  wsprintf(sTmp, TEXT("Parse error at line %d\r\n\"%s\"\r\nLine skipped."), linecount, p );
			  // LKTOKEN  _@M68_ = "Airspace" 
			  if (MessageBoxX(NULL, sTmp, gettext(TEXT("_@M68_")), MB_OKCANCEL) == IDCANCEL) return;
			}
            break;
            
          case _T('C'): //DC - Circle
			p++; p++;
			Radius = StrToDouble(p,NULL);
			Radius = (Radius * NAUTICALMILESTOMETRES);
			Latitude = CenterX;
			Longitude = CenterY;
            break;
            
          case _T('P'): //DP - polygon point
			p++; p++; // skip P and space
			if (ReadCoords(p,&lon, &lat)) {
			  points.push_back(CGeoPoint(lat,lon));
			} else {
			  wsprintf(sTmp, TEXT("Parse error at line %d\r\n\"%s\"\r\nLine skipped."), linecount, p );
			  // LKTOKEN  _@M68_ = "Airspace" 
			  if (MessageBoxX(NULL, sTmp, gettext(TEXT("_@M68_")), MB_OKCANCEL) == IDCANCEL) return;
			}
            break;
            
            // todo DY airway segment
            // what about 'V T=' ?
		  default:
			wsprintf(sTmp, TEXT("Parse error at line %d\r\n\"%s\"\r\nLine skipped."), linecount, p );
			// LKTOKEN  _@M68_ = "Airspace" 
			if (MessageBoxX(NULL, sTmp, gettext(TEXT("_@M68_")), MB_OKCANCEL) == IDCANCEL) return;
			break;
		} //sw
		break;
	  
      case _T('V'):
		p++; p++; //skip V and space
		if(StartsWith(p, TEXT("X="))) {
			if (ReadCoords(p+2,&CenterX, &CenterY))
			  break;
		  }
		else if(StartsWith(p,TEXT("D=-")))
		  {
			Rotation = -1;
			break;
		  }
		else if(StartsWith(p,TEXT("D=+")))
		  {
			Rotation = +1;
			break;
		  }
		else if(StartsWith(p,TEXT("Z")))
		  {
			// ToDo Display Zool Level
			break;
		  }
		else if(StartsWith(p,TEXT("W")))
		  {
			// ToDo width of an airway
			break;
		  }
		else if(StartsWith(p,TEXT("T")))
		  {
			// ----- JMW THIS IS REQUIRED FOR LEGACY FILES
			break;
		  }

		wsprintf(sTmp, TEXT("Parse error at line %d\r\n\"%s\"\r\nLine skipped."), linecount, p );
		// LKTOKEN  _@M68_ = "Airspace" 
		if (MessageBoxX(NULL, sTmp, gettext(TEXT("_@M68_")), MB_OKCANCEL) == IDCANCEL) return;
		break;
        
      case _T('S'):  // ignore the SB,SP ...
		p++;
        if (*p == _T('B')) continue;
        if (*p == _T('P')) continue;
		// if none of the above, then falling to default
		
	  default:
		wsprintf(sTmp, TEXT("Parse error at line %d\r\n\"%s\"\r\nLine skipped."), linecount, p );
		// LKTOKEN  _@M68_ = "Airspace" 
		if (MessageBoxX(NULL, sTmp, gettext(TEXT("_@M68_")), MB_OKCANCEL) == IDCANCEL) return;
		break;
	}//sw
	
  }//wh readline

  // Push last one to the list
  if (parsing_state==10) {
	if (Radius>0) {
	  // Last one was a circle
	  newairspace = new CAirspace_Circle(Longitude, Latitude, Radius);
	} else {
		// Last one was an area
		// Skip it if we dont have minimum 3 points
		if (points.size()<3) {
		}
		newairspace = new CAirspace_Area();
		newairspace->SetPoints(points);
	}
	newairspace->Init(Name, Type, Base, Top);
	CCriticalSection::CGuard guard(_csairspaces);
	_airspaces.push_back(newairspace);
  }

  CCriticalSection::CGuard guard(_csairspaces);
  StartupStore(TEXT(". Readed %d airspaces%s"), _airspaces.size(), NEWLINE);
//  list<CAirspace*>::iterator it;
//  for ( it = _airspaces.begin(); it != _airspaces.end(); ++it) (*it)->Dump();
}


void CAirspaceManager::ReadAirspaces()
{
  TCHAR	szFile1[MAX_PATH] = TEXT("\0");
  TCHAR	szFile2[MAX_PATH] = TEXT("\0");
  char zfilename[MAX_PATH];

  ZZIP_FILE *fp=NULL;
  ZZIP_FILE *fp2=NULL;

#if AIRSPACEUSEBINFILE > 0
  FILETIME LastWriteTime;
  FILETIME LastWriteTime2;
#endif

  GetRegistryString(szRegistryAirspaceFile, szFile1, MAX_PATH);
  ExpandLocalPath(szFile1);
  GetRegistryString(szRegistryAdditionalAirspaceFile, szFile2, MAX_PATH);
  ExpandLocalPath(szFile2);

  if (_tcslen(szFile1)>0) {
    unicode2ascii(szFile1, zfilename, MAX_PATH);
    fp  = zzip_fopen(zfilename, "rt");
  } else {
    //* 091206 back on 
    static TCHAR  szMapFile[MAX_PATH] = TEXT("\0");
    GetRegistryString(szRegistryMapFile, szMapFile, MAX_PATH);
    ExpandLocalPath(szMapFile);
    wcscat(szMapFile,TEXT("/"));
    wcscat(szMapFile,TEXT(LKF_AIRSPACES)); // 091206
    unicode2ascii(szMapFile, zfilename, MAX_PATH);
    fp  = zzip_fopen(zfilename, "rt");
    //*/
  }

  if (_tcslen(szFile2)>0) {
    unicode2ascii(szFile2, zfilename, MAX_PATH);
    fp2 = zzip_fopen(zfilename, "rt");
  }

  SetRegistryString(szRegistryAirspaceFile, TEXT("\0"));
  SetRegistryString(szRegistryAdditionalAirspaceFile, TEXT("\0"));

  if (fp != NULL){
	FillAirspacesFromOpenAir(fp);
    zzip_fclose(fp);

    // file 1 was OK, so save it
    ContractLocalPath(szFile1);
    SetRegistryString(szRegistryAirspaceFile, szFile1);

    // also read any additional airspace
    if (fp2 != NULL) {
	  FillAirspacesFromOpenAir(fp2);
      zzip_fclose(fp2);
      
      // file 2 was OK, so save it
      ContractLocalPath(szFile2);
      SetRegistryString(szRegistryAdditionalAirspaceFile, szFile2);
    } else {
      StartupStore(TEXT(". No airspace file 2%s"),NEWLINE);
    }
  } else {
    StartupStore(TEXT("... No airspace file 1%s"),NEWLINE);
  }
}


void CAirspaceManager::CloseAirspaces()
{
  CAirspaceList::iterator it;
  
  AirspaceWarnListClear();
  CCriticalSection::CGuard guard(_csairspaces);

  for ( it = _airspaces.begin(); it != _airspaces.end(); ++it) delete *it;
  _airspaces.clear();
  _airspaces_near.clear();
}

void CAirspaceManager::QnhChangeNotify(const double &newQNH)
{
  static double lastQNH;
  static bool first = true;
  
  if ( (newQNH != lastQNH) || first) {
	CAirspaceList::iterator i;
	CCriticalSection::CGuard guard(_csairspaces);

	for(i= _airspaces.begin(); i != _airspaces.end(); ++i) (*i)->QnhChangeNotify();

	first = false;
    lastQNH = newQNH; 
  }
}


void CAirspaceManager::ScanAirspaceLine(double lats[], double lons[], double heights[], 
		      int airspacetype[AIRSPACE_SCANSIZE_H][AIRSPACE_SCANSIZE_X]) const
{		      

  int i,j;
  double x1 = lons[0];
  double dx = lons[AIRSPACE_SCANSIZE_X-1]-x1;
  double y1 = lats[0];
  double dy = lats[AIRSPACE_SCANSIZE_X-1]-y1;
  double h_min = heights[0];
  double h_max = heights[AIRSPACE_SCANSIZE_H-1];

  rectObj lineRect;
  bool inside;
  
  lineRect.minx = min(x1, x1+dx);
  lineRect.maxx = max(x1, x1+dx);
  lineRect.miny = min(y1, y1+dy);
  lineRect.maxy = max(y1, y1+dy);

  CAirspaceList::const_iterator it;
  CCriticalSection::CGuard guard(_csairspaces);

  for (it = _airspaces.begin(); it != _airspaces.end(); ++it) {
	// ignore if outside scan height
	if ( !((h_max<=(*it)->Base()->Altitude) || (h_min>=(*it)->Top()->Altitude)) ) {
	  const rectObj &pbounds = (*it)->Bounds();
	  // ignore if scan line doesn't intersect bounds
	  if (msRectOverlap(&lineRect, &pbounds)) {
		for (i=0; i<AIRSPACE_SCANSIZE_X; i++) {
			inside = (*it)->Inside(lons[i], lats[i]);
				if (inside) {
					for (j=0; j<AIRSPACE_SCANSIZE_H; j++) {
						if ((heights[j]>(*it)->Base()->Altitude)&&
								(heights[j]<(*it)->Top()->Altitude)) {
							airspacetype[j][i] = (*it)->Type();
						} // inside height
					} // finished scanning height
				} // inside
		} // finished scanning range
	  } // if overlaps bounds
	}//if inside height
  } // for iterator
}


////////////////////////
//
// Finds nearest airspace (whether circle or area) to the specified point.
// Returns -1 in foundcircle or foundarea if circle or area is not found
// Otherwise, returns index of the circle or area that is closest to the specified point.
//
// Also returns the distance and bearing to the boundary of the airspace,
// (TODO enhancement: return also the vertical separation).  
//
// Distance <0 means interior.
//
// This only searches within a range of 100km of the target

const CAirspace* CAirspaceManager::FindNearestAirspace(const double &longitude, const double &latitude,
			 double *nearestdistance, double *nearestbearing, double *height) const
{
  double nearestd = 100000; // 100km
  double nearestb = 0;

  bool iswarn;
  bool isdisplay;
  double basealt;
  double topalt;
  bool altok;
  double bearing;
  CAirspace *found = NULL;
  int type;
  double dist;

  CAirspaceList::const_iterator it;
  CCriticalSection::CGuard guard(_csairspaces);

  for (it = _airspaces.begin(); it != _airspaces.end(); ++it) {
	type = (*it)->Type();
	//TODO check index
    iswarn = (MapWindow::iAirspaceMode[type]>=2);
    isdisplay = ((MapWindow::iAirspaceMode[type]%2)>0);

    if (!isdisplay || !iswarn) {
      // don't want warnings for this one
      continue;
    }

    if ((*it)->Base()->Base != abAGL) {
      basealt = (*it)->Base()->Altitude;
    } else {
      basealt = (*it)->Base()->AGL + CALCULATED_INFO.TerrainAlt;
    }
    if ((*it)->Top()->Base != abAGL) {
      topalt = (*it)->Top()->Altitude;
    } else {
      topalt = (*it)->Top()->AGL + CALCULATED_INFO.TerrainAlt;
    }
    
    if (height) {
      altok = ((*height > basealt) && (*height < topalt));
    } else {
      altok = CheckAirspaceAltitude(basealt, topalt)==TRUE;
    }
    if(altok) {
      
      dist = (*it)->Range(longitude, latitude, bearing);
      
      if(dist < nearestd ) {
		  nearestd = dist;
		  nearestb = bearing;
		  found = *it;
		  if (dist<0) {
				// no need to continue search, inside
				break; //for
		  }
      }
    }
  } //for
  
  if (nearestdistance) *nearestdistance = nearestd;
  if (nearestbearing) *nearestbearing = nearestb;
  return found;
}


void CAirspaceManager::SortAirspaces(void)
{
  StartupStore(TEXT(". SortAirspace%s"),NEWLINE);

  // force acknowledgement before sorting
  ClearAirspaceWarnings(true, false);

//   qsort(AirspaceArea,
// 	NumberOfAirspaceAreas,
// 	sizeof(AIRSPACE_AREA),
// 	SortAirspaceAreaCompare);
// 
//   qsort(AirspaceCircle,
// 	NumberOfAirspaceCircles,
// 	sizeof(AIRSPACE_CIRCLE),
// 	SortAirspaceCircleCompare);

}

bool CAirspaceManager::ValidAirspaces(void) const
{
  CCriticalSection::CGuard guard(_csairspaces);
  bool res = _airspaces.size()>0;
  return res;
}


void CAirspaceManager::AirspaceWarning(NMEA_INFO *Basic, DERIVED_INFO *Calculated) 
{
  if(!AIRSPACEWARNINGS) return;				//Airspace warnings disabled in config

  CCriticalSection::CGuard guard(_csairspaces);

  if ( _airspaces_near.size() == 0 ) return;
  
  //TODO what is this???
  if (_GlobalClearAirspaceWarnings == true) {
    _GlobalClearAirspaceWarnings = false;
    Calculated->IsInAirspace = false;
  }

  double alt = Calculated->NextAltitude;
  double agl = Calculated->NextAltitudeAGL;
  double lat = Calculated->NextLatitude;
  double lon = Calculated->NextLongitude;

  //Check for new airspaces to add
  CAirspaceList::iterator it;
  for (it=_airspaces_near.begin(); it != _airspaces_near.end(); ++it) {
	  // Already in warning list
	  if ((*it)->WarningState() > awsNone) continue;		
	  
	  // Check for warnings enabled for this class
	  if (MapWindow::iAirspaceMode[(*it)->Type()] < 2) continue;
	  
	  // Check for altitude
      if (((((*it)->Base()->Base != abAGL) && (alt >= (*it)->Base()->Altitude))
           || (((*it)->Base()->Base == abAGL) && (agl >= (*it)->Base()->AGL)))
          && ((((*it)->Top()->Base != abAGL) && (alt < (*it)->Top()->Altitude))
           || (((*it)->Top()->Base == abAGL) && (agl < (*it)->Top()->AGL)))) {
		// Check for inside
		if ((*it)->Inside(lon, lat)) {
		  (*it)->WarningState(awsNew);
		  CCriticalSection::CGuard guard(_cswarnlist);
		  _airspaces_warning.push_front(*it);
		  #ifdef DEBUG_AIRSPACE
		  StartupStore(TEXT("LKAIRSP: %s added%s"),(*it)->Name(),NEWLINE );
		  #endif
        }
	  }
  }
  
  //Process warning list
  AirspaceWarnListProcess(Basic, Calculated);

  //TODO What is this?
  NearestAirspaceHDist=0;
}


CAirspaceList CAirspaceManager::GetVisibleAirspacesAtPoint(const double &lon, const double &lat) const
{
  CAirspaceList res;
  CAirspaceList::const_iterator it;
  CCriticalSection::CGuard guard(_csairspaces);
  for (it = _airspaces.begin(); it != _airspaces.end(); ++it) {
	if ((*it)->DrawStyle()) {
	  if ((*it)->Inside(lon, lat)) res.push_back(*it);
	}
  }
  return res;
}

void CAirspaceManager::SetFarVisible(const rectObj &bounds_active) 
{
  bool farvisible;
  CAirspaceList::iterator it;

  CCriticalSection::CGuard guard(_csairspaces);
  _airspaces_near.clear();
  for (it = _airspaces.begin(); it != _airspaces.end(); ++it) {
	farvisible = (*it)->GetFarVisible(bounds_active);
	if (farvisible) _airspaces_near.push_back(*it);
  }
}


void CAirspaceManager::CalculateScreenPositionsAirspace(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const double &ResMapScaleOverDistanceModify)
{
  CAirspaceList::iterator it;

  CCriticalSection::CGuard guard(_csairspaces);
  for (it = _airspaces_near.begin(); it!= _airspaces_near.end(); ++it) {
	(*it)->CalculateScreenPosition(screenbounds_latlon, iAirspaceMode, iAirspaceBrush, ResMapScaleOverDistanceModify);
  }
}

// Passing reference instead
// CAirspaceList CAirspaceManager::GetAirspacesToDraw() const
// {
//   CAirspaceList res;
//   CAirspaceList::const_iterator it;
//   CCriticalSection::CGuard guard(_csairspaces);
// 
//   for (it = _airspaces_near.begin(); it != _airspaces_near.end(); ++it) {
// 	if ((*it)->Visible() == 2) res.push_back(*it);
//   }
//   return res;
// }

const CAirspaceList& CAirspaceManager::GetNearAirspacesRef() const
{
  CCriticalSection::CGuard guard(_csairspaces);
  return _airspaces_near;
}

CAirspaceList CAirspaceManager::GetAllAirspaces() const
{
  CAirspaceList res;
  CCriticalSection::CGuard guard(_csairspaces);
  res = _airspaces;
  return res;
}

#endif /* LKAIRSPACE */
