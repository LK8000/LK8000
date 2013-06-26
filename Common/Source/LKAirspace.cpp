/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

*/

#include "externs.h"
#include "LKAirspace.h"
#include "RasterTerrain.h"
#include "LKProfiles.h"
#include "dlgTools.h"

#include <ctype.h>

#include <Point2D.h>
#include "md5.h"

#include "utils/2dpclip.h"

#if TESTBENCH
//#define DEBUG_NEAR_POINTS	1
 #define DEBUG_AIRSPACE
#endif


static const int k_nAreaCount = 14;
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
                    _T("G"),
                    _T("TMZ")
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
                    CLASSG,
                    CLASSTMZ};


// CAirspaceManager class attributes
CAirspaceManager CAirspaceManager::_instance = CAirspaceManager(CAirspaceManager::_instance);

// CAirspace class attributes
#ifndef LKAIRSP_INFOBOX_USE_SELECTED 
int CAirspace::_nearesthdistance = 0;            // for infobox
int CAirspace::_nearestvdistance = 0;            // for infobox
TCHAR* CAirspace::_nearesthname = NULL;            // for infobox
TCHAR* CAirspace::_nearestvname = NULL;            // for infobox
#endif
bool CAirspace::_pos_in_flyzone = false;        // for refine warnings in flyzones
bool CAirspace::_pred_in_flyzone = false;        // for refine warnings in flyzones
bool CAirspace::_pos_in_acked_nonfly_zone = false;        // for refine warnings in flyzones
bool CAirspace::_pred_in_acked_nonfly_zone = false;        // for refine warnings in flyzones
int CAirspace::_now = 0;                        // gps time saved
int CAirspace::_hdistancemargin = 0;            // calculated horizontal distance margin to use
CPoint2D CAirspace::_lastknownpos(0,0);                // last known position saved for calculations
int CAirspace::_lastknownalt = 0;                // last known alt saved for calculations
int CAirspace::_lastknownagl = 0;                // last known agl saved for calculations
int CAirspace::_lastknownheading = 0;            // last known heading saved for calculations
int CAirspace::_lastknowntrackbearing = 0;       // last known track bearing saved for calculations
bool CAirspace::_pred_blindtime = true;               // disable predicted position based warnings near takeoff, and other conditions
CAirspace* CAirspace::_sideview_nearest_instance = NULL;  // collect nearest airspace instance for sideview during warning calculations

//
// CAIRSPACE CLASS
//

// Dumps object instance to Runtime.log
void CAirspace::Dump() const
{
  //StartupStore(TEXT("CAirspace Dump%s"),NEWLINE);
  StartupStore(TEXT(" Name:%s%s"),_name,NEWLINE);
  StartupStore(TEXT(" Type:%d (%s)%s"),_type,k_strAreaStart[_type], NEWLINE);
  StartupStore(TEXT(" Base.Altitude:%f%s"),_base.Altitude,NEWLINE);
  StartupStore(TEXT(" Base.FL:%f%s"),_base.FL,NEWLINE);
  StartupStore(TEXT(" Base.AGL:%f%s"),_base.AGL,NEWLINE);
  StartupStore(TEXT(" Base.Base:%d%s"),_base.Base,NEWLINE);
  StartupStore(TEXT(" Top.Altitude:%f%s"),_top.Altitude,NEWLINE);
  StartupStore(TEXT(" Top.FL:%f%s"),_top.FL,NEWLINE);
  StartupStore(TEXT(" Top.AGL:%f%s"),_top.AGL,NEWLINE);
  StartupStore(TEXT(" Top.Base:%d%s"),_top.Base,NEWLINE);
  StartupStore(TEXT(" bounds.minx,miny:%f,%f%s"),_bounds.minx,_bounds.miny,NEWLINE);
  StartupStore(TEXT(" bounds.maxx,maxy:%f,%f%s"),_bounds.maxx,_bounds.maxy,NEWLINE);
   
}

const TCHAR* CAirspace::TypeName(void) const
{
	return ((TCHAR*) (CAirspaceManager::Instance().GetAirspaceTypeText(_type)));

};

const COLORREF CAirspace::TypeColor(void) const
{
	return MapWindow::GetAirspaceColourByClass(_type);
}

const HBRUSH CAirspace::TypeBrush(void) const
{
	return MapWindow::GetAirspaceBrushByClass(_type);
}


// Calculate unique hash code for this airspace - prototype, normally never called
void CAirspace::Hash(char *hashout, int maxbufsize) const
{
  *hashout=0;
}

