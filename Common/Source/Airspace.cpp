/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Airspace.cpp,v 8.3 2010/12/10 22:19:10 root Exp root $
*/

#include "externs.h"
#include "Airspace.h"
#include "Utils.h"
#include "MapWindow.h"
#include "RasterTerrain.h"
#include <commctrl.h>
#include <aygshell.h>
#include <ctype.h>


using std::min;
using std::max;


#ifdef DEBUG
void DumpAirspaceFile(void);
#endif

#include <LKAirspace.h>

/////////////////////////////


///////////////////////////////

// this can now be called multiple times to load several airspaces.
// to start afresh, call CloseAirspace()


bool CheckInsideLongitude(double longitude,
                         const double lon_min, const double lon_max) {
  if (lon_min<=lon_max) {
    // normal case
    return ((longitude>lon_min) && (longitude<lon_max));
  } else {
    // area goes across 180 degree boundary, so lon_min is +ve, lon_max is -ve (flipped)
    return ((longitude>lon_min) || (longitude<lon_max));
  }
}


/////////////////////////////////////////////////////////////////////////////////

// this is a slow function
// adapted from The Aviation Formulary 1.42

// finds the point along a distance dthis between p1 and p2, which are
// separated by dtotal
void IntermediatePoint(double lon1, double lat1,
		       double lon2, double lat2,
		       double dthis,
		       double dtotal,
		       double *lon3, double *lat3) {
  double A, B, x, y, z, d, f;
  /*
  lat1 *= DEG_TO_RAD;
  lat2 *= DEG_TO_RAD;
  lon1 *= DEG_TO_RAD;
  lon2 *= DEG_TO_RAD;
  */

  ASSERT(lat3 != NULL);
  ASSERT(lon3 != NULL);

  if ((lon1 == lon2) && (lat1 == lat2)){
    *lat3 = lat1;
    *lon3 = lon1;
    return;
  }

  if (dtotal>0) {
    f = dthis/dtotal;
    d = dtotal;
  } else {
    d = 1.0e-7;
    f = 0.0;
  }
  f = min(1.0,max(0.0,f));

  double coslat1 = cos(lat1);
  double coslat2 = cos(lat2);

  A=sin((1-f)*d)/sin(d);
  B=sin(f*d)/sin(d);
  x = A*coslat1*cos(lon1) +  B*coslat2*cos(lon2);
  y = A*coslat1*sin(lon1) +  B*coslat2*sin(lon2);
  z = A*sin(lat1)           +  B*sin(lat2);
  *lat3=atan2(z,sqrt(x*x+y*y))*RAD_TO_DEG;
  *lon3=atan2(y,x)*RAD_TO_DEG;
}

// finds cross track error in meters and closest point p4 between p3 and
// desired track p1-p2.
// very slow function!
double CrossTrackError(double lon1, double lat1,
                       double lon2, double lat2,
                       double lon3, double lat3,
                       double *lon4, double *lat4) {

  double dist_AD, crs_AD;
  DistanceBearing(lat1, lon1, lat3, lon3, &dist_AD, &crs_AD);
  dist_AD/= (RAD_TO_DEG * 111194.9267); crs_AD*= DEG_TO_RAD;

  double dist_AB, crs_AB;
  DistanceBearing(lat1, lon1, lat2, lon2, &dist_AB, &crs_AB);
  dist_AB/= (RAD_TO_DEG * 111194.9267); crs_AB*= DEG_TO_RAD;

  lat1 *= DEG_TO_RAD;
  lat2 *= DEG_TO_RAD;
  lat3 *= DEG_TO_RAD;
  lon1 *= DEG_TO_RAD;
  lon2 *= DEG_TO_RAD;
  lon3 *= DEG_TO_RAD;

  double XTD; // cross track distance
  double ATD; // along track distance
  //  The "along track distance", ATD, the distance from A along the
  //  course towards B to the point abeam D

  double sindist_AD = sin(dist_AD);

  XTD = asin(sindist_AD*sin(crs_AD-crs_AB));

  double sinXTD = sin(XTD);
  ATD = asin(sqrt( sindist_AD*sindist_AD - sinXTD*sinXTD )/cos(XTD));
  
  if (lon4 && lat4) {
    IntermediatePoint(lon1, lat1, lon2, lat2, ATD, dist_AB,
		      lon4, lat4);
  }

  // units
  XTD *= (RAD_TO_DEG * 111194.9267);

  return XTD;
}


