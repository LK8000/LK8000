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
#include <Point2D.h>

#ifdef LKAIRSPACE

using std::min;
using std::max;

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

// CAirspaceManager class attributes
CAirspaceManager CAirspaceManager::_instance = CAirspaceManager(CAirspaceManager::_instance);

// CAirspace class attributes
int CAirspace::_nearesthdistance = 0;            // for infobox
int CAirspace::_nearestvdistance = 0;            // for infobox
TCHAR* CAirspace::_nearestname = NULL;            // for infobox
bool CAirspace::_pos_in_flyzone = false;        // for refine warnings in flyzones
bool CAirspace::_pred_in_flyzone = false;        // for refine warnings in flyzones
bool CAirspace::_pos_in_acked_nonfly_zone = false;        // for refine warnings in flyzones
bool CAirspace::_pred_in_acked_nonfly_zone = false;        // for refine warnings in flyzones
int CAirspace::_now = 0;                        // gps time saved
int CAirspace::_hdistancemargin = 0;            // calculated horizontal distance margin to use
CPoint2D CAirspace::_lastknownpos(0,0);                // last known position saved for calculations
int CAirspace::_lastknownalt = 0;                // last known alt saved for calculations
int CAirspace::_lastknownagl = 0;                // last known agl saved for calculations


//
// CAIRSPACE CLASS
//

// Dumps object instance to Runtime.log
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