void CAirspace::AirspaceAGLLookup(double av_lat, double av_lon, double *basealt_out, double *topalt_out) const
{
  double base_out = _base.Altitude;
  double top_out = _top.Altitude;
  
  if (((_base.Base == abAGL) || (_top.Base == abAGL))) {
    RasterTerrain::Lock();
    // want most accurate rounding here
    RasterTerrain::SetTerrainRounding(0,0);
    double th = RasterTerrain::GetTerrainHeight(av_lat, av_lon);

    if (th==TERRAIN_INVALID) th=0; //@ 101027 FIX
    // 101027 We still use 0 altitude for no terrain, what else can we do..
    RasterTerrain::Unlock();
    
    if (_base.Base == abAGL) {
      base_out = th;
      if (_base.AGL>=0) base_out += _base.AGL;
    }
    if (_top.Base == abAGL) {
      top_out = th;
      if (_top.AGL>=0) top_out += _top.AGL;
    }
  }
  if (basealt_out) *basealt_out = base_out;
  if (topalt_out) *topalt_out = top_out;
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
  
#ifndef LKAIRSP_INFOBOX_USE_SELECTED 
  _nearesthname = NULL; 
  _nearestvname = NULL; 
  _nearesthdistance=100000; 
  _nearestvdistance=100000;
#endif

  _sideview_nearest_instance = NULL;     // Init nearest instance for sideview

  // 110518 PENDING_QUESTION
  // From Paolo to Kalman: casting a double to a signed int won't create problems 
  // if for any reason it overflows the positive sign, going negative?
  // Kalman: overflow occurs after 24855days (68years) runtime, i think it will not cause problems.
  _now = (int)Basic->Time;
  
  //Save position for further calculations made by gui threads
  if (Basic->BaroAltitudeAvailable && EnableNavBaroAltitude) {
    _lastknownalt = (int)Basic->BaroAltitude;
  } else {
    _lastknownalt = (int)Basic->Altitude;
  }
  _lastknownagl = (int)Calculated->AltitudeAGL;
  if (_lastknownagl < 0) _lastknownagl = 0;            // Limit agl to zero
  CPoint2D position_now(Basic->Latitude, Basic->Longitude);
  _lastknownpos = position_now;

  // Horizontal distance margin
   _hdistancemargin = (int) (Basic->Speed * WarningTime); // 110518 casting forced

  // Heading
   _lastknownheading = (int) Calculated->Heading;
   
  // Track bearing
  _lastknowntrackbearing = (int) Basic->TrackBearing;
   
  // Predicted position blind time near takeoff
   _pred_blindtime = false;
   if ((!Calculated->Flying)  || ((!SIMMODE)&&((Basic->Time - Calculated->TakeOffTime) < 60))) _pred_blindtime = true;
   // When we are inside dlgConfiguration, NO AIRSPACE WARNINGS!
   if (MenuActive) _pred_blindtime = true;
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
  if (Basic->BaroAltitudeAvailable && EnableNavBaroAltitude) {
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

#ifndef LKAIRSP_INFOBOX_USE_SELECTED   
  if (_flyzone && _pos_inside_now) {
    // If in flyzone, nearest warning point given (nearest distance to leaving the fly zone)
    if ( abs(_hdistance) < abs(_nearesthdistance) ) {
      _nearesthname = _name;
      _nearesthdistance = abs(_hdistance);
    }
    if ( abs(_vdistance) < abs(_nearestvdistance) ) {
      _nearestvname = _name;
      _nearestvdistance = _vdistance;
    }
  }
  if (!_flyzone) {
    if (_pos_inside_now) {
      // Inside a non fly zone, distance is zero
      _nearesthname = _name;
      _nearesthdistance = 0;
      _nearestvname = _name;
      _nearestvdistance = 0;
    } else {
      // If outside nofly zone, then nearest distance selected
      // Do not count it, if directly above or below (_hdistance<=0), or give zero horiz distance?
      if ( (abs(_hdistance) < abs(_nearesthdistance)) && (_hdistance > 0) && IsAltitudeInside(alt,agl,AirspaceWarningVerticalMargin/10) ) {
        _nearesthname = _name;
        _nearesthdistance = abs(_hdistance);
      }
      // Just directly above or below distances counts
      if ( (abs(_vdistance) < abs(_nearestvdistance)) && (_hdistance < 0) ) {
        _nearestvname = _name;
        _nearestvdistance = _vdistance;
      }
    }
  }
#endif

  if (_sideview_nearest_instance==NULL) {
    _sideview_nearest_instance = this;
  } else {
    if (_3ddistance > 0) {
      if (_3ddistance < _sideview_nearest_instance->_3ddistance) {
        _sideview_nearest_instance = this;
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
    int abs_beardiff = abs((int)AngleLimit180(_lastknownheading - _bearing));
    
    //Calculate warning state based on airspace warning events
    switch (_warnevent) {
      default:
        break;

      // Events for FLY zones
      case aweMovingInsideFly:
        // If far away from border, set warnlevel to none
        // If base is sfc, we skip near warnings to base, to not get disturbing messages on landing.
        if ( (abs_hdistance>(_hdistancemargin+hdistance_histeresis)) && 
             (abs_vdistance>((AirspaceWarningVerticalMargin/10)+vdistance_histeresis)) 
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
          double dist = abs(_hdistance) + hdistance_lookout;
          FindLatitudeLongitude(_lastknownpos.Latitude(), _lastknownpos.Longitude(), _bearing, dist, &lat, &lon);
          
          if ( !CAirspaceManager::Instance().AirspaceWarningIsGoodPosition(lon, lat, _lastknownalt, _lastknownagl) ) {
            // Near to outside, modify warnevent to inform user
            _warninglevel = awYellow;
            _warnevent = aweNearOutsideFly;
            _hwarninglabel_hide = false;
          }
        }
        
        _vwarninglabel_hide = true;
        if (abs_vdistance<(AirspaceWarningVerticalMargin/10)) {
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
        if (_pred_blindtime) break;         //Do not count predicted events near takeoff, filters not settled yet
        if ( !(_pred_in_flyzone || _pred_in_acked_nonfly_zone) ) {
              // if predicted position not in other fly or acked nonfly zone, then leaving this one should be wrong
              _warninglevel = awYellow;
        }
        break;
        
      case aweLeavingFly:
        if (_pred_blindtime) break;         //Do not count predicted events near takeoff, filters not settled yet
        if ( !(_pos_in_flyzone || _pos_in_acked_nonfly_zone) ) {
              // if current position not in other fly or acked nonfly zone, then leaving this one should be wrong
              _warninglevel = awRed;
        }
        break;
        
      case awePredictedEnteringFly:
        break;
        
      case aweEnteringFly:
        if (_pred_blindtime) break;         //Do not count predicted events near takeoff, filters not settled yet
        // Also preset warnlevel to awYellow, because we entering yellow zone. 
        // but we don't need to generate a warning message right now - force no change in warnlevel
        _hwarninglabel_hide = true;
        if (abs_hdistance<_hdistancemargin) {
          // Check what is outside this flyzone. If another flyzone or acked nonfly zone, then we don't have to increase the warn state
          double lon = 0;
          double lat =0;
          double dist = abs(_hdistance) + hdistance_lookout;
          FindLatitudeLongitude(_lastknownpos.Latitude(), _lastknownpos.Longitude(), _bearing, dist, &lat, &lon);
          
          if ( !CAirspaceManager::Instance().AirspaceWarningIsGoodPosition(lon, lat, _lastknownalt, _lastknownagl) ) {
            _warninglevelold = _warninglevel = awYellow;          
            _hwarninglabel_hide = false;
          }
        }
        
        _vwarninglabel_hide = true;
        if (abs_vdistance<(AirspaceWarningVerticalMargin/10)) {
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
        if (_pred_blindtime) break;         //Do not count predicted events near takeoff, filters not settled yet
        if ( (_hdistance > (_hdistancemargin + hdistance_histeresis)) ||
             (!IsAltitudeInside(_lastknownalt, _lastknownagl, (AirspaceWarningVerticalMargin/10) + vdistance_histeresis))
          ) {
            // Far away horizontally _or_ vertically
            _warninglevel = awNone;
        }
        if ( (_hdistance < _hdistancemargin) && (abs_beardiff<=90) ) {
          if (IsAltitudeInside(_lastknownalt, _lastknownagl, (AirspaceWarningVerticalMargin/10))) {
            // Near to inside and moving closer, modify warnevent to inform user
            _warninglevel = awYellow;
            _warnevent = aweNearInsideNonfly;
          }
        }
        break;
        
      case awePredictedEnteringNonfly:
        if (_pred_blindtime) break;         //Do not count predicted events near takeoff, filters not settled yet
        _warninglevel = awYellow;
        break;
        
      case aweEnteringNonfly:
        if (_pred_blindtime) break;         //Do not count predicted events near takeoff, filters not settled yet
        _warninglevel = awRed;
        break;

      case aweMovingInsideNonfly:
        if (_pred_blindtime) break;         //Do not count predicted events near takeoff, filters not settled yet
        _warninglevel = awRed;
        break;
        
      case aweLeavingNonFly:
        if (_pred_blindtime) break;         //Do not count predicted events near takeoff, filters not settled yet
        _warninglevel = awYellow;
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

    double dist = abs(_hdistance);
    double basealt, topalt;
    FindLatitudeLongitude(_lastknownpos.Latitude(), _lastknownpos.Longitude(), _bearing, dist, &latitude, &longitude);
    AirspaceAGLLookup(latitude, longitude, &basealt, &topalt);
    
    vdrawstyle = awsBlack;
    if ( (_lastknownalt >= basealt) && ( _lastknownalt < topalt)) {
          if (!_flyzone) vdrawstyle = awsRed;
    } else {
        if (_flyzone) vdrawstyle = awsAmber;
    }
    hdrawstyle = vdrawstyle;
    
    vDistance = _vdistance;
    //if (abs(_vdistance) > (AirspaceWarningVerticalMargin/10)) vdrawstyle = awsHidden;

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

/******************************************************
 * compare name and type for gruping airspaces
 ******************************************************/
bool CAirspace::IsSame( CAirspace &as2 )
{
bool ret = false;
	if(_type  == as2.Type())
	  if( _tcscmp((_name),  (as2.Name()) ) == 0 )
        ret = true;
return ret;
}

// Calculates nearest horizontal, vertical and 3d distance to airspace based on last known position
// Returns true if inside, false if outside
bool CAirspace::CalculateDistance(int *hDistance, int *Bearing, int *vDistance)
{
  bool inside = true;
  int vDistanceBase;
  int vDistanceTop;
  double fbearing;
  double distance;

  distance = Range(_lastknownpos.Longitude(), _lastknownpos.Latitude(), fbearing);
  if (distance > 0) {
    inside = false;
    // if outside we need the terrain height at the intersection point
    double intersect_lat, intersect_lon;
    FindLatitudeLongitude(_lastknownpos.Latitude(), _lastknownpos.Longitude(), fbearing, distance, &intersect_lat, &intersect_lon);
    AirspaceAGLLookup(intersect_lat, intersect_lon, &_base.Altitude, &_top.Altitude);
  } else {
    // if inside we need the terrain height at the current position
    AirspaceAGLLookup(_lastknownpos.Latitude(), _lastknownpos.Longitude(), &_base.Altitude, &_top.Altitude);
  }
  vDistanceBase = _lastknownalt - (int)(_base.Altitude);
  vDistanceTop  = _lastknownalt - (int)(_top.Altitude);

  if (vDistanceBase < 0 || vDistanceTop > 0) inside = false;

  _bearing = (int)fbearing;
  _hdistance = (int)distance;
  if ((-vDistanceBase > vDistanceTop) && ((_base.Base != abAGL) || (_base.AGL>0)))
    _vdistance = vDistanceBase;
  else
    _vdistance = vDistanceTop;
  
  // 3d distance calculation
  if (_hdistance>0) {
    //outside horizontally
    if (vDistanceBase < 0 || vDistanceTop > 0) {
      //outside vertically
      _3ddistance = (int)sqrt(distance*distance + (double)_vdistance*(double)_vdistance);
    } else {
      //inside vertically
      _3ddistance = _hdistance;
    }
  } else {
    //inside horizontally
    if (vDistanceBase < 0 || vDistanceTop > 0) {
      //outside vertically
      _3ddistance = abs(_vdistance);
    } else {
      //inside vertically
      if (abs(_vdistance) < abs(_hdistance)) _3ddistance = -abs(_vdistance); else _3ddistance = _hdistance;
    }
  }
  
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
  _warninglevelold = awNone;
  _distances_ready = false;
}

// Initialize instance attributes
void CAirspace::Init(const TCHAR *name, const int type, const AIRSPACE_ALT &base, const AIRSPACE_ALT &top, bool flyzone)
{ 
  LK_tcsncpy(_name, name, NAME_SIZE); 
  _type = type;
  memcpy(&_base, &base, sizeof(_base));
  memcpy(&_top, &top, sizeof(_top));
  _flyzone = flyzone;
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
    _screenpoints_clipped.reserve(65);
    _screenpoints.reserve(65);
    CalcBounds();
    AirspaceAGLLookup(Center_Latitude, Center_Longitude, &_base.Altitude, &_top.Altitude);
}

// Dumps object instance to Runtime.log
void CAirspace_Circle::Dump() const
{
  StartupStore(TEXT("CAirspace_Circle Dump, CenterLat:%f, CenterLon:%f, Radius:%f%s"), _latcenter, _loncenter, _radius, NEWLINE);
  CAirspace::Dump();
}

// Calculate unique hash code for this airspace
void CAirspace_Circle::Hash(char *hashout, int maxbufsize) const
{
  MD5 md5;
  md5.Update((unsigned char*)&_type, sizeof(_type));
  md5.Update((unsigned char*)_name, _tcslen(_name)*sizeof(TCHAR));
  if (_base.Base == abFL) md5.Update((unsigned char*)&_base.FL, sizeof(_base.FL));
  if (_base.Base == abAGL) md5.Update((unsigned char*)&_base.AGL, sizeof(_base.AGL));
  if (_base.Base == abMSL) md5.Update((unsigned char*)&_base.Altitude, sizeof(_base.Altitude));
  if (_top.Base == abFL) md5.Update((unsigned char*)&_top.FL, sizeof(_top.FL));
  if (_top.Base == abAGL) md5.Update((unsigned char*)&_top.AGL, sizeof(_top.AGL));
  if (_top.Base == abMSL) md5.Update((unsigned char*)&_top.Altitude, sizeof(_top.Altitude));
  md5.Update((unsigned char*)&_latcenter, sizeof(_latcenter));
  md5.Update((unsigned char*)&_loncenter, sizeof(_loncenter));
  md5.Update((unsigned char*)&_radius, sizeof(_radius));
  md5.Final();
  memcpy(hashout,md5.digestChars,min(maxbufsize,33));
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






void CAirspace_Circle::DrawPicto(HDC hDCTemp, const RECT &rc, bool param1)  {

	double fact = 1.0;
	CalculatePictPosition(_bounds, rc, fact);
    SelectObject(hDCTemp,TypeBrush());
    SetTextColor(hDCTemp,TypeColor());
    HPEN FramePen = (HPEN) CreatePen(PS_SOLID, IBLSCALE(1), TypeColor());
    HPEN oldPen   = (HPEN) SelectObject(hDCTemp, FramePen);


	Draw(hDCTemp,  rc ,  param1);

    SelectObject(hDCTemp, oldPen);
    DeleteObject(FramePen);
}


void CAirspace_Area::DrawPicto(HDC hDCTemp, const RECT &rc, bool param1)   {


	double fact = 1.0;
	CalculatePictPosition(_bounds, rc, fact);
    SelectObject(hDCTemp,TypeBrush());
    SetTextColor(hDCTemp,TypeColor());
    HPEN FramePen = (HPEN) CreatePen(PS_SOLID, IBLSCALE(1), TypeColor());
    HPEN oldPen   = (HPEN) SelectObject(hDCTemp, FramePen);

	Draw(hDCTemp,  rc ,  param1);

    SelectObject(hDCTemp, oldPen);
    DeleteObject(FramePen);

}

void CAirspace_Circle::CalculatePictPosition(const rectObj &screenbounds_latlon, const RECT& rcDraw,  double zoom)
{
//  _drawstyle = adsHidden;
	zoom*= 0.9;
	/*
  if (!_enabled)
	_drawstyle = adsOutline;
  else
	_drawstyle = adsFilled;*/
int cx = rcDraw.right-rcDraw.left;
int cy = rcDraw.bottom-rcDraw.top;
double scale = (double) cx;
  if (cy < cx)
	scale = (double) cy;
  scale = scale/(_radius*2.0)*zoom;

  _screenradius = iround(_radius*scale );
  _screencenter.x = rcDraw.left + cx/2;
  _screencenter.y = rcDraw.top + cy/2;

  buildCircle(_screencenter, _screenradius, _screenpoints);
  _screenpoints_clipped.clear();
  LKGeom::ClipPolygon((POINT) {rcDraw.left, rcDraw.top}, (POINT) {rcDraw.right, rcDraw.bottom}, _screenpoints, _screenpoints_clipped);

}

void CAirspace_Area::CalculatePictPosition(const rectObj &screenbounds_latlon, const RECT& rcDraw,  double zoom)
{
int cx = (rcDraw.right-rcDraw.left);
int cy = (rcDraw.bottom-rcDraw.top);
int xoff = rcDraw.left + cx/2;
int yoff = rcDraw.top  + cy/2;
POINT tmpPnt,lastPt={0,0};
double dlon = _bounds.maxx  -   _bounds.minx;
double dlat = _bounds.maxy  -   _bounds.miny;
double PanLongitudeCenter = _bounds.minx + dlon/2;
double PanLatitudeCenter  = _bounds.miny + dlat/2;
zoom *= 0.9;

double	scaleX = (double)(cx)/dlon*zoom/fastcosine(PanLatitudeCenter);
double	scaleY = (double)(cy)/dlat*zoom;
double scale;
  if(scaleX < scaleY)
	scale = scaleX;
  else
	scale = scaleY;

  if (!_enabled)
	_drawstyle = adsOutline;
  else
	_drawstyle = adsFilled;


/*
  StartupStore(_T("Definitions: %s"), NEWLINE);
  StartupStore(_T("%f %f %f %f %s"),_bounds.minx, _bounds.maxx, _bounds.miny,_bounds.maxy, NEWLINE);
  StartupStore(_T("%i %i %i %i %s"),rcDraw.left, rcDraw.right, rcDraw.bottom, rcDraw.top, NEWLINE);

  StartupStore(_T("Range: %s"), NEWLINE);
  StartupStore(_T("%f %f %s"),dlon, dlat, NEWLINE);
  StartupStore(_T("%i %i %s"),cx, cy, NEWLINE);
  StartupStore(_T("Scale: %s"), NEWLINE);
  StartupStore(_T("%f  %s"),scale, NEWLINE);

  StartupStore(_T("Center: %s"), NEWLINE);
  StartupStore(_T("%f %f %s"),PanLongitudeCenter, PanLatitudeCenter, NEWLINE);
  StartupStore(_T("%i %i %s"),xoff, yoff, NEWLINE);
*/


/**************************************************************
 * if PICTO_CLIPPING is defined the polygon will be clipped
 * normally this is not needed since we scale the airspace into
 * the given rectange, this may speedup drawing
 *************************************************************/
#define PICTO_CLIPPING
#ifndef PICTO_CLIPPING
  _screenpoints_clipped.clear();
  CPoint2DArray::iterator it;
  for (it = _geopoints.begin();  it != _geopoints.end(); ++it)
  {
   	tmpPnt.x =xoff + Real2Int((it->Longitude()- PanLongitudeCenter)*fastcosine(it->Latitude()) *scale);
   	tmpPnt.y =yoff - Real2Int((it->Latitude() - PanLatitudeCenter)*scale);
    StartupStore(_T("%i %i %s"),	tmpPnt.x, tmpPnt.y, NEWLINE);
    if (_screenpoints_clipped.size() ==0)
    {
      _screenpoints_clipped.push_back(tmpPnt);
      lastPt = tmpPnt;
    }
    else
    {
      if((abs(lastPt.x - tmpPnt.x) > 3 ) || (abs(lastPt.y - tmpPnt.y) > 3 ))
      {
    	_screenpoints_clipped.push_back(tmpPnt);
        lastPt = tmpPnt;
      }
    }
  }
#else
  _screenpoints.clear();
  CPoint2DArray::iterator it;
  for (it = _geopoints.begin();  it != _geopoints.end(); ++it)
  {
   	tmpPnt.x =xoff - Real2Int((PanLongitudeCenter- it->Longitude())*fastcosine(it->Latitude())*scale);
   	tmpPnt.y =yoff + Real2Int((PanLatitudeCenter - it->Latitude())*scale);

    if (_screenpoints.size() ==0)
    {
    	_screenpoints.push_back(tmpPnt);
      lastPt = tmpPnt;
    }
    else
    {
//      if((abs(lastPt.x - tmpPnt.x) > 2 ) || (abs(lastPt.y - tmpPnt.y) > 2 ))
      {
    	_screenpoints.push_back(tmpPnt);
        lastPt = tmpPnt;
      }
    }
  }
  _screenpoints_clipped.clear();
  _screenpoints_clipped.reserve(_screenpoints.size());

  LKGeom::ClipPolygon((POINT) {rcDraw.left, rcDraw.top}, (POINT) {rcDraw.right, rcDraw.bottom}, _screenpoints, _screenpoints_clipped);
#endif


}



// Calculate screen coordinates for drawing
void CAirspace_Circle::CalculateScreenPosition(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const RECT& rcDraw, const double &ResMapScaleOverDistanceModify) 
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
        
        buildCircle(_screencenter, _screenradius, _screenpoints);
        
        _screenpoints_clipped.clear();
        LKGeom::ClipPolygon((POINT) {rcDraw.left, rcDraw.top}, (POINT) {rcDraw.right, rcDraw.bottom}, _screenpoints, _screenpoints_clipped);
      }
    }
  }
}

// Draw airspace
void CAirspace_Circle::Draw(HDC hDCTemp, const RECT &rc, bool param1) const {
    size_t outLength = _screenpoints_clipped.size();
    const POINT * clip_ptout = &(*_screenpoints_clipped.begin());

    if (param1) {
        if (outLength > 2) {
            Polygon(hDCTemp, clip_ptout, outLength);
        }
    } else {
        if (outLength > 1) {
            Polyline(hDCTemp, clip_ptout, outLength);
        }
    }
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
    StartupStore(TEXT("  Point lat:%f, lon:%f%s"), i->Latitude(), i->Longitude(), NEWLINE);
  }
}

// Calculate unique hash code for this airspace
void CAirspace_Area::Hash(char *hashout, int maxbufsize) const
{
  MD5 md5;
  double dtemp;
  
  md5.Update((unsigned char*)&_type, sizeof(_type));
  md5.Update((unsigned char*)_name, _tcslen(_name)*sizeof(TCHAR));
  if (_base.Base == abFL) md5.Update((unsigned char*)&_base.FL, sizeof(_base.FL));
  if (_base.Base == abAGL) md5.Update((unsigned char*)&_base.AGL, sizeof(_base.AGL));
  if (_base.Base == abMSL) md5.Update((unsigned char*)&_base.Altitude, sizeof(_base.Altitude));
  if (_top.Base == abFL) md5.Update((unsigned char*)&_top.FL, sizeof(_top.FL));
  if (_top.Base == abAGL) md5.Update((unsigned char*)&_top.AGL, sizeof(_top.AGL));
  if (_top.Base == abMSL) md5.Update((unsigned char*)&_top.Altitude, sizeof(_top.Altitude));
  for (CPoint2DArray::const_iterator it = _geopoints.begin(); it != _geopoints.end(); ++it) {
    dtemp = it->Latitude();
    md5.Update((unsigned char*)&dtemp, sizeof(dtemp));
    dtemp = it->Longitude();
    md5.Update((unsigned char*)&dtemp, sizeof(dtemp));
  }
  md5.Final();
  memcpy(hashout,md5.digestChars,min(maxbufsize,33));
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
    AirspaceAGLLookup( (_bounds.miny+_bounds.maxy)/2.0, (_bounds.minx+_bounds.maxx)/2.0, &_base.Altitude, &_top.Altitude); 
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
void CAirspace_Area::CalculateScreenPosition(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const RECT& rcDraw, const double &ResMapScaleOverDistanceModify) 
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
/******************************
 * ULLI remove unneeded points
 ****************************/
#ifndef LKASP_REMOVE_NEAR_POINTS
    POINTList::iterator itr = _screenpoints.begin();
    for (it = _geopoints.begin(), itr = _screenpoints.begin(); it != _geopoints.end(); ++it, ++itr) {
      MapWindow::LatLon2Screen(it->Longitude(), it->Latitude(), *itr);
    }
#else
    POINT tmpPnt,lastPt={0,0};
    _screenpoints.clear();
    for (it = _geopoints.begin();  it != _geopoints.end(); ++it)
    {
      MapWindow::LatLon2Screen(it->Longitude(), it->Latitude(), tmpPnt);

      if (_screenpoints.size() ==0)
      {
        _screenpoints.push_back(tmpPnt);
        lastPt = tmpPnt;
      }
      else
      {
        if((abs(lastPt.x - tmpPnt.x) > 5 ) || (abs(lastPt.y - tmpPnt.y) > 5 ))
        {
          _screenpoints.push_back(tmpPnt);
          lastPt = tmpPnt;
        }
      }
    }
   /*******************************
    * ULLI new code ends here
    *******************************/
#endif
    // add extra point for final point if it doesn't equal the first
    // this is required to close some airspace areas that have missing
    // final point
    if ((_screenpoints[_screenpoints.size() - 1].x != _screenpoints[0].x) || (_screenpoints[_screenpoints.size() - 1].y != _screenpoints[0].y)) {
        _screenpoints.push_back(_screenpoints[0]);
    }
    
    _screenpoints_clipped.clear();
    _screenpoints_clipped.reserve(_screenpoints.size());

    LKGeom::ClipPolygon((POINT) {rcDraw.left, rcDraw.top}, (POINT) {rcDraw.right, rcDraw.bottom}, _screenpoints, _screenpoints_clipped);
    #if DEBUG_NEAR_POINTS
    //StartupStore(_T("... area point geo %i screen %i\n"),_geopoints.size(),_screenpoints.size() );
    #endif
      }
    }
  }
}

// Draw airspace
void CAirspace_Area::Draw(HDC hDCTemp, const RECT &rc, bool param1) const {

    size_t outLength = _screenpoints_clipped.size();
    const POINT * clip_ptout = &(*_screenpoints_clipped.begin());

    if (param1) {
        if (outLength > 2) {
            Polygon(hDCTemp, clip_ptout, outLength);
        }
    } else {
        if (outLength > 1) {
            Polyline(hDCTemp, clip_ptout, outLength);
        }
    }

}
//
// CAIRSPACEMANAGER CLASS
//

bool CAirspaceManager::StartsWith(const TCHAR *Text, const TCHAR *LookFor) const
{
  while(1) {
    if (!(*LookFor)) return true;
    if (*Text != *LookFor) return false;
    ++Text; ++LookFor;
  }
}

bool CAirspaceManager::CheckAirspaceAltitude(const AIRSPACE_ALT &Base, const AIRSPACE_ALT &Top) const
{
    if(AltitudeMode == ALLON) {
        return true;
    } else if(AltitudeMode == ALLOFF) {
        return false;
    } 
  
  double alt;
  double basealt;
  double topalt;
  bool base_is_sfc = false;
  
  LockFlightData();
  if (GPS_INFO.BaroAltitudeAvailable && EnableNavBaroAltitude) {
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
  UnlockFlightData();

  switch (AltitudeMode)
    {
    case ALLON : return true;
        
    case CLIP : 
      if ((basealt < (ClipAltitude/10)) || base_is_sfc) return true; else return false;

    case AUTO:
      if( (( alt > (basealt - (AltWarningMargin/10))) || base_is_sfc )
      && ( alt < (topalt + (AltWarningMargin/10)) ))
    return true;
      else
    return false;

    case ALLBELOW:
      if(  ((basealt - (AltWarningMargin/10)) < alt ) || base_is_sfc )
    return  true;
      else
    return false;
    case INSIDE:
      if( (( alt >= basealt ) || base_is_sfc ) && ( alt < topalt ) )
    return true;
      else
        return false;
    case ALLOFF : return false;
    }
  return true;
}

void CAirspaceManager::ReadAltitude(const TCHAR *Text, AIRSPACE_ALT *Alt) const
{
  TCHAR *Stop = NULL;
  TCHAR sTmp[128];
  TCHAR *pWClast = NULL;
  TCHAR *pToken;
  bool  fHasUnit=false;

  LK_tcsncpy(sTmp, Text, sizeof(sTmp)/sizeof(sTmp[0])-1);

  _tcsupr(sTmp);

  pToken = strtok_r(sTmp, (TCHAR*)TEXT(" "), &pWClast);

  Alt->Altitude = 0;
  Alt->FL = 0;
  Alt->AGL = 0;
  Alt->Base = abUndef;

  while((pToken != NULL) && (*pToken != '\0')){

    //BugFix 110922 
    //Malformed alt causes the parser to read wrong altitude, for example on line  AL FL65 (MNM ALT 5500ft)
    //Stop parsing if we have enough info!
    if ( (Alt->Base != abUndef) && (fHasUnit) && ((Alt->Altitude!=0) || (Alt->FL!=0) || (Alt->AGL!=0)) ) break;
    
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

  while (*Stop == ' ') Stop++;

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

  while (*Stop == ' ') Stop++;
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
  double arc_bearing_range;
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
  
  if (Rotation>0) arc_bearing_range = EndBearing - StartBearing; else arc_bearing_range = StartBearing - EndBearing;
  if (arc_bearing_range>360) arc_bearing_range -= 360;
  if (arc_bearing_range<0) arc_bearing_range += 360;
  
  while( arc_bearing_range > 7.5 )
  {
      StartBearing += Rotation *5 ;
      arc_bearing_range -= 5;
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
  double arc_bearing_range;
  TCHAR *Stop = NULL;
  double lat=0,lon=0;
  
  // TODO 110307 FIX problem of StrToDouble returning 0.0 in case of error , and not setting Stop!!
  Radius = NAUTICALMILESTOMETRES * (double)StrToDouble(Text, &Stop);
  StartBearing = (double)StrToDouble(&Stop[1], &Stop);
  EndBearing = (double)StrToDouble(&Stop[1], &Stop);

  if (Rotation>0) arc_bearing_range = EndBearing - StartBearing; else arc_bearing_range = StartBearing - EndBearing;
  if (arc_bearing_range>360) arc_bearing_range -= 360;
  if (arc_bearing_range<0) arc_bearing_range += 360;
  
  while( arc_bearing_range > 7.5 )
  {
    if(StartBearing >= 360) StartBearing -= 360;
    if(StartBearing < 0) StartBearing += 360;

    FindLatitudeLongitude(CenterY, CenterX, StartBearing, Radius, &lat, &lon);

    _geopoints->push_back(CPoint2D(lat,lon));
    
    StartBearing += Rotation *5 ;
    arc_bearing_range -= 5;
  }
  FindLatitudeLongitude(CenterY, CenterX, EndBearing, Radius, &lat, &lon);
  _geopoints->push_back(CPoint2D(lat,lon));
  return true;
}

// Correcting geopointlist
// All algorithms require non self-intersecting and closed polygons.
// Also the geopointlist last element have to be the same as first -> openair doesn't require this, we have to do it here
// Also delete adjacent duplicated vertexes
void CAirspaceManager::CorrectGeoPoints(CPoint2DArray &points)
{
    if (points.size()==0) return;
    
    // Close polygon if not closed
    CPoint2D first = points.front();
    CPoint2D last = points.back();
    if ( (first.Latitude() != last.Latitude()) || (first.Longitude() != last.Longitude()) ) points.push_back(first);
    
    // Delete duplicated vertexes
    bool firstrun = true;
    CPoint2DArray::iterator it = points.begin();
    while (it != points.end()) {
      if (!firstrun && ((*it).Latitude() == last.Latitude()) && ((*it).Longitude() == last.Longitude())) {
        it = points.erase(it);
        continue;
      }
      last = *it;
      ++it;
      firstrun = false;
    }
}

// Reading and parsing OpenAir airspace file
void CAirspaceManager::FillAirspacesFromOpenAir(ZZIP_FILE *fp)
{
  TCHAR    *Comment;
  int        nSize;
  TCHAR Text[READLINE_LENGTH+1];
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
              if (Name[0]!='\0') {  // FIX: do not add airspaces with no name defined.
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
            if (parsing_state == 10) {
              LK_tcsncpy(Name, p, NAME_SIZE);
            }
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
            p++; p++; // skip A and space
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
        p++; //skip V
        if (*p==' ') p++;   //skip space if present
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
  StartupStore(TEXT(". Now we have %d airspaces%s"), _airspaces.size(), NEWLINE);
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

  _tcscpy(szFile1,szAirspaceFile);
  _tcscpy(szFile2,szAdditionalAirspaceFile);
  ExpandLocalPath(szFile1);
  ExpandLocalPath(szFile2);

  if (_tcslen(szFile1)>0) {
    fp  = zzip_fopen(szFile1, "rt");
  } else {
  }

  if (_tcslen(szFile2)>0) {
    fp2 = zzip_fopen(szFile2, "rt");
  }

  _tcscpy(szAirspaceFile,_T(""));
  _tcscpy(szAdditionalAirspaceFile,_T(""));

  if (fp != NULL){
    FillAirspacesFromOpenAir(fp);
    zzip_fclose(fp);

    // file 1 was OK, so save it
    ContractLocalPath(szFile1);
    _tcscpy(szAirspaceFile,szFile1);

    // also read any additional airspace
    if (fp2 != NULL) {
      FillAirspacesFromOpenAir(fp2);
      zzip_fclose(fp2);
      
      // file 2 was OK, so save it
      ContractLocalPath(szFile2);
      _tcscpy(szAdditionalAirspaceFile,szFile2);
    } else {
      StartupStore(TEXT(". No airspace file 2%s"),NEWLINE);
    }
  } else {
    StartupStore(TEXT("... No airspace file 1%s"),NEWLINE);
  }
  #if ASPWAVEOFF
  AirspaceDisableWaveSectors();
  #endif
  LoadSettings();
}


void CAirspaceManager::CloseAirspaces()
{
  CAirspaceList::iterator it;
  CCriticalSection::CGuard guard(_csairspaces);
  if (_airspaces.size()==0) return;
  SaveSettings();
  _selected_airspace = NULL;
  _sideview_nearest = NULL;
  _user_warning_queue.clear();
  _airspaces_near.clear();
  _airspaces_of_interest.clear();
  _airspaces_page24.clear();
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




int CAirspaceManager::ScanAirspaceLineList(double lats[AIRSPACE_SCANSIZE_X], double lons[AIRSPACE_SCANSIZE_X],
                                        double terrain_heights[AIRSPACE_SCANSIZE_X],
                                        AirSpaceSideViewSTRUCT airspacetype[MAX_NO_SIDE_AS],int iMaxNoAs) const
{
int iNoFoundAS=0;            // number of found airspaces in scan line
unsigned int iSelAS =0;      // current selected airspace for processing
unsigned int i;              // loop variable
CAirspaceList::const_iterator it;
CCriticalSection::CGuard guard(_csairspaces);

  airspacetype[0].psAS= NULL;
  for (it = _airspaces.begin(); it != _airspaces.end(); ++it)
  {
	if(( CheckAirspaceAltitude(*(*it)->Base(), *(*it)->Top())==TRUE)&& (iNoFoundAS < iMaxNoAs-1))
    {
      for (i=0; i<AIRSPACE_SCANSIZE_X; i++)
      {
        if ((*it)->IsHorizontalInside(lons[i], lats[i]))
        {
          BOOL bPrevIn = false;
          if(i > 0)
        	if((*it)->IsHorizontalInside(lons[i-1], lats[i-1]))
        	  bPrevIn = true;

		  if( !bPrevIn)/* new AS section in this view*/
		  {
		   /*********************************************************************
		    * switch to next airspace section
		    *********************************************************************/
		    iSelAS = iNoFoundAS;
		    #if BUGSTOP
		    LKASSERT(iNoFoundAS < MAX_NO_SIDE_AS);
		    #endif
		    if(iNoFoundAS < MAX_NO_SIDE_AS-1) iNoFoundAS++;
		    airspacetype[iNoFoundAS].psAS= NULL; // increment and reset head
           /*********************************************************************/
		    airspacetype[iSelAS].psAS  = (*it) ;
		    airspacetype[iSelAS].iType = (*it)->Type();
		    if(airspacetype[iSelAS].szAS_Name != NULL)
		    {
		      LK_tcsncpy((wchar_t*)  airspacetype[iSelAS].szAS_Name,  (wchar_t*)(*it)->Name(), NAME_SIZE-1);
		    }
		    airspacetype[iSelAS].iIdx = iSelAS;
		    airspacetype[iSelAS].bRectAllowed =  true ;
		    airspacetype[iSelAS].bEnabled = (*it)->Enabled();
            /**********************************************************************
             * allow rectangular shape if no AGL reference
             **********************************************************************/
			if ( ((*it)->Top()->Base == abAGL) || (((*it)->Base()->Base == abAGL)))
			  airspacetype[iSelAS].bRectAllowed = false ;
            /**********************************************************************
             * init with minium rectangle right side may be extended
             **********************************************************************/
		    airspacetype[iSelAS].rc.left   = i;
		    airspacetype[iSelAS].rc.right  = i+1;
		    airspacetype[iSelAS].rc.bottom = (unsigned int) (*it)->Base()->Altitude;
		    airspacetype[iSelAS].rc.top    = (unsigned int) (*it)->Top()->Altitude;
		    airspacetype[iSelAS].iNoPolyPts=0;

		  }
		  int iHeight;
		  airspacetype[iSelAS].rc.right  = i+1;
          if(i==AIRSPACE_SCANSIZE_X-1)
            airspacetype[iSelAS].rc.right= i+3;

		  if(airspacetype[iSelAS].bRectAllowed == false)
		  {

	        if( airspacetype[iSelAS].psAS->Base()->Base == abAGL )
	          iHeight = (unsigned int)(airspacetype[iSelAS].psAS->Base()->AGL + terrain_heights[i]);
		    else
		      iHeight = (unsigned int)airspacetype[iSelAS].psAS->Base()->Altitude;
			LKASSERT((airspacetype[iSelAS].iNoPolyPts) < GC_MAX_POLYGON_PTS);
		    airspacetype[iSelAS].apPolygon[airspacetype[iSelAS].iNoPolyPts++] = (POINT){(LONG)i,(LONG)iHeight};

	        /************************************************************
	         *  resort and copy polygon array
			 **************************************************************/
		    bool bLast= false;
            if(i==AIRSPACE_SCANSIZE_X-1)
              bLast= true;
            else
            {
               if (airspacetype[iSelAS].psAS->IsHorizontalInside(lons[i+1], lats[i+1]))
                 bLast= false;
               else
                 bLast= true;
             }

             if (bLast)
			 {
       		   airspacetype[iSelAS].apPolygon[airspacetype[iSelAS].iNoPolyPts].x = i+1;
               if(i==AIRSPACE_SCANSIZE_X-1)
           		 airspacetype[iSelAS].apPolygon[airspacetype[iSelAS].iNoPolyPts].x = i+3;

   		  	   LKASSERT((airspacetype[iSelAS].iNoPolyPts)   < GC_MAX_POLYGON_PTS);
       		   airspacetype[iSelAS].apPolygon[airspacetype[iSelAS].iNoPolyPts].y = airspacetype[iSelAS].apPolygon[airspacetype[iSelAS].iNoPolyPts-1].y;
       		   airspacetype[iSelAS].iNoPolyPts++;
   			   LKASSERT((airspacetype[iSelAS].iNoPolyPts) < GC_MAX_POLYGON_PTS);
			   int iN = airspacetype[iSelAS].iNoPolyPts;
			   int iCnt=airspacetype[iSelAS].iNoPolyPts;


			   for (int iPt = 0 ;iPt < iN; iPt++)
			   {
				  LKASSERT(iCnt >= 0);
			      LKASSERT(iCnt < GC_MAX_POLYGON_PTS);

				  airspacetype[iSelAS].apPolygon[iCnt] = airspacetype[iSelAS].apPolygon[iN-iPt-1];
				  if( airspacetype[iSelAS].psAS->Top()->Base == abAGL )
					airspacetype[iSelAS].apPolygon[iCnt].y = (unsigned int)(airspacetype[iSelAS].psAS->Top()->AGL + terrain_heights[airspacetype[iSelAS].apPolygon[iCnt].x]);
				  else
					airspacetype[iSelAS].apPolygon[iCnt].y = (unsigned int) airspacetype[iSelAS].psAS->Top()->Altitude;
			      LKASSERT(iCnt >=iPt);

				  LKASSERT(iPt >= 0);
			      LKASSERT(iPt < GC_MAX_POLYGON_PTS);
                  if(iCnt == 0)
                  {
			        airspacetype[iSelAS].rc.bottom =airspacetype[iSelAS].apPolygon[0].y;
			        airspacetype[iSelAS].rc.top    =airspacetype[iSelAS].apPolygon[0].y;
                  }
                  else
                  {
				    airspacetype[iSelAS].rc.bottom = min(airspacetype[iSelAS].rc.bottom ,airspacetype[iSelAS].apPolygon[iCnt].y);
				    airspacetype[iSelAS].rc.top    = max( airspacetype[iSelAS].rc.top   ,airspacetype[iSelAS].apPolygon[iCnt].y);
                  }
				  if(iCnt < GC_MAX_POLYGON_PTS-1)
					iCnt++;
				}
			    LKASSERT(iCnt < GC_MAX_POLYGON_PTS);
				airspacetype[iSelAS].apPolygon[iCnt++] = airspacetype[iSelAS].apPolygon[0];
				airspacetype[iSelAS].iNoPolyPts = iCnt;
			}
		  }
		  RECT rcs =  airspacetype[iSelAS].rc;
		  airspacetype[iSelAS].iAreaSize = abs(rcs.right - rcs.left) *	abs(rcs.top - rcs.bottom);
        } // inside
      } // finished scanning range
    } // if overlaps bounds
  } // for iterator

return iNoFoundAS;
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
  double calc_terrainalt;
  
  LockFlightData();
  calc_terrainalt = CALCULATED_INFO.TerrainAlt;
  UnlockFlightData();

  CAirspaceList::const_iterator it;
  CCriticalSection::CGuard guard(_csairspaces);

  for (it = _airspaces.begin(); it != _airspaces.end(); ++it) {
  if((*it)->Enabled()){
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
        basealt = (*it)->Base()->AGL + calc_terrainalt;
        if ((*it)->Base()->AGL <= 0) base_is_sfc = true;
      }
      if ((*it)->Top()->Base != abAGL) {
        topalt = (*it)->Top()->Altitude;
      } else {
        topalt = (*it)->Top()->AGL + calc_terrainalt;
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
  } // enabled
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
  #if TESTBENCH
  StartupStore(TEXT(". SortAirspace%s"),NEWLINE);
  #endif

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
 
  CCriticalSection::CGuard guard(_csairspaces);
  CAirspaceList::iterator it;

   // We need a valid GPS fix in FLY mode
   if (Basic->NAVWarning && !SIMMODE) return;
  
  #ifdef DEBUG_AIRSPACE_2
  int starttick = GetTickCount();
  StartupStore(TEXT("---AirspaceWarning start%s"),NEWLINE);
  #endif

  
  switch (step) {
    default:
    case 0:
        // MULTICALC STEP 1
        // Calculate area of interest
        interest_radius = Basic->Speed * WarningTime * 1.25;        // 25% more than required
        if (interest_radius < 10000) interest_radius = 10000;         // but minimum 15km
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
            // Check if it enabled
            if (!(*it)->Enabled()) {
              (*it)->ResetWarnings();
              continue;
            }
            
            (*it)->CalculateWarning( Basic, Calculated );
            _airspaces_of_interest.push_back(*it);
        }
        ++step;
        break;
        
    case 1:
        // MULTICALC STEP 2
        // Step3 Run warning fsms, refine warnings in fly zones, collect user messages
        bool there_is_msg;
        for (it=_airspaces_of_interest.begin(); it != _airspaces_of_interest.end(); ++it) {
            there_is_msg = (*it)->FinishWarning();
            if (there_is_msg && AIRSPACEWARNINGS) {     // Pass warning messages only if warnings enabled
                  AirspaceWarningMessage msg;
                  msg.originator = *it;
                  msg.event = (*it)->WarningEvent();
                  msg.warnlevel = (*it)->WarningLevel();
                  _user_warning_queue.push_back(msg);
            }
        }

        // This is used nowhere.
        Calculated->IsInAirspace = false;
        // Give the sideview the nearest instance calculated
        _sideview_nearest = CAirspace::GetSideviewNearestInstance();

        // Fill infoboxes - Nearest horizontal
#ifndef LKAIRSP_INFOBOX_USE_SELECTED 
        if (CAirspace::GetNearestHName() != NULL) {
          LK_tcsncpy(NearestAirspaceName, CAirspace::GetNearestHName(), NAME_SIZE);
          NearestAirspaceHDist = CAirspace::GetNearestHDistance();
        } else {
          NearestAirspaceName[0]=0;
          NearestAirspaceHDist=0;
        }
        // Fill infoboxes - Nearest vertical
        if (CAirspace::GetNearestVName() != NULL) {
          LK_tcsncpy(NearestAirspaceVName, CAirspace::GetNearestVName(), NAME_SIZE);
          NearestAirspaceVDist = CAirspace::GetNearestVDistance();
        } else {
          NearestAirspaceVName[0]=0;
          NearestAirspaceVDist=0;
        }
#endif

#ifdef LKAIRSP_INFOBOX_USE_SELECTED 
        if (_selected_airspace != NULL) {
          _selected_airspace->CalculateDistance(NULL,NULL,NULL);
          LK_tcsncpy(NearestAirspaceName, _selected_airspace->Name(), NAME_SIZE);
          NearestAirspaceHDist = _selected_airspace->LastCalculatedHDistance();
          
          LK_tcsncpy(NearestAirspaceVName, _selected_airspace->Name(), NAME_SIZE);
          NearestAirspaceVDist = _selected_airspace->LastCalculatedVDistance();
        } else if (_sideview_nearest != NULL) {
          //use nearest distances, if no selection
          LK_tcsncpy(NearestAirspaceName, _sideview_nearest->Name(), NAME_SIZE);
          NearestAirspaceHDist = _sideview_nearest->LastCalculatedHDistance();
          
          LK_tcsncpy(NearestAirspaceVName, _sideview_nearest->Name(), NAME_SIZE);
          NearestAirspaceVDist = _sideview_nearest->LastCalculatedVDistance();
        } else {
          NearestAirspaceName[0]=0;
          NearestAirspaceHDist=0;
          NearestAirspaceVName[0]=0;
          NearestAirspaceVDist=0;
        }
#endif
        step = 0;
        break;
        
  } // sw step

  #ifdef DEBUG_AIRSPACE_2
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
  #if DEBUG_NEAR_POINTS
  int iCnt=0;
  StartupStore(_T("... enter SetFarVisible\n"));
  #endif
  CCriticalSection::CGuard guard(_csairspaces);
  _airspaces_near.clear();
  for (it = _airspaces.begin(); it != _airspaces.end(); ++it) {
    // Check if airspace overlaps given bounds
    if ( (msRectOverlap(&bounds_active, &((*it)->Bounds())) == MS_TRUE)
       ) {_airspaces_near.push_back(*it);
         #if DEBUG_NEAR_POINTS
         iCnt++;
         #endif
       }
  }
  #if DEBUG_NEAR_POINTS
  StartupStore(_T("... leaving SetFarVisible %i airspaces\n"),iCnt);
  #endif
}


void CAirspaceManager::CalculateScreenPositionsAirspace(const rectObj &screenbounds_latlon, const int iAirspaceMode[], const int iAirspaceBrush[], const RECT& rcDraw, const double &ResMapScaleOverDistanceModify)
{
CAirspaceList::iterator it;
//#define LKASP_CALC_ON_CHANGE_ONLY
#ifndef LKASP_CALC_ON_CHANGE_ONLY
  CCriticalSection::CGuard guard(_csairspaces);
  for (it = _airspaces_near.begin(); it!= _airspaces_near.end(); ++it) {
    (*it)->CalculateScreenPosition(screenbounds_latlon, iAirspaceMode, iAirspaceBrush, rcDraw, ResMapScaleOverDistanceModify);
  }

#else

  BOOL bChange = false;
  static double oldDistMod = -1;
  static rectObj old_screenbounds_latlon;
  POINT Pt1, Pt2;
  static POINT oldPt1 = {0,0};
  static POINT oldPt2 = {0,0};
  static RECT oldrcDraw = {0,0,0,0};
	MapWindow::LatLon2Screen(old_screenbounds_latlon.maxx   , old_screenbounds_latlon.maxy , Pt1);
	MapWindow::LatLon2Screen(old_screenbounds_latlon.minx   , old_screenbounds_latlon.miny , Pt2);

	if(abs(Pt1.x - oldPt1.x) > 2)
	  bChange = true;
	if(abs(Pt1.y - oldPt1.y) > 2)
	  bChange = true;
	if(abs(Pt2.x - oldPt2.x) > 2)
	  bChange = true;
	if(abs(Pt2.y - oldPt2.y) > 2)
	  bChange = true;

	if (oldDistMod != ResMapScaleOverDistanceModify)
	  bChange = true;

	if(oldrcDraw.bottom != rcDraw.bottom)
	  bChange = true;
	if(oldrcDraw.top    != rcDraw.top)
	  bChange = true;
	if(oldrcDraw.left   != rcDraw.left)
	  bChange = true;
	if(oldrcDraw.right  != rcDraw.right)
	  bChange = true;

	if(!bChange)
	{
	  #if DEBUG_NEAR_POINTS
//	  StartupStore(_T("... skip CalculateScreenPositionsAirspace\n"));
	  #endif
	}
	else
	{

//#if DEBUG_NEAR_POINTS
//StartupStore(_T("... CalculateScreenPositionsAirspace\n"));
//#endif
	  oldrcDraw = rcDraw;
	  oldPt1 = Pt1;
	  oldPt2 = Pt2;
	  oldDistMod = ResMapScaleOverDistanceModify;
	  old_screenbounds_latlon = screenbounds_latlon;
	  CCriticalSection::CGuard guard(_csairspaces);
	  for (it = _airspaces_near.begin(); it!= _airspaces_near.end(); ++it) {
		(*it)->CalculateScreenPosition(screenbounds_latlon, iAirspaceMode, iAirspaceBrush, rcDraw, ResMapScaleOverDistanceModify);
	  }
//StartupStore(_T("... Ready CalculateScreenPositionsAirspace\n"));
	}
#endif
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
const CAirspaceList& CAirspaceManager::GetAirspacesForWarningLabels()
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
  LKASSERT(airspace!=NULL);
  CCriticalSection::CGuard guard(_csairspaces);
  return *airspace;
}

// Calculate distances from a given airspace
bool CAirspaceManager::AirspaceCalculateDistance(CAirspace *airspace, int *hDistance, int *Bearing, int *vDistance)
{
  LKASSERT(airspace!=NULL);
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
CAirspaceList::const_iterator it;

  if(!AirspaceAckAllSame)
  {
    airspace.WarningAckLevel(ackstate);
    airspace.SetAckTimeout();
  }
  else
  {
    for (it = _airspaces.begin(); it != _airspaces.end(); ++it)
    {
      if( (*it)->IsSame(airspace))
	  {
    	(*it)->WarningAckLevel(ackstate);
    	(*it)->SetAckTimeout();
#ifdef DEBUG_AIRSPACE
StartupStore(TEXT("LKAIRSP: %s AirspaceWarnListAckForTime()%s"),(*it)->Name(),NEWLINE );
#endif
	  }
    }
  }
}

// Ack an airspace for a current level
void CAirspaceManager::AirspaceAckWarn(CAirspace &airspace)
{
CCriticalSection::CGuard guard(_csairspaces);
CAirspaceList::const_iterator it;

  if(!AirspaceAckAllSame)
  {
	airspace.WarningAckLevel(airspace.WarningLevel());
	airspace.SetAckTimeout();
  }
  else
  {
    for (it = _airspaces.begin(); it != _airspaces.end(); ++it)
    {
      if( (*it)->IsSame(airspace))
	  {
    	(*it)->WarningAckLevel(airspace.WarningLevel());
    	(*it)->SetAckTimeout();
#ifdef DEBUG_AIRSPACE
StartupStore(TEXT("LKAIRSP: %s AirspaceWarnListAck()%s"),(*it)->Name(),NEWLINE );
#endif
	  }
    }
  }
}

// Ack an airspace for all future warnings
void CAirspaceManager::AirspaceAckSpace(CAirspace &airspace)
{
CCriticalSection::CGuard guard(_csairspaces);
CAirspaceList::const_iterator it;

  if(!AirspaceAckAllSame)
  {
	airspace.WarningAckLevel(awRed);
  }
  else
  {
    for (it = _airspaces.begin(); it != _airspaces.end(); ++it)
    {
      if( (*it)->IsSame(airspace))
	  {
    	(*it)->WarningAckLevel(awRed);
#ifdef DEBUG_AIRSPACE
StartupStore(TEXT("LKAIRSP: %s AirspaceAckSpace()%s"),(*it)->Name(),NEWLINE );
#endif
	  }
    }
  }
}

// Disable an airspace 
void CAirspaceManager::AirspaceDisable(CAirspace &airspace)
{
CCriticalSection::CGuard guard(_csairspaces);
CAirspaceList::const_iterator it;

  if(!AirspaceAckAllSame)
  {
	airspace.Enabled(false);
  }
  else
  {
    for (it = _airspaces.begin(); it != _airspaces.end(); ++it)
    {
      if( (*it)->IsSame(airspace))
	  {
    	(*it)->Enabled(false);
#ifdef DEBUG_AIRSPACE
          StartupStore(TEXT("LKAIRSP: %s AirspaceDisable()%s"),(*it)->Name(),NEWLINE );
#endif
	  }
    }
  }
}

// Enable an airspace
void CAirspaceManager::AirspaceEnable(CAirspace &airspace)
{
CCriticalSection::CGuard guard(_csairspaces);
CAirspaceList::const_iterator it;

  if(!AirspaceAckAllSame)
  {
	airspace.Enabled(true);
  }
  else
  {
    for (it = _airspaces.begin(); it != _airspaces.end(); ++it)
    {
      if( (*it)->IsSame(airspace))
	  {
    	(*it)->Enabled(true);
#ifdef DEBUG_AIRSPACE
          StartupStore(TEXT("LKAIRSP: %s AirspaceEnable()%s"),(*it)->Name(),NEWLINE );
#endif
	  }
    }
  }
}

// Toggle flyzone on an airspace
void CAirspaceManager::AirspaceFlyzoneToggle(CAirspace &airspace)
{
CCriticalSection::CGuard guard(_csairspaces);
CAirspaceList::const_iterator it;

  if(!AirspaceAckAllSame)
  {
	 airspace.FlyzoneToggle();
  }
  else
  {
    for (it = _airspaces.begin(); it != _airspaces.end(); ++it)
    {
      if( (*it)->IsSame(airspace))
	  {
    	(*it)->FlyzoneToggle();
#ifdef DEBUG_AIRSPACE
          StartupStore(TEXT("LKAIRSP: %s FlyzoneToggle()%s"),(*it)->Name(),NEWLINE );
#endif
	  }
    }
  }
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
      case CLASSTMZ:
        return TEXT("TMZ");
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
    case CLASSTMZ:
        return TEXT("TMZ");
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
  LK_tcsncpy(buffer, intbuf, bufferlen-1);
}


void CAirspaceManager::GetSimpleAirspaceAltText(TCHAR *buffer, int bufferlen, const AIRSPACE_ALT *alt) const
{
  TCHAR sUnitBuffer[24];
  TCHAR intbuf[128];

  Units::FormatUserAltitude(alt->Altitude, sUnitBuffer, sizeof(sUnitBuffer)/sizeof(sUnitBuffer[0]));

  switch (alt->Base) {
    case abUndef:
        _stprintf(intbuf, TEXT("%s"), sUnitBuffer);
      break;
    case abMSL:
//       _stprintf(intbuf, TEXT("%s MSL"), sUnitBuffer);
        _stprintf(intbuf, TEXT("%s"), sUnitBuffer);
      break;
    case abAGL:
      if (alt->AGL <= 0)
        _stprintf(intbuf, TEXT("SFC"));
      else {
        Units::FormatUserAltitude(alt->AGL, sUnitBuffer, sizeof(sUnitBuffer)/sizeof(sUnitBuffer[0]));
        _stprintf(intbuf, TEXT("%s AGL"), sUnitBuffer);
      }
      break;
    case abFL:
      if (Units::GetUserAltitudeUnit() == unMeter) {
        _stprintf(intbuf, TEXT("%.0fm"), alt->Altitude);
      } else {
        _stprintf(intbuf, TEXT("%.0fft"), alt->Altitude*TOFEET);
      }
      break;
  }
  LK_tcsncpy(buffer, intbuf, bufferlen-1);
}


// Operations for nearest page 2.4
// Because of the multicalc approach, we need to store multicalc state inside airspacemanager
// in this case not need to have a notifier facility if airspace list changed during calculations
void CAirspaceManager::SelectAirspacesForPage24(const double latitude, const double longitude, const double interest_radius)
{
  double lon,lat,bearing;
  rectObj bounds;

  CCriticalSection::CGuard guard(_csairspaces);
  if (_airspaces.size()<1) return;

  // Calculate area of interest
  lon = longitude;
  lat = latitude;
  bounds.minx = lon;
  bounds.maxx = lon;
  bounds.miny = lat;
  bounds.maxy = lat;

  bearing = 0;
  {
    FindLatitudeLongitude(latitude, longitude, bearing, interest_radius, &lat, &lon);
    bounds.minx = min(lon, bounds.minx);
    bounds.maxx = max(lon, bounds.maxx);
    bounds.miny = min(lat, bounds.miny);
    bounds.maxy = max(lat, bounds.maxy);
  }
  bearing = 90;
  {
    FindLatitudeLongitude(latitude, longitude, bearing, interest_radius, &lat, &lon);
    bounds.minx = min(lon, bounds.minx);
    bounds.maxx = max(lon, bounds.maxx);
    bounds.miny = min(lat, bounds.miny);
    bounds.maxy = max(lat, bounds.maxy);
  }
  bearing = 180;
  {
    FindLatitudeLongitude(latitude, longitude, bearing, interest_radius, &lat, &lon);
    bounds.minx = min(lon, bounds.minx);
    bounds.maxx = max(lon, bounds.maxx);
    bounds.miny = min(lat, bounds.miny);
    bounds.maxy = max(lat, bounds.maxy);
  }
  bearing = 270;
  {
    FindLatitudeLongitude(latitude, longitude, bearing, interest_radius, &lat, &lon);
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

  // Select nearest ones (based on bounds)
  _airspaces_page24.clear();
  for (CAirspaceList::iterator it = _airspaces.begin(); it != _airspaces.end(); ++it) {
    if (msRectOverlap(&bounds, &(*it)->Bounds()) == MS_TRUE) _airspaces_page24.push_back(*it);
  }
}


void CAirspaceManager::CalculateDistancesForPage24()
{
  CCriticalSection::CGuard guard(_csairspaces);
  for (CAirspaceList::iterator it = _airspaces_page24.begin(); it != _airspaces_page24.end(); ++it) {
    (*it)->CalculateDistance(NULL, NULL, NULL);
  }
}

CAirspaceList CAirspaceManager::GetAirspacesForPage24()
{
  CCriticalSection::CGuard guard(_csairspaces);
  return _airspaces_page24;
}

// Set or change or deselect selected airspace
void CAirspaceManager::AirspaceSetSelect(CAirspace &airspace)
{
  CCriticalSection::CGuard guard(_csairspaces);
  // Deselect if we get the same asp
  if (_selected_airspace == &airspace) {
    _selected_airspace->Selected(false);
    _selected_airspace = NULL;
    return;
  }
  
  if (_selected_airspace != NULL) _selected_airspace->Selected(false);
  _selected_airspace = &airspace;
  if (_selected_airspace != NULL) _selected_airspace->Selected(true);
}

// Save airspace settings
void CAirspaceManager::SaveSettings() const
{
  char hashbuf[33];
  FILE *f;
  TCHAR szFileName[MAX_PATH];
  char buf[MAX_PATH+1];
  TCHAR ubuf[(MAX_PATH*2)+1];

  LocalPath(szFileName, TEXT(LKF_AIRSPACE_SETTINGS));
  f=_tfopen(szFileName, TEXT("w"));
  if (f!=NULL) {  
    // File header
    fprintf(f,"# LK8000 AIRSPACE SETTINGS\n");
    fprintf(f,"# THIS FILE IS GENERATED AUTOMATICALLY ON LK SHUTDOWN - DO NOT ALTER BY HAND, DO NOT COPY BEETWEEN DEVICES!\n");
    
    CCriticalSection::CGuard guard(_csairspaces);
    for (CAirspaceList::const_iterator it = _airspaces.begin(); it != _airspaces.end(); ++it) {
      (*it)->Hash(hashbuf,33);
      //Asp hash
      fprintf(f,"%32s ",hashbuf);
      
      //Settings chr1 - Flyzone or not
      if ((*it)->Flyzone()) fprintf(f,"F"); else fprintf(f,"-"); 
      //Settings chr2 - Enabled or not
      if ((*it)->Enabled()) fprintf(f,"E"); else fprintf(f,"-"); 
      //Settings chr3 - Selected or not
      if ((*it)->Selected()) fprintf(f,"S"); else fprintf(f,"-"); 
      
      //Comment
      wsprintf(ubuf,TEXT(" #%s"),(*it)->Name());
      unicode2utf(ubuf,buf,sizeof(buf));
      fprintf(f,"%s",buf); 
      //Newline
      fprintf(f,"\n");
    }
    StartupStore(TEXT(". Settings for %d airspaces saved to file <%s>%s"), _airspaces.size(), szFileName, NEWLINE);
    fclose(f);
  } else StartupStore(TEXT("Failed to save airspace settings to file <%s>%s"),szFileName,NEWLINE);
}

// Load airspace settings
void CAirspaceManager::LoadSettings()
{
  char linebuf[MAX_PATH+1];
  char hash[MAX_PATH+1];
  char flagstr[MAX_PATH+1];
  FILE *f;
  TCHAR szFileName[MAX_PATH];
  typedef struct _asp_data_struct {
    CAirspace* airspace;
    char hash[33];
  } asp_data_struct;
  asp_data_struct *asp_data;
  unsigned int i,retval;
  unsigned int airspaces_restored = 0;
    
  LocalPath(szFileName, TEXT(LKF_AIRSPACE_SETTINGS));
  f=_tfopen(szFileName, TEXT("r"));
  if (f!=NULL) {  
    // Generate hash map on loaded airspaces
    CCriticalSection::CGuard guard(_csairspaces);
    asp_data = (asp_data_struct*)malloc(sizeof(asp_data_struct) * _airspaces.size());
    if (asp_data==NULL) {
      OutOfMemory(__FILE__,__LINE__);
      fclose(f);
      return;
    }
    i = 0;
    for (CAirspaceList::iterator it = _airspaces.begin(); it != _airspaces.end(); ++it, ++i) {
      (*it)->Hash(asp_data[i].hash,33);
      asp_data[i].airspace = *it;
    }

    while (fgets(linebuf, MAX_PATH, f) != NULL) {
      //Parse next line
      retval = sscanf(linebuf,"%s %s",hash,flagstr);
      if (retval==2 && hash[0]!='#') {
        // Get the airspace pointer associated with the hash
        for (i=0; i<_airspaces.size(); ++i) {
          if (asp_data[i].airspace==NULL) continue;
          if (strcmp(hash,asp_data[i].hash) == 0) {
            //Match, restore settings
            //chr1 F=Flyzone
            if (flagstr[0]=='F') {
              if (!asp_data[i].airspace->Flyzone()) asp_data[i].airspace->FlyzoneToggle();
            } else {
              if (asp_data[i].airspace->Flyzone()) asp_data[i].airspace->FlyzoneToggle();
            }
            //chr2 E=Enabled
            if (flagstr[1]=='E') asp_data[i].airspace->Enabled(true); else asp_data[i].airspace->Enabled(false);
            //chr3 S=Selected
            if (flagstr[2]=='S') AirspaceSetSelect(*(asp_data[i].airspace));
            
            // This line is readed, never needed anymore
            //StartupStore(TEXT(". Airspace settings loaded for %s%s"),asp_data[i].airspace->Name(),NEWLINE);
            asp_data[i].airspace = NULL;
            airspaces_restored++;
          }
        }
      }
    }
    
    if (asp_data) free(asp_data);
    StartupStore(TEXT(". Settings for %d of %d airspaces loaded from file <%s>%s"), airspaces_restored, _airspaces.size(), szFileName, NEWLINE);
    fclose(f);
  } else StartupStore(TEXT(". Failed to load airspace settings from file <%s>%s"),szFileName,NEWLINE);
}


#if ASPWAVEOFF
void CAirspaceManager::AirspaceDisableWaveSectors(void)
{
CCriticalSection::CGuard guard(_csairspaces);
CAirspaceList::const_iterator it;


    for (it = _airspaces.begin(); it != _airspaces.end(); ++it)
    {
      if( (*it)->Type()  == WAVE )
	  {
    	(*it)->Enabled(false);
    	(*it)->Flyzone(true);
#ifdef DEBUG_AIRSPACE
          StartupStore(TEXT("LKAIRSP: %s AirspaceDisable()%s"),(*it)->Name(),NEWLINE );
#endif
	  }
    }

}
#endif