void ScreenClosestPoint(const POINT &p1, const POINT &p2, 
			const POINT &p3, POINT *p4, int offset) {

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


// Calculates projected distance from P3 along line P1-P2
double ProjectedDistance(double lon1, double lat1,
                         double lon2, double lat2,
                         double lon3, double lat3) {
  double lon4, lat4;

  CrossTrackError(lon1, lat1,
                  lon2, lat2,
                  lon3, lat3,
                   &lon4, &lat4);
  double tmpd;
  DistanceBearing(lat1, lon1, lat4, lon4, &tmpd, NULL);
  return tmpd;
}




#ifdef DEBUG
void DumpAirspaceFile(void){

  FILE * fp;
  int i;

  fp  = _tfopen(TEXT("XCSoarAirspace.dmp"), TEXT("wt"));

  for (i=0; i < (int)NumberOfAirspaceAreas; i++){

    _ftprintf(fp, TEXT("*** Aera id: %d %s "), i, AirspaceArea[i].Name);

    switch (AirspaceArea[i].Type){
      case RESTRICT:
        _ftprintf(fp, TEXT("Restricted")); break;
      case PROHIBITED:
        _ftprintf(fp, TEXT("Prohibited")); break;
      case DANGER:
        _ftprintf(fp, TEXT("Danger Area")); break;
      case CLASSA:
        _ftprintf(fp, TEXT("Class A")); break;
      case CLASSB:
        _ftprintf(fp, TEXT("Class B")); break;
      case CLASSC:
        _ftprintf(fp, TEXT("Class C")); break;
      case CLASSD:
        _ftprintf(fp, TEXT("Class D")); break;
      case CLASSE:
        _ftprintf(fp, TEXT("Class E")); break;
      case CLASSF:
        _ftprintf(fp, TEXT("Class F")); break;
      case CLASSG:
        _ftprintf(fp, TEXT("Class G")); break;
      case NOGLIDER:
        _ftprintf(fp, TEXT("No Glider")); break;
      case CTR:
        _ftprintf(fp, TEXT("CTR")); break;
      case WAVE:
        _ftprintf(fp, TEXT("Wave")); break;
      case CLASSTMZ:
        _ftprintf(fp, TEXT("TMZ")); break;
      default:
        _ftprintf(fp, TEXT("Unknown"));
    }

    _ftprintf(fp, TEXT(")\r\n"), i);

    switch (AirspaceArea[i].Top.Base){
      case abUndef:
        _ftprintf(fp, TEXT("  Top  : %.0f[m] %.0f[ft] [?]\r\n"), AirspaceArea[i].Top.Altitude, AirspaceArea[i].Top.Altitude*TOFEET);
      break;
      case abMSL:
        _ftprintf(fp, TEXT("  Top  : %.0f[m] %.0f[ft] [MSL]\r\n"), AirspaceArea[i].Top.Altitude, AirspaceArea[i].Top.Altitude*TOFEET);
      break;
      case abAGL:
        _ftprintf(fp, TEXT("  Top  : %.0f[m] %.0f[ft] [AGL]\r\n"), AirspaceArea[i].Top.AGL, AirspaceArea[i].Top.AGL*TOFEET);
      break;
      case abFL:
        _ftprintf(fp, TEXT("  Top  : FL %.0f (%.0f[m] %.0f[ft])\r\n"), AirspaceArea[i].Top.FL, AirspaceArea[i].Top.Altitude, AirspaceArea[i].Top.Altitude*TOFEET);
      break;
    }

    switch (AirspaceArea[i].Base.Base){
      case abUndef:
        _ftprintf(fp, TEXT("  Base : %.0f[m] %.0f[ft] [?]\r\n"), AirspaceArea[i].Base.Altitude, AirspaceArea[i].Base.Altitude*TOFEET);
      break;
      case abMSL:
        _ftprintf(fp, TEXT("  Base : %.0f[m] %.0f[ft] [MSL]\r\n"), AirspaceArea[i].Base.Altitude, AirspaceArea[i].Base.Altitude*TOFEET);
      break;
      case abAGL:
        _ftprintf(fp, TEXT("  Base : %.0f[m] %.0f[ft] [AGL]\r\n"), AirspaceArea[i].Base.AGL, AirspaceArea[i].Base.AGL*TOFEET);
      break;
      case abFL:
        _ftprintf(fp, TEXT("  Base : FL %.0f (%.0f[m] %.0f[ft])\r\n"), AirspaceArea[i].Base.FL, AirspaceArea[i].Base.Altitude, AirspaceArea[i].Base.Altitude*TOFEET);
      break;
    }

    _ftprintf(fp, TEXT("\r\n"), i);
  }

  for (i=0; i < (int)NumberOfAirspaceCircles; i++){

    _ftprintf(fp, TEXT("\r\n*** Circle id: %d %s ("), i, AirspaceCircle[i].Name);

    switch (AirspaceArea[i].Type){
      case RESTRICT:
        _ftprintf(fp, TEXT("Restricted")); break;
      case PROHIBITED:
        _ftprintf(fp, TEXT("Prohibited")); break;
      case DANGER:
        _ftprintf(fp, TEXT("Danger Area")); break;
      case CLASSA:
        _ftprintf(fp, TEXT("Class A")); break;
      case CLASSB:
        _ftprintf(fp, TEXT("Class B")); break;
      case CLASSC:
        _ftprintf(fp, TEXT("Class C")); break;
      case CLASSD:
        _ftprintf(fp, TEXT("Class D")); break;
      case CLASSE:
        _ftprintf(fp, TEXT("Class E")); break;
      case CLASSF:
        _ftprintf(fp, TEXT("Class F")); break;
      case CLASSG:
        _ftprintf(fp, TEXT("Class G")); break;
      case NOGLIDER:
        _ftprintf(fp, TEXT("No Glider")); break;
      case CTR:
        _ftprintf(fp, TEXT("CTR")); break;
      case WAVE:
        _ftprintf(fp, TEXT("Wave")); break;
      case CLASSTMZ:
        _ftprintf(fp, TEXT("TMZ")); break;
      default:
        _ftprintf(fp, TEXT("Unknown"));
    }

    _ftprintf(fp, TEXT(")\r\n"), i);

    switch (AirspaceCircle[i].Top.Base){
      case abUndef:
        _ftprintf(fp, TEXT("  Top  : %.0f[m] %.0f[ft] [?]\r\n"), AirspaceCircle[i].Top.Altitude, AirspaceCircle[i].Top.Altitude*TOFEET);
      break;
      case abMSL:
        _ftprintf(fp, TEXT("  Top  : %.0f[m] %.0f[ft] [MSL]\r\n"), AirspaceCircle[i].Top.Altitude, AirspaceCircle[i].Top.Altitude*TOFEET);
      break;
      case abAGL:
        _ftprintf(fp, TEXT("  Top  : %.0f[m] %.0f[ft] [AGL]\r\n"), AirspaceCircle[i].Top.AGL, AirspaceCircle[i].Top.AGL*TOFEET);
      break;
      case abFL:
        _ftprintf(fp, TEXT("  Top  : FL %.0f (%.0f[m] %.0f[ft])\r\n"), AirspaceCircle[i].Top.FL, AirspaceCircle[i].Top.Altitude, AirspaceCircle[i].Top.Altitude*TOFEET);
      break;
    }

    switch (AirspaceCircle[i].Base.Base){
      case abUndef:
        _ftprintf(fp, TEXT("  Base : %.0f[m] %.0f[ft] [?]\r\n"), AirspaceCircle[i].Base.Altitude, AirspaceCircle[i].Base.Altitude*TOFEET);
      break;
      case abMSL:
        _ftprintf(fp, TEXT("  Base : %.0f[m] %.0f[ft] [MSL]\r\n"), AirspaceCircle[i].Base.Altitude, AirspaceCircle[i].Base.Altitude*TOFEET);
      break;
      case abAGL:
        _ftprintf(fp, TEXT("  Base : %.0f[m] %.0f[ft] [AGL]\r\n"), AirspaceCircle[i].Base.AGL, AirspaceCircle[i].Base.AGL*TOFEET);
      break;
      case abFL:
        _ftprintf(fp, TEXT("  Base : FL %.0f (%.0f[m] %.0f[ft])\r\n"), AirspaceCircle[i].Base.FL, AirspaceCircle[i].Base.Altitude, AirspaceCircle[i].Base.Altitude*TOFEET);
      break;
    }

  _ftprintf(fp, TEXT("\r\n"), i);

  }

  fclose(fp);

}
#endif // DEBUG