// Called when QNH changed
void CAirspace::QnhChangeNotify()
{
  if (_top.Base == abFL) _top.Altitude = AltitudeToQNHAltitude((_top.FL * 100)/TOFEET);
  if (_base.Base == abFL) _base.Altitude = AltitudeToQNHAltitude((_base.FL * 100)/TOFEET);
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

// returns true if the given altitude inside this airspace + alt extension
bool CAirspace::IsAltitudeInside(int alt, int agl, int extension) const
{
  return (
      ((((_base.Base != abAGL) && ( alt >= (_base.Altitude - extension)))
        || ((_base.Base == abAGL) && ( agl >= (_base.AGL - extension)))))
      && ((((_top.Base != abAGL) && (alt < (_top.Altitude + extension))))
        || ((_top.Base == abAGL) && (agl < (_top.AGL + extension))))
  );
}

// Step1:
// warning calculation, set initial states, etc.
void CAirspace::StartWarningCalculation(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  _pos_in_flyzone = false;
  _pred_in_flyzone = false;
  _pos_in_acked_nonfly_zone = false;
  _pred_in_acked_nonfly_zone = false;
  
  _nearestname = NULL; 
  _nearesthdistance=100000; 
  _nearestvdistance=100000;

  _now = Basic->Time;
  
  //Save position for further calculations made by gui threads
  if (Basic->BaroAltitudeAvailable) {
    _lastknownalt = (int)Basic->BaroAltitude;
  } else {
    _lastknownalt = (int)Basic->Altitude;
  }
  _lastknownagl = (int)Calculated->AltitudeAGL;
  if (_lastknownagl < 0) _lastknownagl = 0;            // Limit agl to zero
  CPoint2D position_now(Basic->Latitude, Basic->Longitude);
  _lastknownpos = position_now;

  // Horizontal distance margin
   _hdistancemargin = Basic->Speed * WarningTime;

}

// Step2: first pass on all airspace instances
// Calculate warnlevel based on last/now/next position
void CAirspace::CalculateWarning(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  _warnevent = aweNone;
  
  int alt;
  int agl;
  
  //Check actual position
  _pos_inside_now = false;
  if (Basic->BaroAltitudeAvailable) {
    alt = (int)Basic->BaroAltitude;
  } else {
    alt = (int)Basic->Altitude;
  }
  agl = (int)Calculated->AltitudeAGL;
  if (agl<0) agl = 0;        // Limit actual altitude to surface to not get warnings if close to ground

  // Calculate distances
  CalculateDistance(NULL,NULL,NULL);
  if ( _hdistance <= 0 ) {
    _pos_inside_now = true;
  }
  // Check for altitude
  bool pos_altitude = IsAltitudeInside(alt, agl);
  if (!pos_altitude) _pos_inside_now = false;

  // TODO Until we have one infobox, we have to collect nearest distance values differently!
  if (_flyzone && _pos_inside_now) {
    // If in flyzone, nearest warning point given (nearest distance to leaving the fly zone)
    if ( abs(_hdistance) < abs(_nearesthdistance) ) {
      _nearestname = _name;
      _nearesthdistance = abs(_hdistance);
      // not used now _nearestvdistance = _vdistance;
    }
  }
  if (!_flyzone) {
    if (_pos_inside_now) {
      // Inside a non fly zone, distance is zero
      _nearestname = _name;
      _nearesthdistance = 0;
      // not used now _nearestvdistance = 0;
    } else {
      // If outside nofly zone, then nearest distance selected
      if ( (abs(_hdistance) < abs(_nearesthdistance)) ) {
        _nearestname = _name;
        _nearesthdistance = abs(_hdistance);
        // not used now _nearestvdistance = _vdistance;
      }
    }
  }
  
  // We have to calculate with the predicted position
  bool pred_inside_now = false;
  alt = (int)Calculated->NextAltitude;
  agl = (int)Calculated->NextAltitudeAGL;
  if (agl<0) agl = 0;        // Limit predicted agl to surface
  // Check for altitude
  pos_altitude = IsAltitudeInside(alt, agl);
  if (pos_altitude) pred_inside_now = IsHorizontalInside(Calculated->NextLongitude, Calculated->NextLatitude);

  if (_flyzone) {
    // FLY-ZONE
    if (pred_inside_now) _pred_in_flyzone = true;
    if (_pos_inside_now) {
      _pos_in_flyzone = true;
      if (_pos_inside_last) {
        if (pred_inside_now) {
          // FLY-ZONE _pos_inside_last = true, _pos_inside_now = true, _pred_inside_now = true
          // moving inside -> normal, no warning event
          _warnevent = aweMovingInsideFly;
        } else {
          // FLY-ZONE _pos_inside_last = true, _pos_inside_now = true, _pred_inside_now = false
          // predicted leaving, yellow warning
          _warnevent = awePredictedLeavingFly;
        }
      } else {
        // FLY-ZONE _pos_inside_last = false, _pos_inside_now = true, _pred_inside_now = X
        // Entering, generate info msg
        _warnevent = aweEnteringFly;
      }
    } else {
      if (_pos_inside_last) {
        // FLY-ZONE _pos_inside_last = true, _pos_inside_now = false, _pred_inside_now = X
        // leaving, red warning
        _warnevent = aweLeavingFly;
      } else {
        if (pred_inside_now) {
          // FLY-ZONE _pos_inside_last = false, _pos_inside_now = false, _pred_inside_now = true
          // predicted enter
          _warnevent = awePredictedEnteringFly;
        } else {
          // FLY-ZONE _pos_inside_last = false, _pos_inside_now = false, _pred_inside_now = true
          // moving outside
          _warnevent = aweMovingOutsideFly;
        }
      }
    }
  } else {
    // Default NON-FLY ZONE
    if (pred_inside_now && (_warningacklevel > awNone)) _pred_in_acked_nonfly_zone = true;
    if (_pos_inside_now) {
        if (_warningacklevel > awNone) _pos_in_acked_nonfly_zone = true;
      if (_pos_inside_last) {
        //  NON-FLY ZONE _pos_inside_last = true, _pos_inside_now = true, _pred_inside_now = X
        // Moving indside or predicted leaving, nothing to do
        _warnevent = aweMovingInsideNonfly;
      } else {
        // NON-FLY ZONE _pos_inside_last = false, _pos_inside_now = true, _pred_inside_now = X
        // Entering, set warnlevel
        _warnevent = aweEnteringNonfly;
      }
    } else {
      if (_pos_inside_last) {
        // NON-FLY ZONE _pos_inside_last = true, _pos_inside_now = false, _pred_inside_now = X
        // leaving, or leaving and then predicted entry? -> nothing to do
        _warnevent = aweLeavingNonFly;
      } else {
        if (pred_inside_now) {
          // NON-FLY ZONE _pos_inside_last = false, _pos_inside_now = false, _pred_inside_now = true
          // predicted enter
          _warnevent = awePredictedEnteringNonfly;
        } else {
          // NON-FLY ZONE _pos_inside_last = false, _pos_inside_now = false, _pred_inside_now = false
          // moving outside
          _warnevent = aweMovingOutsideNonfly;
        }
      }
    }
  }//if else flyzone

  _pos_inside_last = _pos_inside_now;
}

// Step3: second pass on all airspace instances
// returns true if a warning message has to be printed
bool CAirspace::FinishWarning()
{
    bool res = false;
    int abs_hdistance = abs(_hdistance);
    int abs_vdistance = abs(_vdistance);
    int hdistance_histeresis = 500;           // Horizontal distance histeresis to step back awNone
    int vdistance_histeresis = 20;            // Vertical distance histeresis to step back awNone
    int hdistance_lookout = 200;              // Horizontal distance to lookout from a flyzone to check what is outside
    int vdistance_lookout = 20;               // Vertical distance to lookout from a flyzone to check what is outside
    
    //Calculate warning state based on airspace warning events
    switch (_warnevent) {
      default:
        break;

      // Events for FLY zones
      case aweMovingInsideFly:
        // If far away from border, set warnlevel to none
        // If base is sfc, we skip near warnings to base, to not get disturbing messages on landing.
        if ( (abs_hdistance>(_hdistancemargin+hdistance_histeresis)) && 
             (abs_vdistance>(AirspaceWarningVerticalMargin+vdistance_histeresis)) 
           ) {
          // Far away horizontally _and_ vertically
          _warninglevel = awNone;
          _hwarninglabel_hide = false;
          _vwarninglabel_hide = false;
          break;
        }
        _hwarninglabel_hide = true;
        if (abs_hdistance<_hdistancemargin) {
          // Check what is outside this flyzone. If another flyzone or acked nonfly zone, then we don't have to increase the warn state
          double lon = 0;
          double lat =0;
          double dist = fabs(_hdistance) + hdistance_lookout;
          FindLatitudeLongitude(_lastknownpos.Latitude(), _lastknownpos.Longitude(), _bearing, dist, &lat, &lon);
          
          if ( !CAirspaceManager::Instance().AirspaceWarningIsGoodPosition(lon, lat, _lastknownalt, _lastknownagl) ) {
            // Near to outside, modify warnevent to inform user
            _warninglevel = awYellow;
            _warnevent = aweNearOutsideFly;
            _hwarninglabel_hide = false;
          }
        }
        
        _vwarninglabel_hide = true;
        if (abs_vdistance<AirspaceWarningVerticalMargin) {
          // Check what is outside vertically this flyzone. If another flyzone or acked nonfly zone, then we don't have to increase the warn state
          int alt = _lastknownalt;
          int agl = _lastknownagl;
          if (_vdistance<0) {
            // adjacent airspace will be above this one 
            alt += abs_vdistance + vdistance_lookout;
            agl += abs_vdistance + vdistance_lookout;
          } else {
            // adjacent airspace will be below this one 
            alt -= abs_vdistance + vdistance_lookout;
            agl -= abs_vdistance + vdistance_lookout;
          }
          if (agl<0) agl = 0;
          
          if ( !CAirspaceManager::Instance().AirspaceWarningIsGoodPosition(_lastknownpos.Longitude(), _lastknownpos.Latitude(), alt, agl) ) {
            // Near to outside, modify warnevent to inform user
            _warninglevel = awYellow;
            _warnevent = aweNearOutsideFly;
            _vwarninglabel_hide = false;
          }
        }
        break;
        
      case awePredictedLeavingFly:
        if ( !(_pred_in_flyzone || _pred_in_acked_nonfly_zone) ) {
              // if predicted position not in other fly or acked nonfly zone, then leaving this one should be wrong
              _warninglevel = awYellow;
        }
        break;
        
      case aweLeavingFly:
        if ( !(_pos_in_flyzone || _pos_in_acked_nonfly_zone) ) {
              // if current position not in other fly or acked nonfly zone, then leaving this one should be wrong
              _warninglevel = awRed;
        }
        break;
        
      case awePredictedEnteringFly:
        break;
        
      case aweEnteringFly:
        // Also preset warnlevel to awYellow, because we entering yellow zone. 
        // but we don't need to generate a warning message right now - force no change in warnlevel
        _hwarninglabel_hide = true;
        if (abs_hdistance<_hdistancemargin) {
          // Check what is outside this flyzone. If another flyzone or acked nonfly zone, then we don't have to increase the warn state
          double lon = 0;
          double lat =0;
          double dist = fabs(_hdistance) + hdistance_lookout;
          FindLatitudeLongitude(_lastknownpos.Latitude(), _lastknownpos.Longitude(), _bearing, dist, &lat, &lon);
          
          if ( !CAirspaceManager::Instance().AirspaceWarningIsGoodPosition(lon, lat, _lastknownalt, _lastknownagl) ) {
            _warninglevelold = _warninglevel = awYellow;          
            _hwarninglabel_hide = false;
          }
        }
        
        _vwarninglabel_hide = true;
        if (abs_vdistance<AirspaceWarningVerticalMargin) {
          // Check what is outside vertically this flyzone. If another flyzone or acked nonfly zone, then we don't have to increase the warn state
          int alt = _lastknownalt;
          int agl = _lastknownagl;
          if (_vdistance<0) {
            // adjacent airspace will be above this one 
            alt += abs_vdistance + vdistance_lookout;
            agl += abs_vdistance + vdistance_lookout;
          } else {
            // adjacent airspace will be below this one 
            alt -= abs_vdistance + vdistance_lookout;
            agl -= abs_vdistance + vdistance_lookout;
          }
          if (agl<0) agl = 0;
          
          if ( !CAirspaceManager::Instance().AirspaceWarningIsGoodPosition(_lastknownpos.Longitude(), _lastknownpos.Latitude(), alt, agl) ) {
            _warninglevelold = _warninglevel = awYellow;          
            _vwarninglabel_hide = false;
          }
        }
        // Do info message on entering a fly zone
        res = true;
        break;
        
      case aweMovingOutsideFly:
        // if outside, but in good zone, then this one is good as well
        if ( (_pos_in_flyzone || _pos_in_acked_nonfly_zone) ) _warninglevel = awNone;
        break;

        
      // Events for NON-FLY zones
      case aweMovingOutsideNonfly:
        if ( (abs_hdistance > (_hdistancemargin + hdistance_histeresis)) ||
             (!IsAltitudeInside(_lastknownalt, _lastknownagl, AirspaceWarningVerticalMargin + vdistance_histeresis))
          ) {
            // Far away horizontally _or_ vertically
            _warninglevel = awNone;
        }
        if ( _hdistance < _hdistancemargin ) {
          if (IsAltitudeInside(_lastknownalt, _lastknownagl, AirspaceWarningVerticalMargin)) {
            // Near to inside, modify warnevent to inform user
            _warninglevel = awYellow;
            _warnevent = aweNearInsideNonfly;
          }
        }
        break;
        
      case awePredictedEnteringNonfly:
        _warninglevel = awYellow;
        break;
        
      case aweEnteringNonfly:
        _warninglevel = awRed;
        break;

      case aweMovingInsideNonfly:
        _warninglevel = awRed;
        break;
        
      case aweLeavingNonFly:
        // Do info message on leaving a nonfly zone
        res = true;
        break;
    }//sw warnevent
    _warneventold = _warnevent;

    // Warnstate increased above ack state -> generate message
    if ( (_warninglevel > _warninglevelold) && (_warninglevel > _warningacklevel) ) {
        _warn_repeat_time = _now + AirspaceWarningRepeatTime;
        res = true;
    }
    
    // Unacknowledged warning repeated after some time
    if ( (_warninglevel > _warningacklevel) && (_now > _warn_repeat_time) ) {
        _warn_repeat_time = _now + AirspaceWarningRepeatTime;
        res = true;
    }

    //ACK Step back, if ack time ellapsed and warningstate below ack state
    if ( (_warningacklevel>_warninglevel) && (_now > _warnacktimeout) ) _warningacklevel=_warninglevel;
    
    _warninglevelold = _warninglevel;

    return res;
}

// Set ack timeout to configured value
void CAirspace::SetAckTimeout()
{
  _warnacktimeout = _now + AcknowledgementTime;
}

// Gets calculated distances, returns true if distances valid
bool CAirspace::GetDistanceInfo(bool &inside, int &hDistance, int &Bearing, int &vDistance) const
{
  if (_distances_ready) {
    Bearing = _bearing;
    hDistance = _hdistance;
    vDistance = _vdistance;
    inside = _pos_inside_now;
    return true;
  }
  return false;
}

// Get warning point coordinates, returns true if distances valid
bool CAirspace::GetWarningPoint(double &longitude, double &latitude, AirspaceWarningDrawStyle_t &hdrawstyle, int &vDistance, AirspaceWarningDrawStyle_t &vdrawstyle) const
{
  if (_distances_ready && _enabled) {
    if (_flyzone && !_pos_inside_now ) return false;    // no warning labels if outside a flyzone

    vdrawstyle = awsBlack;
    if (IsAltitudeInside(_lastknownalt, _lastknownagl, 0)) {
          if (!_flyzone) vdrawstyle = awsRed;
    } else {
        if (_flyzone) vdrawstyle = awsAmber;
    }
    hdrawstyle = vdrawstyle;

    double dist = abs(_hdistance);
    FindLatitudeLongitude(_lastknownpos.Latitude(), _lastknownpos.Longitude(), _bearing, dist, &latitude, &longitude);

    vDistance = _vdistance;
    //if (abs(_vdistance) > AirspaceWarningVerticalMargin) vdrawstyle = awsHidden;

    // Nofly zones
    if (!_flyzone && (_hdistance<0)) hdrawstyle = awsHidden;       // No horizontal warning label if directly below or above
    if (!_flyzone && (_hdistance>0)) vdrawstyle = awsHidden;       // No vertical warning label if outside horizontally

    //In flyzones if adjacent flyzone exists, we do not display labels
    if (_hwarninglabel_hide) hdrawstyle = awsHidden;
    if (_vwarninglabel_hide) vdrawstyle = awsHidden;

    return true;
  }
  return false;
}

// Calculates nearest horizontal and vertical distance to airspace based on last known position
// Returns true if inside, false if outside
bool CAirspace::CalculateDistance(int *hDistance, int *Bearing, int *vDistance)
{
  bool inside = true;
  int vDistanceBase;
  int vDistanceTop;
  double fbearing;
  double distance;

  distance = Range(_lastknownpos.Longitude(), _lastknownpos.Latitude(), fbearing);
  if (distance > 0) inside = false;
  if (_base.Base != abAGL) {
      vDistanceBase = _lastknownalt - (int)(_base.Altitude);
  } else {
      vDistanceBase = _lastknownagl - (int)(_base.AGL);
  }
  if (_top.Base != abAGL) {
      vDistanceTop  = _lastknownalt - (int)(_top.Altitude);
  } else {
      vDistanceTop  = _lastknownagl - (int)(_top.AGL);
  }

  if (vDistanceBase < 0 || vDistanceTop > 0) inside = false;

  _bearing = (int)fbearing;
  _hdistance = (int)distance;
  if ((-vDistanceBase > vDistanceTop) && (_base.Base != abAGL))
    _vdistance = vDistanceBase;
  else
    _vdistance = vDistanceTop;
  
  if (Bearing) *Bearing = _bearing;
  if (hDistance) *hDistance = _hdistance;
  if (vDistance) *vDistance = _vdistance;
  _distances_ready = true;
  return inside;
}

// Reset warnings, if airspace outside calculation scope
void CAirspace::ResetWarnings()
{
  _warninglevel = awNone;
  _distances_ready = false;
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

// Dumps object instance to Runtime.log
void CAirspace_Circle::Dump() const
{
  StartupStore(TEXT("CAirspace_Circle Dump, CenterLat:%lf, CenterLon:%lf, Radius:%lf%s"), _latcenter, _loncenter, _radius, NEWLINE);
  CAirspace::Dump();
}

// Check if the given coordinate is inside the airspace
bool CAirspace_Circle::IsHorizontalInside(const double &longitude, const double &latitude) const
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

// Calculate horizontal distance from a given point
double CAirspace_Circle::Range(const double &longitude, const double &latitude, double &bearing) const
{
  double distance;
  DistanceBearing(latitude,longitude,
                  _latcenter, 
                  _loncenter,
                  &distance, &bearing);
  distance -= _radius;
  if (distance<0) bearing += 180;
  if (bearing>360) bearing -= 360;
  return distance;
}

// Helper function to calculate circle bounds
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

// Calculate airspace bounds
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

// Calculate screen coordinates for drawing
void CAirspace_Circle::CalculateScreenPosition(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const double &ResMapScaleOverDistanceModify) 
{
  _drawstyle = adsHidden;
  if (!_enabled) return;
  
  if (iAirspaceMode[_type]%2 == 1) {
    if(CAirspaceManager::Instance().CheckAirspaceAltitude(_base, _top)) {
      if (msRectOverlap(&_bounds, &screenbounds_latlon) 
         // || msRectContained(&screenbounds_latlon, &_bounds) is redundant here, msRectOverlap also returns true on containing!
         ) {

    if ((!(iAirspaceBrush[_type] == NUMAIRSPACEBRUSHES-1)) && ((_warninglevel == awNone) || (_warninglevel > _warningacklevel))) {
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

// Draw airspace
void CAirspace_Circle::Draw(HDC hDCTemp, const RECT &rc, bool param1) const
{
  Circle(hDCTemp, _screencenter.x, _screencenter.y, _screenradius ,rc, true, param1);
}


//
// CAIRSPACE AREA CLASS
//
// Dumps object instance to Runtime.log
void CAirspace_Area::Dump() const
{
  CPoint2DArray::const_iterator i;

  StartupStore(TEXT("CAirspace_Area Dump%s"), NEWLINE);
  CAirspace::Dump();
  for (i = _geopoints.begin(); i != _geopoints.end(); ++i) {
    StartupStore(TEXT("  Point lat:%lf, lon:%lf%s"), i->Latitude(), i->Longitude(), NEWLINE);
  }
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
inline static float
isLeft( const CPoint2D &P0, const CPoint2D &P1, const double &longitude, const double &latitude )
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
  CPoint2DArray::const_iterator it = _geopoints.begin();
  CPoint2DArray::const_iterator itnext = it;
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


// Check if the given coordinate is inside the airspace
bool CAirspace_Area::IsHorizontalInside(const double &longitude, const double &latitude) const
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

// Calculate horizontal distance from a given point
double CAirspace_Area::Range(const double &longitude, const double &latitude, double &bearing) const
{
  // find nearest distance to line segment
  unsigned int i;
  unsigned int dist = 0;
  unsigned int dist_candidate = 0;
  CPoint2D p3(latitude, longitude);
  int x=0,y=0,z=0;
  int xc=0, yc=0, zc=0;

  int    wn = 0;    // the winding number counter
  
  CPoint2DArray::const_iterator it = _geopoints.begin();
  CPoint2DArray::const_iterator itnext = it;
  ++itnext;
  
  for (i=0; i<_geopoints.size()-1; ++i) {
    dist = p3.DistanceXYZ(*it, *itnext, &x, &y, &z);

    if (it->Latitude() <= latitude) {         // start y <= P.Latitude
        if (itnext->Latitude() > latitude)      // an upward crossing
            if (isLeft( *it, *itnext, longitude, latitude) > 0)  // P left of edge
                ++wn;            // have a valid up intersect
    } else {                       // start y > P.Latitude (no test needed)
        if (itnext->Latitude() <= latitude)     // a downward crossing
            if (isLeft( *it, *itnext, longitude, latitude) < 0)  // P right of edge
                --wn;            // have a valid down intersect
    }

    if ((dist<dist_candidate)||(i==0)) {
      dist_candidate = dist;
      xc = x;
      yc = y;
      zc = z;
    }
    ++it;
    ++itnext;
  }

  CPoint2D p4(xc,yc,zc);
  double nearestdistance;
  double nearestbearing;

  DistanceBearing(latitude, longitude, p4.Latitude(), p4.Longitude(), &nearestdistance, &nearestbearing);
  
  bearing = nearestbearing;
  if (wn!=0) return -nearestdistance; else return nearestdistance;
}

// Set polygon point list
void CAirspace_Area::SetPoints(CPoint2DArray &Area_Points)
{
    POINT p;
    _geopoints = Area_Points;
    _screenpoints.clear();
    for (unsigned int i=0; i<_geopoints.size(); ++i) _screenpoints.push_back(p);
    CalcBounds();
    AirspaceAGLLookup( (_bounds.miny+_bounds.maxy)/2.0, (_bounds.minx+_bounds.maxx)/2.0 ); 
}

// Calculate airspace bounds
void CAirspace_Area::CalcBounds()
{
  CPoint2DArray::iterator it = _geopoints.begin();
  
  _bounds.minx = it->Longitude();
  _bounds.maxx = it->Longitude();
  _bounds.miny = it->Latitude();
  _bounds.maxy = it->Latitude();
  for(it = _geopoints.begin(); it != _geopoints.end(); ++it) {
    _bounds.minx = min((double)it->Longitude(), _bounds.minx);
    _bounds.maxx = max((double)it->Longitude(), _bounds.maxx);
    _bounds.miny = min((double)it->Latitude(), _bounds.miny);
    _bounds.maxy = max((double)it->Latitude(), _bounds.maxy);
  }

  // JMW detect airspace that wraps across 180
  if ((_bounds.minx< -90) && (_bounds.maxx>90)) {
    double tmp = _bounds.minx;
    _bounds.minx = _bounds.maxx;
    _bounds.maxx = tmp;
    for(it = _geopoints.begin(); it != _geopoints.end(); ++it) {
      if (it->Longitude()<0) {
        CPoint2D newpoint(it->Latitude(), it->Longitude() + 360);
        *it = newpoint;
      }
    }
  }
}

// Calculate screen coordinates for drawing
void CAirspace_Area::CalculateScreenPosition(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const double &ResMapScaleOverDistanceModify) 
{
  _drawstyle = adsHidden;
  if (!_enabled) return;
  
  if (iAirspaceMode[_type]%2 == 1) {
    if(CAirspaceManager::Instance().CheckAirspaceAltitude(_base, _top)) {
      if (msRectOverlap(&_bounds, &screenbounds_latlon) 
         // || msRectContained(&screenbounds_latlon, &_bounds) is redundant here, msRectOverlap also returns true on containing!
         ) {

    if ((!(iAirspaceBrush[_type] == NUMAIRSPACEBRUSHES-1)) && ((_warninglevel == awNone) || (_warninglevel > _warningacklevel))) {
      _drawstyle = adsFilled;
    } else {
      _drawstyle = adsOutline;
    }
    CPoint2DArray::iterator it;
    POINTList::iterator itr;
    for (it = _geopoints.begin(), itr = _screenpoints.begin(); it != _geopoints.end(); ++it, ++itr) {
        MapWindow::LatLon2Screen(it->Longitude(), it->Latitude(), *itr);
    }
      }
    }
  }
}

// Draw airspace
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

bool CAirspaceManager::CheckAirspaceAltitude(const AIRSPACE_ALT &Base, const AIRSPACE_ALT &Top) const
{
  
  double alt;
  double basealt;
  double topalt;
  bool base_is_sfc = false;
  
  if (GPS_INFO.BaroAltitudeAvailable) {
    alt = GPS_INFO.BaroAltitude;
  } else {
    alt = GPS_INFO.Altitude;
  }

    if (Base.Base != abAGL) {
      basealt = Base.Altitude;
    } else {
      basealt = Base.AGL + CALCULATED_INFO.TerrainAlt;
      if (Base.AGL <= 0) base_is_sfc = true;
    }
    if (Top.Base != abAGL) {
      topalt = Top.Altitude;
    } else {
      topalt = Top.AGL + CALCULATED_INFO.TerrainAlt;
    }

  switch (AltitudeMode)
    {
    case ALLON : return TRUE;
        
    case CLIP : 
      if ((basealt < ClipAltitude) || base_is_sfc) return TRUE; else return FALSE;

    case AUTO:
      if( (( alt > (basealt - AltWarningMargin)) || base_is_sfc )
      && ( alt < (topalt + AltWarningMargin) ))
    return TRUE;
      else
    return FALSE;

    case ALLBELOW:
      if(  ((basealt - AltWarningMargin) < alt ) || base_is_sfc )
    return  TRUE;
      else
    return FALSE;
    case INSIDE:
      if( (( alt >= basealt ) || base_is_sfc ) && ( alt < topalt ) )
    return TRUE;
      else
        return FALSE;
    case ALLOFF : return FALSE;
    }
  return TRUE;
}

void CAirspaceManager::ReadAltitude(const TCHAR *Text, AIRSPACE_ALT *Alt) const
{
  TCHAR *Stop = NULL;
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
  TCHAR *Stop = Text;

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


bool CAirspaceManager::CalculateArc(TCHAR *Text, CPoint2DArray *_geopoints, double &CenterX, const double &CenterY, const int &Rotation) const
{
  double StartLat, StartLon;
  double EndLat, EndLon;
  double StartBearing;
  double EndBearing;
  double Radius;
  TCHAR *Comma = NULL;
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
  _geopoints->push_back(CPoint2D(StartLat, StartLon));
  
  while(fabs(EndBearing-StartBearing) > 7.5)
  {
      StartBearing += Rotation *5 ;
      if(StartBearing > 360) StartBearing -= 360;
      if(StartBearing < 0) StartBearing += 360;
      FindLatitudeLongitude(CenterY, CenterX, StartBearing, Radius, &lat, &lon);
      _geopoints->push_back(CPoint2D(lat,lon));
  }
  _geopoints->push_back(CPoint2D(EndLat, EndLon));
  return true;
}

bool CAirspaceManager::CalculateSector(TCHAR *Text, CPoint2DArray *_geopoints, double &CenterX, const double &CenterY, const int &Rotation) const
{
  double Radius;
  double StartBearing;
  double EndBearing;
  TCHAR *Stop = NULL;
  double lat=0,lon=0;
  
  // TODO 110307 FIX problem of StrToDouble returning 0.0 in case of error , and not setting Stop!!
  Radius = NAUTICALMILESTOMETRES * (double)StrToDouble(Text, &Stop);
  StartBearing = (double)StrToDouble(&Stop[1], &Stop);
  EndBearing = (double)StrToDouble(&Stop[1], &Stop);

  while(fabs(EndBearing-StartBearing) > 7.5)
  {
    if(StartBearing >= 360) StartBearing -= 360;
    if(StartBearing < 0) StartBearing += 360;

    FindLatitudeLongitude(CenterY, CenterX, StartBearing, Radius, &lat, &lon);

    _geopoints->push_back(CPoint2D(lat,lon));
    
    StartBearing += Rotation *5 ;
  }
  FindLatitudeLongitude(CenterY, CenterX, EndBearing, Radius, &lat, &lon);
  _geopoints->push_back(CPoint2D(lat,lon));
  return true;
}

// Correcting geopointlist
// All algorithms require non self-intersecting and closed polygons.
// Also the geopointlist last element have to be the same as first -> openair doesn't require this, we have to do it here
void CAirspaceManager::CorrectGeoPoints(CPoint2DArray &points)
{
    if (points.size()==0) return;
    CPoint2D first = points.front();
    CPoint2D last = points.back();
    
    if ( (first.Latitude() != last.Latitude()) || (first.Longitude() != last.Longitude()) ) points.push_back(first);
}

// Reading and parsing OpenAir airspace file
void CAirspaceManager::FillAirspacesFromOpenAir(ZZIP_FILE *fp)
{
  TCHAR    *Comment;
  int        nSize;
  TCHAR Text[READLINE_LENGTH];
  TCHAR sTmp[READLINE_LENGTH+100];
  TCHAR *p;
  int linecount=0;
  int parsing_state = 0;
  CAirspace *newairspace = NULL;
  // Variables to store airspace parameters
  TCHAR Name[NAME_SIZE+1];
  CPoint2DArray points;
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
  bool flyzone = false;
  
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
        *Comment = _T('\0');        // Truncate line
        nSize = Comment - p;        // Reset size
        if (nSize < 3)
          continue;                // Ensure newline removal won't fail
    }
    nSize = _tcsclen(p);
    if(p[nSize-1] == _T('\n')) p[--nSize] = _T('\0');
    if(p[nSize-1] == _T('\r')) p[--nSize] = _T('\0');

//    StartupStore(TEXT(".  %s%s"),p,NEWLINE);

    switch (*p) {
      case _T('A'):
        p++; // skip A
        switch (*p) {
          case _T('C'):    //AC
            p++; // skip C
            if (parsing_state==10) { // New airspace begin, store the old one, reset parser
              newairspace = NULL;
              if (Radius>0) {
                // Last one was a circle
                newairspace = new CAirspace_Circle(Longitude, Latitude, Radius);
              } else {
                  // Last one was an area
                  CorrectGeoPoints(points);
                  // Skip it if we dont have minimum 3 points
                  if (points.size()>3) {
                    newairspace = new CAirspace_Area;
                    newairspace->SetPoints(points);
                  }
              }
              if (newairspace!=NULL) {
                newairspace->Init(Name, Type, Base, Top, flyzone);

                if (1) {
                  CCriticalSection::CGuard guard(_csairspaces);
                  _airspaces.push_back(newairspace);
                }
              }
              
              Name[0]='\0';
              Radius = 0;
              Longitude = 0;
              Latitude = 0;
              points.clear();
              Type = 0;
              Base.Base = abUndef;
              Top.Base = abUndef;
              flyzone = false;
              newairspace = NULL;
              parsing_state = 0;
            }
            // New AC
            p++; //Skip space
            Type = OTHER;
            for (int i = 0; i < k_nAreaCount; ++i) {
              if (StartsWith(p, k_strAreaStart[i])) {
                  Type = k_nAreaType[i];
                  break;
              }
            }
            Rotation = +1;
            parsing_state = 10;
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
            
          //OpenAir non standard field - AF - define a fly zone
          case _T('F'): // AF - Fly zone, no parameter
              flyzone = true;
            continue;

          case _T('T'): // AT
            // ignore airspace labels
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
              points.push_back(CPoint2D(lat,lon));
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
        CorrectGeoPoints(points);
        // Skip it if we dont have minimum 3 points
        if (points.size()<3) {
        }
        newairspace = new CAirspace_Area();
        newairspace->SetPoints(points);
    }
    newairspace->Init(Name, Type, Base, Top, flyzone);
    CCriticalSection::CGuard guard(_csairspaces);
    _airspaces.push_back(newairspace);
  }

  CCriticalSection::CGuard guard(_csairspaces);
  StartupStore(TEXT(". Readed %d airspaces%s"), _airspaces.size(), NEWLINE);
  // For debugging, dump all readed airspaces to runtime.log
  //CAirspaceList::iterator it;
  //for ( it = _airspaces.begin(); it != _airspaces.end(); ++it) (*it)->Dump();
}


void CAirspaceManager::ReadAirspaces()
{
  TCHAR    szFile1[MAX_PATH] = TEXT("\0");
  TCHAR    szFile2[MAX_PATH] = TEXT("\0");

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
    fp  = zzip_fopen(szFile1, "rt");
  } else {
    //* 091206 back on 
    static TCHAR  szMapFile[MAX_PATH] = TEXT("\0");
    GetRegistryString(szRegistryMapFile, szMapFile, MAX_PATH);
    ExpandLocalPath(szMapFile);
    wcscat(szMapFile,TEXT("/"));
    wcscat(szMapFile,TEXT(LKF_AIRSPACES)); // 091206
    fp  = zzip_fopen(szMapFile, "rt");
    //*/
  }

  if (_tcslen(szFile2)>0) {
    fp2 = zzip_fopen(szFile2, "rt");
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
  
  CCriticalSection::CGuard guard(_csairspaces);
  _user_warning_queue.clear();
  _airspaces_near.clear();
  _airspaces_of_interest.clear();
  for ( it = _airspaces.begin(); it != _airspaces.end(); ++it) delete *it;
  _airspaces.clear();
  StartupStore(TEXT(". CloseLKAirspace%s"),NEWLINE);
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
            inside = (*it)->IsHorizontalInside(lons[i], lats[i]);
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

CAirspace* CAirspaceManager::FindNearestAirspace(const double &longitude, const double &latitude,
             double *nearestdistance, double *nearestbearing, double *height) const
{
  double nearestd = 100000; // 100km
  double nearestb = 0;

  bool iswarn;
  bool isdisplay;
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
    
    if (height) {
      double basealt;
      double topalt;
      bool base_is_sfc = false;

      if ((*it)->Base()->Base != abAGL) {
        basealt = (*it)->Base()->Altitude;
      } else {
        basealt = (*it)->Base()->AGL + CALCULATED_INFO.TerrainAlt;
        if ((*it)->Base()->AGL <= 0) base_is_sfc = true;
      }
      if ((*it)->Top()->Base != abAGL) {
        topalt = (*it)->Top()->Altitude;
      } else {
        topalt = (*it)->Top()->AGL + CALCULATED_INFO.TerrainAlt;
      }
      altok = (((*height > basealt) || base_is_sfc) && (*height < topalt));
    } else {
      altok = CheckAirspaceAltitude(*(*it)->Base(), *(*it)->Top())==TRUE;
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

bool airspace_sorter(CAirspace *a, CAirspace *b)
{
    return (a->Top()->Altitude < b->Top()->Altitude);
}

void CAirspaceManager::SortAirspaces(void)
{
  StartupStore(TEXT(". SortAirspace%s"),NEWLINE);

  // Sort by top altitude for drawing
  CCriticalSection::CGuard guard(_csairspaces);
  std::sort(_airspaces.begin(), _airspaces.end(), airspace_sorter );
}

bool CAirspaceManager::ValidAirspaces(void) const
{
  CCriticalSection::CGuard guard(_csairspaces);
  bool res = _airspaces.size()>0;
  return res;
}


// Special function call for calculating warnings on flyzones. Called from CAirspace, mutex already locked!
bool CAirspaceManager::AirspaceWarningIsGoodPosition(float longitude, float latitude, int alt, int agl) const
{
    if (agl<0) agl = 0;        // Limit alt to surface
    for (CAirspaceList::const_iterator it=_airspaces_of_interest.begin(); it!=_airspaces_of_interest.end(); ++it) {
      if ( (!(*it)->Flyzone()) && ((*it)->WarningAckLevel() == awNone) ) continue;
      // Check for altitude
      if ((*it)->IsAltitudeInside(alt, agl)) {
        if ((*it)->IsHorizontalInside(longitude, latitude)) return true;
      }
    }
    return false;
}

void CAirspaceManager::AirspaceWarning(NMEA_INFO *Basic, DERIVED_INFO *Calculated) 
{
  static int step = 0;
  static double bearing = 0;
  static double interest_radius = 0;
  static rectObj bounds = {0};
  static double lon = 0;
  static double lat = 0;
 
  if(!AIRSPACEWARNINGS) {                       //Airspace warnings disabled in config
    CCriticalSection::CGuard guard(_csairspaces);
    // Set CAirspace class attributes to to range calculations well from UI
    CAirspace::StartWarningCalculation( Basic, Calculated );
    // No infobox values if warnings disabled, because no range calculation done.
    NearestAirspaceName[0]=0;
    NearestAirspaceHDist=0;
    NearestAirspaceVDist=0;
    return;
  }
  
  CCriticalSection::CGuard guard(_csairspaces);
  CAirspaceList::iterator it;

  if ( _airspaces_near.size() == 0 ) return;
  
  #ifdef DEBUG_AIRSPACE
  int starttick = GetTickCount();
  StartupStore(TEXT("---AirspaceWarning start%s"),NEWLINE);
  #endif

  
  switch (step) {
    default:
    case 0:
        // MULTICALC STEP 1
        // Calculate area of interest
        interest_radius = Basic->Speed * WarningTime * 1.25;        // 25% more than required
        lon = Basic->Longitude;
        lat = Basic->Latitude;
        bounds.minx = lon;
        bounds.maxx = lon;
        bounds.miny = lat;
        bounds.maxy = lat;

        bearing = 0;
        {
          FindLatitudeLongitude(Basic->Latitude, Basic->Longitude, bearing, interest_radius, &lat, &lon);
          bounds.minx = min(lon, bounds.minx);
          bounds.maxx = max(lon, bounds.maxx);
          bounds.miny = min(lat, bounds.miny);
          bounds.maxy = max(lat, bounds.maxy);
        }
        bearing = 90;
        {
          FindLatitudeLongitude(Basic->Latitude, Basic->Longitude, bearing, interest_radius, &lat, &lon);
          bounds.minx = min(lon, bounds.minx);
          bounds.maxx = max(lon, bounds.maxx);
          bounds.miny = min(lat, bounds.miny);
          bounds.maxy = max(lat, bounds.maxy);
        }
        bearing = 180;
        {
          FindLatitudeLongitude(Basic->Latitude, Basic->Longitude, bearing, interest_radius, &lat, &lon);
          bounds.minx = min(lon, bounds.minx);
          bounds.maxx = max(lon, bounds.maxx);
          bounds.miny = min(lat, bounds.miny);
          bounds.maxy = max(lat, bounds.maxy);
        }
        bearing = 270;
        {
          FindLatitudeLongitude(Basic->Latitude, Basic->Longitude, bearing, interest_radius, &lat, &lon);
          bounds.minx = min(lon, bounds.minx);
          bounds.maxx = max(lon, bounds.maxx);
          bounds.miny = min(lat, bounds.miny);
          bounds.maxy = max(lat, bounds.maxy);
        }

        // JMW detect airspace that wraps across 180
        if ((bounds.minx< -90) && (bounds.maxx>90)) {
          double tmp = bounds.minx;
          bounds.minx = bounds.maxx;
          bounds.maxx = tmp;
        }
        
        // Step1 Init calculations
        CAirspace::StartWarningCalculation( Basic, Calculated );
        ++step;
        break;
        
    case 1:
        // MULTICALC STEP 2
        // Step2 select airspaces in range, and do warning calculations on it, add to interest list
        _airspaces_of_interest.clear();
        for (it=_airspaces_near.begin(); it != _airspaces_near.end(); ++it) {
            // Check for warnings enabled for this class
            if (MapWindow::iAirspaceMode[(*it)->Type()] < 2) {
              (*it)->ResetWarnings();
              continue;
            }
            // Check if in interest area
            if (!msRectOverlap(&bounds, &(*it)->Bounds())) {
              (*it)->ResetWarnings();
              continue;
            }
            
            (*it)->CalculateWarning( Basic, Calculated );
            _airspaces_of_interest.push_back(*it);
        }
        ++step;
        break;
        
    case 2:
        // MULTICALC STEP 3
        // Step3 Run warning fsms, refine warnings in fly zones, collect user messages
        bool there_is_msg;
        for (it=_airspaces_of_interest.begin(); it != _airspaces_of_interest.end(); ++it) {
            there_is_msg = (*it)->FinishWarning();
            if (there_is_msg) {
              // Add new warning message to queue
              AirspaceWarningMessage msg;
              msg.originator = *it;
              msg.event = (*it)->WarningEvent();
              msg.warnlevel = (*it)->WarningLevel();
              _user_warning_queue.push_back(msg);
            }
        }

        // This is used nowhere.
        Calculated->IsInAirspace = false;

        // Fill infoboxes
        // TODO Until we have one infobox, we have to collect nearest distance values differently!
        if (CAirspace::GetNearestName() != NULL) {
          _tcsncpy(NearestAirspaceName, CAirspace::GetNearestName(), NAME_SIZE);
          NearestAirspaceName[NAME_SIZE]=0;
          NearestAirspaceHDist = CAirspace::GetNearestHDistance();
          NearestAirspaceVDist = CAirspace::GetNearestVDistance();
        } else {
          NearestAirspaceName[0]=0;
          NearestAirspaceHDist=0;
          NearestAirspaceVDist=0;
        }
        step = 0;
        break;
        
  } // sw step

  #ifdef DEBUG_AIRSPACE
  StartupStore(TEXT("   step %d takes %dms, processed %d airspaces from %d%s"), step, GetTickCount()-starttick, _airspaces_of_interest.size(), _airspaces_near.size(), NEWLINE);
  #endif
}


CAirspaceList CAirspaceManager::GetVisibleAirspacesAtPoint(const double &lon, const double &lat) const
{
  CAirspaceList res;
  CAirspaceList::const_iterator it;
  CCriticalSection::CGuard guard(_csairspaces);
  for (it = _airspaces.begin(); it != _airspaces.end(); ++it) {
    if ((*it)->DrawStyle()) {
      if ((*it)->IsHorizontalInside(lon, lat)) res.push_back(*it);
    }
  }
  return res;
}

void CAirspaceManager::SetFarVisible(const rectObj &bounds_active) 
{
  CAirspaceList::iterator it;

  CCriticalSection::CGuard guard(_csairspaces);
  _airspaces_near.clear();
  for (it = _airspaces.begin(); it != _airspaces.end(); ++it) {
    // Check if airspace overlaps given bounds
    if ( (msRectOverlap(&bounds_active, &((*it)->Bounds())) == MS_TRUE)
        && (*it)->Enabled()
       ) _airspaces_near.push_back(*it);
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


const CAirspaceList& CAirspaceManager::GetNearAirspacesRef() const
{
  return _airspaces_near;
}

const CAirspaceList CAirspaceManager::GetAllAirspaces() const
{
  CCriticalSection::CGuard guard(_csairspaces);
  return _airspaces;
}

// Comparer to sort airspaces based on label priority for drawing labels
bool airspace_label_priority_sorter( CAirspace *a, CAirspace *b )
{
  return a->LabelPriority() > b->LabelPriority();
}

// Get airspaces list for label drawing
const CAirspaceList CAirspaceManager::GetAirspacesForWarningLabels()
{
  CCriticalSection::CGuard guard(_csairspaces);
  if (_airspaces_of_interest.size()>1) std::sort(_airspaces_of_interest.begin(), _airspaces_of_interest.end(), airspace_label_priority_sorter);
  return _airspaces_of_interest;
}

// Feedback from mapwindow DrawAirspaceLabels to set a round-robin priority
void CAirspaceManager::AirspaceWarningLabelPrinted(CAirspace &airspace, bool success)
{
  CCriticalSection::CGuard guard(_csairspaces);
  if (success) airspace.LabelPriorityZero(); else airspace.LabelPriorityInc();
}

// Gets a list of airspaces which has a warning or an ack level different than awNone
CAirspaceList CAirspaceManager::GetAirspacesInWarning() const
{
  CAirspaceList res;
  CCriticalSection::CGuard guard(_csairspaces);
  for (CAirspaceList::const_iterator it = _airspaces_near.begin(); it != _airspaces_near.end(); ++it) {
    if ( (*it)->WarningLevel()>awNone || (*it)->WarningAckLevel()>awNone ) res.push_back(*it);
  }
  return res;
}

// Gets an airspace object instance copy for a given airspace
// to display instance attributes 
// NOTE: virtual methods don't work on copied instances!
//       they have to be mapped through airspacemanager class because of the mutex
CAirspace CAirspaceManager::GetAirspaceCopy(const CAirspace* airspace) const
{
  CCriticalSection::CGuard guard(_csairspaces);
  return *airspace;
}

// Calculate distances from a given airspace
bool CAirspaceManager::AirspaceCalculateDistance(CAirspace *airspace, int *hDistance, int *Bearing, int *vDistance)
{
  CCriticalSection::CGuard guard(_csairspaces);
  return airspace->CalculateDistance(hDistance, Bearing, vDistance);
}


bool warning_queue_sorter(AirspaceWarningMessage a, AirspaceWarningMessage b)
{
    return (a.warnlevel > b.warnlevel);
}


// Gets an airspace warning message to show
bool CAirspaceManager::PopWarningMessage(AirspaceWarningMessage *msg)
{
/*  CAirspace *res = NULL;
  CCriticalSection::CGuard guard(_csairspaces);
  if (_user_warning_queue.size() == 0) return NULL;
  res = _user_warning_queue.front();
  _user_warning_queue.pop_front();            // remove message from fifo
  return res;*/

  if (msg == NULL) return false;
  CCriticalSection::CGuard guard(_csairspaces);
  int size;

  //Sort warning messages
  size = _user_warning_queue.size();
  if (size == 0) return false;
  if (size>1) std::sort(_user_warning_queue.begin(), _user_warning_queue.end(), warning_queue_sorter);

  *msg = _user_warning_queue.front();
  _user_warning_queue.pop_front();            // remove message from fifo
  return true;
}

// Ack an airspace for a given ack level and acknowledgement time
void CAirspaceManager::AirspaceSetAckLevel(CAirspace &airspace, AirspaceWarningLevel_t ackstate)
{
    CCriticalSection::CGuard guard(_csairspaces);
    airspace.WarningAckLevel(ackstate);
    airspace.SetAckTimeout();
    #ifdef DEBUG_AIRSPACE
    StartupStore(TEXT("LKAIRSP: %s AirspaceWarnListAckForTime()%s"),airspace.Name(),NEWLINE );
    #endif
}

// Ack an airspace for a current level
void CAirspaceManager::AirspaceAckWarn(CAirspace &airspace)
{
    CCriticalSection::CGuard guard(_csairspaces);
    airspace.WarningAckLevel(airspace.WarningLevel());
    airspace.SetAckTimeout();
    #ifdef DEBUG_AIRSPACE
    StartupStore(TEXT("LKAIRSP: %s AirspaceWarnListAck()%s"),airspace.Name(),NEWLINE );
    #endif
}

// Ack an airspace for all future warnings
void CAirspaceManager::AirspaceAckSpace(CAirspace &airspace)
{
    CCriticalSection::CGuard guard(_csairspaces);
    airspace.WarningAckLevel(awRed);
    airspace.SetAckTimeout();
    #ifdef DEBUG_AIRSPACE
    StartupStore(TEXT("LKAIRSP: %s AirspaceAckSpace()%s"),airspace.Name(),NEWLINE );
    #endif
}

// Disable an airspace 
void CAirspaceManager::AirspaceDisable(CAirspace &airspace)
{
    CCriticalSection::CGuard guard(_csairspaces);
    airspace.Enabled(false);
    #ifdef DEBUG_AIRSPACE
    StartupStore(TEXT("LKAIRSP: %s AirspaceDisable()%s"),airspace.Name(),NEWLINE );
    #endif
}

// Enable an airspace
void CAirspaceManager::AirspaceEnable(CAirspace &airspace)
{
    CCriticalSection::CGuard guard(_csairspaces);
    airspace.Enabled(true);
    #ifdef DEBUG_AIRSPACE
    StartupStore(TEXT("LKAIRSP: %s AirspaceEnable()%s"),airspace.Name(),NEWLINE );
    #endif
}

// Toggle flyzone on an airspace
void CAirspaceManager::AirspaceFlyzoneToggle(CAirspace &airspace)
{
    CCriticalSection::CGuard guard(_csairspaces);
    airspace.FlyzoneToggle();
}

// Centralized function to get airspace type texts
TCHAR* CAirspaceManager::GetAirspaceTypeText(int type) const
{
    switch (type) {
      case RESTRICT:
        // LKTOKEN  _@M565_ = "Restricted" 
        return gettext(TEXT("_@M565_"));
      case PROHIBITED:
        // LKTOKEN  _@M537_ = "Prohibited" 
        return gettext(TEXT("_@M537_"));
      case DANGER:
        // LKTOKEN  _@M213_ = "Danger Area" 
        return gettext(TEXT("_@M213_"));
      case CLASSA:
        return TEXT("Class A");
      case CLASSB:
        return TEXT("Class B");
      case CLASSC:
        return TEXT("Class C");
      case CLASSD:
        return TEXT("Class D");
      case CLASSE:
        return TEXT("Class E");
      case CLASSF:
        return TEXT("Class F");
      case CLASSG:
        return TEXT("Class G");
      case NOGLIDER:
        // LKTOKEN  _@M464_ = "No Glider" 
        return gettext(TEXT("_@M464_"));
      case CTR:
        return TEXT("CTR");
      case WAVE:
        // LKTOKEN  _@M794_ = "Wave" 
        return gettext(TEXT("_@M794_"));
      case AATASK:
        return TEXT("AAT");
      case OTHER:
        // LKTOKEN  _@M765_ = "Unknown" 
        return gettext(TEXT("_@M765_"));
      default:
        return TEXT("");
    }
}

// Centralized function to get airspace type texts in short form
TCHAR* CAirspaceManager::GetAirspaceTypeShortText(int type) const
{
  switch (type) {
    case RESTRICT:
      return TEXT("Res");
    case PROHIBITED:
      return TEXT("Prb");
    case DANGER:
      return TEXT("Dgr");
    case CLASSA:
      return TEXT("A");
    case CLASSB:
      return TEXT("B");
    case CLASSC:
      return TEXT("C");
    case CLASSD:
      return TEXT("D");
    case CLASSE:
      return TEXT("E");
    case CLASSF:
      return TEXT("F");
    case CLASSG:
      return TEXT("G");
    case NOGLIDER:
      return TEXT("NoGld");
    case CTR:
      return TEXT("CTR");
    case WAVE:
      return TEXT("Wav");
    default:
      return TEXT("?");
    }
}

void CAirspaceManager::GetAirspaceAltText(TCHAR *buffer, int bufferlen, const AIRSPACE_ALT *alt) const
{
  TCHAR sUnitBuffer[24];
  TCHAR sAltUnitBuffer[24];
  TCHAR intbuf[128];

  Units::FormatUserAltitude(alt->Altitude, sUnitBuffer, sizeof(sUnitBuffer)/sizeof(sUnitBuffer[0]));
  Units::FormatAlternateUserAltitude(alt->Altitude, sAltUnitBuffer, sizeof(sAltUnitBuffer)/sizeof(sAltUnitBuffer[0]));

  switch (alt->Base) {
    case abUndef:
      if (Units::GetUserAltitudeUnit() == unMeter) {
        _stprintf(intbuf, TEXT("%s %s"), sUnitBuffer, sAltUnitBuffer);
      } else {
        _stprintf(intbuf, TEXT("%s"), sUnitBuffer);
      }
      break;
    case abMSL:
      if (Units::GetUserAltitudeUnit() == unMeter) {
        _stprintf(intbuf, TEXT("%s %s MSL"), sUnitBuffer, sAltUnitBuffer);
      } else {
        _stprintf(intbuf, TEXT("%s MSL"), sUnitBuffer);
      }
      break;
    case abAGL:
      if (alt->AGL <= 0)
        _stprintf(intbuf, TEXT("SFC"));
      else {
        Units::FormatUserAltitude(alt->AGL, sUnitBuffer, sizeof(sUnitBuffer)/sizeof(sUnitBuffer[0]));
        Units::FormatAlternateUserAltitude(alt->AGL, sAltUnitBuffer, sizeof(sAltUnitBuffer)/sizeof(sAltUnitBuffer[0]));
        if (Units::GetUserAltitudeUnit() == unMeter) {
          _stprintf(intbuf, TEXT("%s %s AGL"), sUnitBuffer, sAltUnitBuffer);
        } else {
          _stprintf(intbuf, TEXT("%s AGL"), sUnitBuffer);
        }
      }
      break;
    case abFL:
      if (Units::GetUserAltitudeUnit() == unMeter) {
        _stprintf(intbuf, TEXT("FL%.0f %.0fm %.0fft"), alt->FL, alt->Altitude, alt->Altitude*TOFEET);
      } else {
        _stprintf(intbuf, TEXT("FL%.0f %.0fft"), alt->FL, alt->Altitude*TOFEET);
      }
      break;
  }
  _tcsncpy(buffer, intbuf, bufferlen-1);
  buffer[bufferlen-1]=0;
}

#endif /* LKAIRSPACE */
