/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

 */

#include "externs.h"
#include "LKAirspace.h"
#include "RasterTerrain.h"
#include "LKProfiles.h"
#include "Dialogs.h"
#include <ctype.h>
#include <utility>
#include <regex>

#include <Point2D.h>
#include "md5.h"
#include "LKObjects.h"

#include "utils/2dpclip.h"
#include "utils/stringext.h"
#include "Draw/ScreenProjection.h"
#include "NavFunctions.h"
#include "Util/TruncateString.hpp"

#include "Topology/shapelib/mapserver.h"
#include "utils/zzip_stream.h"
#include "picojson.h"
#include "Radio.h"
#include "Library/rapidxml/rapidxml.hpp"
#include "utils/tokenizer.h"
#include "utils/printf.h"
#include "Library/TimeFunctions.h"
#include "Baro.h"
#include "utils/lookup_table.h"
#include "LocalPath.h"
#include "InputEvents.h"
#include "utils/charset_helper.h"

using xml_document = rapidxml::xml_document<char>;
using xml_attribute = rapidxml::xml_attribute<char>;

#ifdef _WGS84
#include <GeographicLib/GeodesicLine.hpp>

using GeographicLib::GeodesicLine;
#endif

#define MIN_AS_SIZE 3  // minimum number of point for a valid airspace

unsigned int OutsideAirspaceCnt =0;

namespace {

struct start_with_predicate final {
    bool operator()(tstring_view text, tstring_view prefix) {
        return text.substr(0, prefix.size()) == prefix;
    }
};

auto type_table = lookup_table<tstring_view, int, start_with_predicate>({
    { _T("R"), RESTRICT },
    { _T("Q"), DANGER },
    { _T("P"), PROHIBITED },
    { _T("A"), CLASSA },
    { _T("B"), CLASSB },
    { _T("C"), CLASSC },
    { _T("D"), CLASSD },
    { _T("W"), WAVE },
    { _T("E"), CLASSE },
    { _T("F"), CLASSF },
    { _T("G"), CLASSG },
    { _T("GP"), NOGLIDER },
    { _T("CTR"), CTR },
    { _T("TMZ"), CLASSTMZ },
    { _T("RMZ"), CLASSRMZ },
    { _T("NOTAM"), CLASSNOTAM },
    { _T("GSEC"), GLIDERSECT }
});

/**
 * Update Airspace type from OpenAir Fields
 * @c type from AC Field
 * @y type from AY Field
 */
int update_type( int c, int y) {
    if (c == OTHER) {
        return y;
    }
    return c;
}

} //namespace

// CAirspace class attributes
#ifndef LKAIRSP_INFOBOX_USE_SELECTED
int CAirspace::_nearesthdistance = 0; // for infobox
int CAirspace::_nearestvdistance = 0; // for infobox
TCHAR* CAirspace::_nearesthname = NULL; // for infobox
TCHAR* CAirspace::_nearestvname = NULL; // for infobox
#endif
bool CAirspaceBase::_pos_in_flyzone = false; // for refine warnings in flyzones
bool CAirspaceBase::_pred_in_flyzone = false; // for refine warnings in flyzones
bool CAirspaceBase::_pos_in_acked_nonfly_zone = false; // for refine warnings in flyzones
bool CAirspaceBase::_pred_in_acked_nonfly_zone = false; // for refine warnings in flyzones
int CAirspaceBase::_now = 0; // gps time saved
int CAirspaceBase::_hdistancemargin = 0; // calculated horizontal distance margin to use
CPoint2D CAirspaceBase::_lastknownpos(0, 0); // last known position saved for calculations
int CAirspaceBase::_lastknownalt = 0; // last known alt saved for calculations
int CAirspaceBase::_lastknownagl = 0; // last known agl saved for calculations
int CAirspaceBase::_lastknownheading = 0; // last known heading saved for calculations
int CAirspaceBase::_lastknowntrackbearing = 0; // last known track bearing saved for calculations
bool CAirspaceBase::_pred_blindtime = true; // disable predicted position based warnings near takeoff, and other conditions
CAirspaceWeakPtr CAirspace::_sideview_nearest_instance; // collect nearest airspace instance for sideview during warning calculations

//
// CAIRSPACE CLASS
//

const TCHAR* CAirspaceBase::TypeName() const {
  return CAirspaceManager::GetAirspaceTypeText(_type);
}

const TCHAR* CAirspaceBase::TypeNameShort() const {
  return CAirspaceManager::GetAirspaceTypeShortText(_type);
}

const LKColor& CAirspaceBase::TypeColor() const {
  return MapWindow::GetAirspaceColourByClass(_type);
}

const LKBrush& CAirspaceBase::TypeBrush() const {
#ifdef HAVE_HATCHED_BRUSH
  return MapWindow::GetAirspaceBrushByClass(_type);
#else
  return MapWindow::GetAirSpaceSldBrushByClass(_type);
#endif
}

void CAirspaceBase::AirspaceAGLLookup(double av_lat, double av_lon, double *basealt_out, double *topalt_out) const {
    double th = 0.; 
    if (_floor.agl() || _ceiling.agl()) {
        th = WithLock(RasterTerrain::mutex, [&]() {
            // want most accurate rounding here
            RasterTerrain::SetTerrainRounding(0, 0);
            return RasterTerrain::GetTerrainHeight(av_lat, av_lon);
        });
    }
    if (th == TERRAIN_INVALID) {
      // 101027 We still use 0 altitude for no terrain, what else can we do..
      th = 0;
    }

    if (basealt_out) {
        *basealt_out = _floor.altitude(th);
    }
    if (topalt_out) {
        *topalt_out = _ceiling.altitude(th);
    }
}

// Called when QNH changed

void CAirspaceBase::QnhChangeNotify() {
  _ceiling.qnh_update();
  _floor.qnh_update();
}

inline bool CheckInsideLongitude(const double &longitude, const double &lon_min, const double &lon_max) {
    if (lon_min <= lon_max) {
        // normal case
        return ((longitude > lon_min) && (longitude < lon_max));
    } else {
        // area goes across 180 degree boundary, so lon_min is +ve, lon_max is -ve (flipped)
        return ((longitude > lon_min) || (longitude < lon_max));
    }
}

// returns true if the given altitude inside this airspace + alt extension
//  (below the ceiling and above the floor)
bool CAirspaceBase::IsAltitudeInside(int alt, int agl, int extension) const {
  return _ceiling.below(alt + extension, agl + extension) && _floor.above(alt - extension, agl - extension);
}

// Step1:
// warning calculation, set initial states, etc.

void CAirspace::StartWarningCalculation(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
    _pos_in_flyzone = false;
    _pred_in_flyzone = false;
    _pos_in_acked_nonfly_zone = false;
    _pred_in_acked_nonfly_zone = false;

#ifndef LKAIRSP_INFOBOX_USE_SELECTED
    _nearesthname = NULL;
    _nearestvname = NULL;
    _nearesthdistance = 100000;
    _nearestvdistance = 100000;
#endif

    _sideview_nearest_instance.reset(); // Init nearest instance for sideview

    // 110518 PENDING_QUESTION
    // From Paolo to Kalman: casting a double to a signed int won't create problems
    // if for any reason it overflows the positive sign, going negative?
    // Kalman: overflow occurs after 24855days (68years) runtime, i think it will not cause problems.
    _now = (int) Basic->Time;

    //Save position for further calculations made by gui threads
    _lastknownalt = Calculated->NavAltitude;
    _lastknownagl = Calculated->AltitudeAGL;
    if (_lastknownagl < 0) _lastknownagl = 0; // Limit agl to zero
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
    if ((!Calculated->Flying) || ((!SIMMODE)&&((Basic->Time - Calculated->TakeOffTime) < 60))) _pred_blindtime = true;
    // When we are inside dlgConfiguration, NO AIRSPACE WARNINGS!
    if (MenuActive) _pred_blindtime = true;
}

// Step2: first pass on all airspace instances
// Calculate warnlevel based on last/now/next position

void CAirspace::CalculateWarning(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
    _warnevent = aweNone;

    //Check actual position
    _pos_inside_now = false;
    int alt = Calculated->NavAltitude;
    int agl = Calculated->AltitudeAGL;
    if (agl < 0) agl = 0; // Limit actual altitude to surface to not get warnings if close to ground

    // Calculate distances
    CalculateDistance(NULL, NULL, NULL);
    if (_hdistance <= 0) {
        _pos_inside_now = true;
    }
    // Check for altitude
    bool pos_altitude = IsAltitudeInside(alt, agl);
    if (!pos_altitude) _pos_inside_now = false;

#ifndef LKAIRSP_INFOBOX_USE_SELECTED
    if (_flyzone && _pos_inside_now) {
        // If in flyzone, nearest warning point given (nearest distance to leaving the fly zone)
        if (abs(_hdistance) < abs(_nearesthdistance)) {
            _nearesthname = _name;
            _nearesthdistance = abs(_hdistance);
        }
        if (abs(_vdistance) < abs(_nearestvdistance)) {
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
            if ((abs(_hdistance) < abs(_nearesthdistance)) && (_hdistance > 0) && IsAltitudeInside(alt, agl, AirspaceWarningVerticalMargin / 10)) {
                _nearesthname = _name;
                _nearesthdistance = abs(_hdistance);
            }
            // Just directly above or below distances counts
            if ((abs(_vdistance) < abs(_nearestvdistance)) && (_hdistance < 0)) {
                _nearestvname = _name;
                _nearestvdistance = _vdistance;
            }
        }
    }
#endif

    auto nearest = _sideview_nearest_instance.lock();
    if (!nearest) {
        _sideview_nearest_instance = shared_from_this();
    } else {
        if (_3ddistance > 0) {
            if (_3ddistance < nearest->_3ddistance) {
                _sideview_nearest_instance = shared_from_this();
            }
        }
    }

    // We have to calculate with the predicted position
    bool pred_inside_now = false;
    alt = (int) Calculated->NextAltitude;
    agl = (int) Calculated->NextAltitudeAGL;
    if (agl < 0) agl = 0; // Limit predicted agl to surface
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

bool CAirspace::FinishWarning() {
    bool res = false;
    int abs_hdistance = abs(_hdistance);
    int abs_vdistance = abs(_vdistance);
    int hdistance_histeresis = 500; // Horizontal distance histeresis to step back awNone
    int vdistance_histeresis = 20; // Vertical distance histeresis to step back awNone
    int hdistance_lookout = 200; // Horizontal distance to lookout from a flyzone to check what is outside
    int vdistance_lookout = 20; // Vertical distance to lookout from a flyzone to check what is outside
    int abs_beardiff = abs((int) AngleLimit180(_lastknownheading - _bearing));

    //Calculate warning state based on airspace warning events
    switch (_warnevent) {
        default:
            break;

            // Events for FLY zones
        case aweMovingInsideFly:
            // If far away from border, set warnlevel to none
            // If base is sfc, we skip near warnings to base, to not get disturbing messages on landing.
            if ((abs_hdistance > (_hdistancemargin + hdistance_histeresis)) &&
                    (abs_vdistance > ((AirspaceWarningVerticalMargin / 10) + vdistance_histeresis))
                    ) {
                // Far away horizontally _and_ vertically
                _warninglevel = awNone;
                _hwarninglabel_hide = false;
                _vwarninglabel_hide = false;
                break;
            }
            _hwarninglabel_hide = true;
            if (abs_hdistance < _hdistancemargin) {
                // Check what is outside this flyzone. If another flyzone or acked nonfly zone, then we don't have to increase the warn state
                double lon = 0;
                double lat = 0;
                double dist = abs(_hdistance) + hdistance_lookout;
                FindLatitudeLongitude(_lastknownpos.Latitude(), _lastknownpos.Longitude(), _bearing, dist, &lat, &lon);

                if (!CAirspaceManager::Instance().AirspaceWarningIsGoodPosition(lon, lat, _lastknownalt, _lastknownagl)) {
                    // Near to outside, modify warnevent to inform user
                    _warninglevel = awYellow;
                    _warnevent = aweNearOutsideFly;
                    _hwarninglabel_hide = false;
                }
            }

            _vwarninglabel_hide = true;
            if (abs_vdistance < (AirspaceWarningVerticalMargin / 10)) {
                // Check what is outside vertically this flyzone. If another flyzone or acked nonfly zone, then we don't have to increase the warn state
                int alt = _lastknownalt;
                int agl = _lastknownagl;
                if (_vdistance < 0) {
                    // adjacent airspace will be above this one
                    alt += abs_vdistance + vdistance_lookout;
                    agl += abs_vdistance + vdistance_lookout;
                } else {
                    // adjacent airspace will be below this one
                    alt -= abs_vdistance + vdistance_lookout;
                    agl -= abs_vdistance + vdistance_lookout;
                }
                if (agl < 0) agl = 0;

                if (!CAirspaceManager::Instance().AirspaceWarningIsGoodPosition(_lastknownpos.Longitude(), _lastknownpos.Latitude(), alt, agl)) {
                    // Near to outside, modify warnevent to inform user
                    _warninglevel = awYellow;
                    _warnevent = aweNearOutsideFly;
                    _vwarninglabel_hide = false;
                }
            }
            break;

        case awePredictedLeavingFly:
            if (_pred_blindtime) break; //Do not count predicted events near takeoff, filters not settled yet
            if (!(_pred_in_flyzone || _pred_in_acked_nonfly_zone)) {
                // if predicted position not in other fly or acked nonfly zone, then leaving this one should be wrong
                _warninglevel = awYellow;
            }
            break;

        case aweLeavingFly:
            if (_pred_blindtime) break; //Do not count predicted events near takeoff, filters not settled yet
            if (!(_pos_in_flyzone || _pos_in_acked_nonfly_zone)) {
                // if current position not in other fly or acked nonfly zone, then leaving this one should be wrong
                _warninglevel = awRed;
            }
            break;

        case awePredictedEnteringFly:
            break;

        case aweEnteringFly:
            if (_pred_blindtime) break; //Do not count predicted events near takeoff, filters not settled yet
            // Also preset warnlevel to awYellow, because we entering yellow zone.
            // but we don't need to generate a warning message right now - force no change in warnlevel
            _hwarninglabel_hide = true;
            if (abs_hdistance < _hdistancemargin) {
                // Check what is outside this flyzone. If another flyzone or acked nonfly zone, then we don't have to increase the warn state
                double lon = 0;
                double lat = 0;
                double dist = abs(_hdistance) + hdistance_lookout;
                FindLatitudeLongitude(_lastknownpos.Latitude(), _lastknownpos.Longitude(), _bearing, dist, &lat, &lon);

                if (!CAirspaceManager::Instance().AirspaceWarningIsGoodPosition(lon, lat, _lastknownalt, _lastknownagl)) {
                    _warninglevelold = _warninglevel = awYellow;
                    _hwarninglabel_hide = false;
                }
            }

            _vwarninglabel_hide = true;
            if (abs_vdistance < (AirspaceWarningVerticalMargin / 10)) {
                // Check what is outside vertically this flyzone. If another flyzone or acked nonfly zone, then we don't have to increase the warn state
                int alt = _lastknownalt;
                int agl = _lastknownagl;
                if (_vdistance < 0) {
                    // adjacent airspace will be above this one
                    alt += abs_vdistance + vdistance_lookout;
                    agl += abs_vdistance + vdistance_lookout;
                } else {
                    // adjacent airspace will be below this one
                    alt -= abs_vdistance + vdistance_lookout;
                    agl -= abs_vdistance + vdistance_lookout;
                }
                if (agl < 0) agl = 0;

                if (!CAirspaceManager::Instance().AirspaceWarningIsGoodPosition(_lastknownpos.Longitude(), _lastknownpos.Latitude(), alt, agl)) {
                    _warninglevelold = _warninglevel = awYellow;
                    _vwarninglabel_hide = false;
                }
            }
            // Do info message on entering a fly zone
            res = true;
            break;

        case aweMovingOutsideFly:
            // if outside, but in good zone, then this one is good as well
            if ((_pos_in_flyzone || _pos_in_acked_nonfly_zone)) _warninglevel = awNone;
            break;


            // Events for NON-FLY zones
        case aweMovingOutsideNonfly:
            if (_pred_blindtime) break; //Do not count predicted events near takeoff, filters not settled yet
            if ((_hdistance > (_hdistancemargin + hdistance_histeresis)) ||
                    (!IsAltitudeInside(_lastknownalt, _lastknownagl, (AirspaceWarningVerticalMargin / 10) + vdistance_histeresis))
                    ) {
                // Far away horizontally _or_ vertically
                _warninglevel = awNone;
            }
            if ((_hdistance < _hdistancemargin) && (abs_beardiff <= 90)) {
                if (IsAltitudeInside(_lastknownalt, _lastknownagl, (AirspaceWarningVerticalMargin / 10))) {
                    // Near to inside and moving closer, modify warnevent to inform user
                    _warninglevel = awYellow;
                    _warnevent = aweNearInsideNonfly;
                }
            }
            break;

        case awePredictedEnteringNonfly:
            if (_pred_blindtime) break; //Do not count predicted events near takeoff, filters not settled yet
            _warninglevel = awYellow;
            break;

        case aweEnteringNonfly:
            if (_pred_blindtime) break; //Do not count predicted events near takeoff, filters not settled yet
            _warninglevel = awRed;
            break;

        case aweMovingInsideNonfly:
            if (_pred_blindtime) break; //Do not count predicted events near takeoff, filters not settled yet
            _warninglevel = awRed;
            break;

        case aweLeavingNonFly:
            if (_pred_blindtime) break; //Do not count predicted events near takeoff, filters not settled yet
            _warninglevel = awYellow;
            // Do info message on leaving a nonfly zone
            res = true;
            break;
    }//sw warnevent
    _warneventold = _warnevent;

    // Warnstate increased above ack state -> generate message
    if ((_warninglevel > _warninglevelold) && (_warninglevel > _warningacklevel)) {
        _warn_repeat_time = _now + AirspaceWarningRepeatTime;
        res = true;
    }

    // Unacknowledged warning repeated after some time
    if ((_warninglevel > _warningacklevel) && (_now > _warn_repeat_time)) {
        _warn_repeat_time = _now + AirspaceWarningRepeatTime;
        res = true;
    }

    //ACK Step back, if ack time ellapsed and warningstate below ack state
    if ((_warningacklevel > _warninglevel) && (_now > _warn_ack_timeout)) _warningacklevel = _warninglevel;

    _warninglevelold = _warninglevel;

    return res;
}

// Set ack timeout to configured value

void CAirspaceBase::SetAckTimeout() {
    _warn_ack_timeout = _now + AcknowledgementTime;
}

bool  CAirspaceBase::Acknowledged(void) const {
	if (_warningacklevel >  awNone)
		return true;
	else
		return false;
}
// Gets calculated distances, returns true if distances valid

bool CAirspaceBase::GetDistanceInfo(bool &inside, int &hDistance, int &Bearing, int &vDistance) const {
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

bool CAirspaceBase::GetWarningPoint(double &longitude, double &latitude, AirspaceWarningDrawStyle_t &hdrawstyle, int &vDistance, AirspaceWarningDrawStyle_t &vdrawstyle) const {
    if (_distances_ready && _enabled) {
        if (_flyzone && !_pos_inside_now) return false; // no warning labels if outside a flyzone

        double dist = abs(_hdistance);
        double basealt, topalt;
        FindLatitudeLongitude(_lastknownpos.Latitude(), _lastknownpos.Longitude(), _bearing, dist, &latitude, &longitude);
        AirspaceAGLLookup(latitude, longitude, &basealt, &topalt);

        vdrawstyle = awsBlack;
        if ((_lastknownalt >= basealt) && (_lastknownalt < topalt)) {
            if (!_flyzone) vdrawstyle = awsRed;
        } else {
            if (_flyzone) vdrawstyle = awsAmber;
        }
        hdrawstyle = vdrawstyle;

        vDistance = _vdistance;
        //if (abs(_vdistance) > (AirspaceWarningVerticalMargin/10)) vdrawstyle = awsHidden;

        // Nofly zones
        if (!_flyzone && (_hdistance < 0)) hdrawstyle = awsHidden; // No horizontal warning label if directly below or above
        if (!_flyzone && (_hdistance > 0)) vdrawstyle = awsHidden; // No vertical warning label if outside horizontally

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
bool CAirspaceBase::IsSame(CAirspaceBase &as2) const {
    bool ret = false;
    if (_type == as2.Type()) {
        if (_tcscmp((_name), (as2.Name())) == 0) {
            ret = true;
        }
    }
    return ret;
}

// Calculates nearest horizontal, vertical and 3d distance to airspace based on last known position
// Returns true if inside, false if outside

bool CAirspace::CalculateDistance(int *hDistance, int *Bearing, int *vDistance, double Longitude, double Latitude, int Altitude) {
    bool inside = true;
    double vDistanceBase;
    double vDistanceTop;
    double fbearing;
    double distance;

    distance = Range(Longitude, Latitude, fbearing);
    if (distance > 0) {
        inside = false;
        // if outside we need the terrain height at the intersection point
        double intersect_lat, intersect_lon;
        FindLatitudeLongitude(Latitude, Longitude, fbearing, distance, &intersect_lat, &intersect_lon);
        AirspaceAGLLookup(intersect_lat, intersect_lon, &vDistanceBase, &vDistanceTop);
    } else {
        // if inside we need the terrain height at the current position
        AirspaceAGLLookup(Latitude, Longitude, &vDistanceBase, &vDistanceTop);
    }
    vDistanceBase = Altitude - vDistanceBase;
    vDistanceTop = Altitude - vDistanceTop;

    if (vDistanceBase < 0 || vDistanceTop > 0) inside = false;

    _bearing = (int) fbearing;
    _hdistance = (int) distance;
    if ((-vDistanceBase > vDistanceTop) && _floor.agl())
        _vdistance = vDistanceBase;
    else
        _vdistance = vDistanceTop;

    // 3d distance calculation
    if (_hdistance > 0) {
        //outside horizontally
        if (vDistanceBase < 0 || vDistanceTop > 0) {
            //outside vertically
            _3ddistance = (int) sqrt(distance * distance + (double) _vdistance * (double) _vdistance);
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
            if (abs(_vdistance) < abs(_hdistance)) _3ddistance = -abs(_vdistance);
            else _3ddistance = _hdistance;
        }
    }

    if (Bearing) *Bearing = _bearing;
    if (hDistance) *hDistance = _hdistance;
    if (vDistance) *vDistance = _vdistance;
    _distances_ready = true;
    return inside;
}


bool CAirspace::CheckVisible() const {
    if (AltitudeMode == ALLON) {
        return true;
    } else if (AltitudeMode == ALLOFF) {
        return false;
    }

    LockFlightData();
    double alt = CALCULATED_INFO.NavAltitude;
    double alt_agl = CALCULATED_INFO.TerrainAlt;
    UnlockFlightData();

    double basealt = _floor.altitude(alt_agl);
    double topalt = _ceiling.altitude(alt_agl);
    double base_is_sfc = _floor.sfc();

    switch (AltitudeMode) {
        case ALLON: return true;

        case CLIP:
            if ((basealt < (ClipAltitude / 10)) || base_is_sfc) return true;
            else return false;

        case AUTO:
            if (((alt > (basealt - (AltWarningMargin / 10))) || base_is_sfc)
                    && (alt < (topalt + (AltWarningMargin / 10))))
                return true;
            else
                return false;

        case ALLBELOW:
            if (((basealt - (AltWarningMargin / 10)) < alt) || base_is_sfc)
                return true;
            else
                return false;
        case INSIDE:
            if (((alt >= basealt) || base_is_sfc) && (alt < topalt))
                return true;
            else
                return false;
        case ALLOFF: return false;
    }
    return true;
}

// Reset warnings, if airspace outside calculation scope

void CAirspaceBase::ResetWarnings() {
    _warninglevel = awNone;
    _warninglevelold = awNone;
    _distances_ready = false;
}

// Initialize instance attributes
void CAirspaceBase::Init(const TCHAR *name, int type, vertical_bound &&base, vertical_bound &&top, bool flyzone, const TCHAR *comment) {
    lk::strcpy(_name, name);
    if (_tcslen(_name) <  _tcslen(name)) {
        _comment = name;
    }
    else {
      _comment.clear();
    }

    // always allocate string to avoid unchecked nullptr exception
    if (comment) {
        if (!_comment.empty()) {
            _comment += _T("\n");
        }
        _comment += comment;
    }

    _type = type;
    _flyzone = flyzone;
    _floor = base;
    _ceiling = top;
}

//
// CAIRSPACE_CIRCLE CLASS
//

CAirspace_Circle::CAirspace_Circle(const GeoPoint &Center, const double Radius) 
    : CAirspace(), _center(Center), _radius(Radius) 
{

    _bounds.minx = _center.longitude;
    _bounds.maxx = _center.longitude;
    _bounds.miny = _center.latitude;
    _bounds.maxy = _center.latitude;

    _geopoints.reserve(90);
    for (unsigned i = 0; i < 90; ++i) {
        GeoPoint pt = _center.Direct(static_cast<double> (i*4), _radius);

        _bounds.minx = std::min(pt.longitude, _bounds.minx);
        _bounds.maxx = std::max(pt.longitude, _bounds.maxx);
        _bounds.miny = std::min(pt.latitude, _bounds.miny);
        _bounds.maxy = std::max(pt.latitude, _bounds.maxy);

        _geopoints.emplace_back(pt.latitude, pt.longitude);
    }
}

// Calculate unique hash code for this airspace
std::string CAirspace_Circle::Hash() const {
    MD5 md5;

    CAirspace::Hash(md5);

    md5.Update(_center.latitude);
    md5.Update(_center.longitude);
    md5.Update(_radius);

    return md5.Final();
}

// Check if the given coordinate is inside the airspace

bool CAirspace_Circle::IsHorizontalInside(const double &longitude, const double &latitude) const {
    double bearing;
    if ((latitude > _bounds.miny) &&
            (latitude < _bounds.maxy) &&
            CheckInsideLongitude(longitude, _bounds.minx, _bounds.maxx)
            ) {
        if (Range(longitude, latitude, bearing) < 0) {
            return true;
        }
    }
    return false;
}

// Calculate horizontal distance from a given point

double CAirspace_Circle::Range(const double &longitude, const double &latitude, double &bearing) const {
    double distance;
    DistanceBearing(latitude, longitude,
            _center.latitude,
            _center.longitude,
            &distance, &bearing);
    distance -= _radius;
    if (distance < 0) {
        bearing = AngleLimit360(bearing + 180);
    }
    return distance;
}



// Calculate screen coordinates for drawing
void CAirspace::CalculateScreenPosition(const rectObj &screenbounds_latlon, const airspace_mode_array& aAirspaceMode, const int iAirspaceBrush[], const RECT& rcDraw, const ScreenProjection& _Proj) {

    /** TODO 
     *   check map projection change
     *   move CAirspace::CheckVisible() outside draw function
     */
    
    
    /** optimized pseudo code
     * 
     * if ( Map projection has Changed ) {
     *    - clear screen coordinate cache.
     * }
     * 
     * if (airspace is visible and _screenpoints is empty ) {
     *    - calculate screen position
     *    if ( screen bounds does not include aispace bounds ) {
     *      - clip screen coordinate to rcDraw
     *    }
     *    - set drawing style
     * }
     */

    _drawstyle = adsHidden;        

    // Check Visibility : faster first
    bool is_visible = aAirspaceMode[_type].display(); // airspace class disabled ?
    if(is_visible) {
      is_visible = !_ceiling.below_msl();
    }
    if(is_visible) { // no need to msRectOverlap if airspace is not visible
        is_visible = msRectOverlap(&_bounds, &screenbounds_latlon);
    }
    if(is_visible) { // no need to check Altitude if airspace is not visible
        // TODO : "CheckVisible() lock Flight data for altitude : to slow, need to change"
        is_visible = CheckVisible();
    }
    
    if(is_visible) { 
        bool need_clipping = !msRectContained(&_bounds, &screenbounds_latlon);

        if (!rendrer) {
            rendrer = std::make_unique<AirspaceRenderer>();
        }
        rendrer->Update(_geopoints, need_clipping, rcDraw, _Proj);
    }
    else {
        rendrer = nullptr;
    }

    if ( (((!(iAirspaceBrush[_type] == NUMAIRSPACEBRUSHES - 1)) && (_warninglevel == awNone) && (_warningacklevel ==  awNone))|| (_warninglevel > _warningacklevel)/*(_warningacklevel == awNone)*/)) {
        _drawstyle = adsFilled;
    } else {
        _drawstyle = adsOutline;
        //    _drawstyle = adsFilled;
    }
    if (!_enabled) {
      _drawstyle = adsDisabled;
    }

}

void CAirspace::DrawOutline(LKSurface& Surface, PenReference pen) const {
    if (rendrer) {
        rendrer->DrawOutline(Surface, pen);
    }
}

void CAirspace::FillPolygon(LKSurface& Surface, const LKBrush& brush) const {
    if (rendrer) {
        rendrer->FillPolygon(Surface, brush);
    }
}


void CAirspace::Hash(MD5& md5) const {
    md5.Update(_type);
    md5.Update(to_utf8(_name));
    _floor.hash(md5);
    _ceiling.hash(md5);
}

//
// CAIRSPACE AREA CLASS
//
CAirspace_Area::CAirspace_Area(CPoint2DArray &&Area_Points) 
    : CAirspace(std::forward<CPoint2DArray>(Area_Points))
{
    CalcBounds();
}

// Calculate unique hash code for this airspace
std::string CAirspace_Area::Hash() const {
    MD5 md5;

    CAirspace::Hash(md5);

    for (const auto& point : _geopoints) {
        md5.Update(point.Latitude());
        md5.Update(point.Longitude());
    }

    return md5.Final();
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
isLeft(const CPoint2D &P0, const CPoint2D &P1, const double &longitude, const double &latitude) {
    return ( (P1.Longitude() - P0.Longitude()) * (latitude - P0.Latitude())
            - (longitude - P0.Longitude()) * (P1.Latitude() - P0.Latitude()));
}

// wn_PnPoly(): winding number test for a point in a polygon
//      Input:   P = a point,
//               V[] = vertex points of a polygon V[n+1] with V[n]=V[0]
//      Return:  wn = the winding number (=0 only if P is outside V[])

int CAirspace_Area::wn_PnPoly(const double &longitude, const double &latitude) const {
    int wn = 0; // the winding number counter

    // loop through all edges of the polygon
    CPoint2DArray::const_iterator it = _geopoints.begin();
    CPoint2DArray::const_iterator itnext = it;
    ++itnext;
    for (int i = 0; i < ((int) _geopoints.size() - 1); ++i, ++it, ++itnext) {
        if (it->Latitude() <= latitude) { // start y <= P.Latitude
            if (itnext->Latitude() > latitude) // an upward crossing
                if (isLeft(*it, *itnext, longitude, latitude) > 0) // P left of edge
                    ++wn; // have a valid up intersect
        } else { // start y > P.Latitude (no test needed)
            if (itnext->Latitude() <= latitude) // a downward crossing
                if (isLeft(*it, *itnext, longitude, latitude) < 0) // P right of edge
                    --wn; // have a valid down intersect
        }
    }
    return wn;
}


// Check if the given coordinate is inside the airspace

bool CAirspace_Area::IsHorizontalInside(const double &longitude, const double &latitude) const {
    if (_geopoints.size() < 3) return false;
    // first check if point is within bounding box
    if (
            (latitude > _bounds.miny)&&
            (latitude < _bounds.maxy) &&
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

double CAirspace_Area::Range(const double &longitude, const double &latitude, double &bearing) const {
    // find nearest distance to line segment
    unsigned int i;
    unsigned int dist = 0;
    unsigned int dist_candidate = 0;
    CPoint2D p3(latitude, longitude);
    int x = 0, y = 0, z = 0;
    int xc = 0, yc = 0, zc = 0;

    int wn = 0; // the winding number counter

    CPoint2DArray::const_iterator it = _geopoints.begin();
    CPoint2DArray::const_iterator itnext = it;
    ++itnext;

    for (i = 0; i < _geopoints.size() - 1; ++i) {
        dist = p3.DistanceXYZ(*it, *itnext, &x, &y, &z);

        if (it->Latitude() <= latitude) { // start y <= P.Latitude
            if (itnext->Latitude() > latitude) // an upward crossing
                if (isLeft(*it, *itnext, longitude, latitude) > 0) // P left of edge
                    ++wn; // have a valid up intersect
        } else { // start y > P.Latitude (no test needed)
            if (itnext->Latitude() <= latitude) // a downward crossing
                if (isLeft(*it, *itnext, longitude, latitude) < 0) // P right of edge
                    --wn; // have a valid down intersect
        }

        if ((dist < dist_candidate) || (i == 0)) {
            dist_candidate = dist;
            xc = x;
            yc = y;
            zc = z;
        }
        ++it;
        ++itnext;
    }

    CPoint2D p4(xc, yc, zc);
    double nearestdistance;
    double nearestbearing;

    DistanceBearing(latitude, longitude, p4.Latitude(), p4.Longitude(), &nearestdistance, &nearestbearing);

    bearing = nearestbearing;
    if (wn != 0) return -nearestdistance;
    else return nearestdistance;
}

// Calculate airspace bounds

void CAirspace_Area::CalcBounds() {
    LKASSERT(!_geopoints.empty());
    CPoint2DArray::iterator it = _geopoints.begin();

    _bounds.minx = it->Longitude();
    _bounds.maxx = it->Longitude();
    _bounds.miny = it->Latitude();
    _bounds.maxy = it->Latitude();
    for (it = _geopoints.begin(); it != _geopoints.end(); ++it) {
        _bounds.minx = min((double) it->Longitude(), _bounds.minx);
        _bounds.maxx = max((double) it->Longitude(), _bounds.maxx);
        _bounds.miny = min((double) it->Latitude(), _bounds.miny);
        _bounds.maxy = max((double) it->Latitude(), _bounds.maxy);
    }

    // JMW detect airspace that wraps across 180
    if ((_bounds.minx< -90) && (_bounds.maxx > 90)) {
        double tmp = _bounds.minx;
        _bounds.minx = _bounds.maxx;
        _bounds.maxx = tmp;
        for (it = _geopoints.begin(); it != _geopoints.end(); ++it) {
            if (it->Longitude() < 0) {
                CPoint2D newpoint(it->Latitude(), it->Longitude() + 360);
                *it = newpoint;
            }
        }
    }
}

//
// CAIRSPACEMANAGER CLASS
//

bool CAirspaceManager::StartsWith(const TCHAR *Text, const TCHAR *LookFor) {
    if (!(*LookFor)) return true;
    int count_look=_tcslen(LookFor);
    do {
        //if (!(*LookFor)) return true;
        if (*Text != *LookFor) return false;
        ++Text;
        ++LookFor;
    } while (--count_look);
    return true;
}

bool CAirspaceManager::ReadCoords(TCHAR *Text, double *X, double *Y) {
    double Ydeg = 0, Ymin = 0, Ysec = 0;
    double Xdeg = 0, Xmin = 0, Xsec = 0;
    const TCHAR *Stop = Text;

    // ToDo, add more error checking and making it more tolerant/robust

    Ydeg = (double) StrToDouble(Text, &Stop);
    if ((Text == Stop) || (*Stop == '\0')) return false;
    Stop++;
    Ymin = (double) StrToDouble(Stop, &Stop);
    if (Ymin < 0 || Ymin >= 60) {
        // ToDo
    }
    if (*Stop == '\0') return false;
    if (*Stop == ':') {
        Stop++;
        if (*Stop == '\0') return false;
        Ysec = (double) StrToDouble(Stop, &Stop);
        if (Ysec < 0 || Ysec >= 60) {
            // ToDo
        }
    }

    *Y = Ysec / 3600 + Ymin / 60 + Ydeg;

    while (*Stop == ' ') Stop++;

    if (*Stop == '\0') return false;
    if ((*Stop == 'S') || (*Stop == 's')) {
        *Y = *Y * -1;
    }
    Stop++;
    if (*Stop == '\0') return false;

    Xdeg = (double) StrToDouble(Stop, &Stop);
    if (*Stop == '\0') return false;
    Stop++;
    Xmin = (double) StrToDouble(Stop, &Stop);
    if (*Stop == ':') {
        Stop++;
        if (*Stop == '\0') return false;
        Xsec = (double) StrToDouble(Stop, &Stop);
    }

    *X = Xsec / 3600 + Xmin / 60 + Xdeg;

    while (*Stop == ' ') Stop++;
    if (*Stop == '\0') return false;
    if ((*Stop == 'W') || (*Stop == 'w')) {
        *X = *X * -1;
    }

    if (*X<-180) {
        *X += 360;
    }
    if (*X > 180) {
        *X -= 360;
    }

    return true;
}

bool CAirspaceManager::CalculateArc(TCHAR *Text, CPoint2DArray *_geopoints, double Center_lon, double Center_lat, int Rotation) {
    double StartLat, StartLon;
    double EndLat, EndLon;
    double StartBearing;
    double EndBearing;
    double Radius;
    double arc_bearing_range;
    TCHAR *Comma = NULL;
    double lat, lon;

    ReadCoords(Text, &StartLon, &StartLat);

    Comma = _tcschr(Text, ',');
    if (!Comma)
        return false;

    ReadCoords(&Comma[1], &EndLon, &EndLat);

    DistanceBearing(Center_lat, Center_lon, StartLat, StartLon, &Radius, &StartBearing);
    DistanceBearing(Center_lat, Center_lon, EndLat, EndLon, NULL, &EndBearing);
    _geopoints->emplace_back(StartLat, StartLon);

    if (Rotation > 0) {
        arc_bearing_range = AngleLimit360(EndBearing - StartBearing);
    } 
    else {
        arc_bearing_range = AngleLimit360(StartBearing - EndBearing);
    }

    // TODO : use radius for calculate bearing increment.
    while (arc_bearing_range > 7.5) {
        StartBearing = AngleLimit360(StartBearing + Rotation * 5);
        arc_bearing_range -= 5;

        FindLatitudeLongitude(Center_lat, Center_lon, StartBearing, Radius, &lat, &lon);
        _geopoints->emplace_back(lat, lon);
    }
    _geopoints->emplace_back(EndLat, EndLon);
    return true;
}

bool CAirspaceManager::CalculateSector(TCHAR *Text, CPoint2DArray *_geopoints, double Center_lon, double Center_lat, int Rotation) {
    double arc_bearing_range = 0.0;
    const TCHAR *Stop = nullptr;
    double lat = 0, lon = 0;

    // TODO 110307 FIX problem of StrToDouble returning 0.0 in case of error , and not setting Stop!!
    double Radius = Units::From(unNauticalMiles, StrToDouble(Text, &Stop));
    if(!Stop) {
        return false;
    }
    double StartBearing = (double) StrToDouble(&Stop[1], &Stop);
    if(!Stop) {
        return false;
    }
    double EndBearing = (double) StrToDouble(&Stop[1], nullptr);

    if (Rotation > 0) {
        arc_bearing_range = AngleLimit360(EndBearing - StartBearing);
    } 
    else {
        arc_bearing_range = AngleLimit360(StartBearing - EndBearing);
    }

    while (arc_bearing_range > 7.5) {
        if (StartBearing >= 360) StartBearing -= 360;
        if (StartBearing < 0) StartBearing += 360;

        FindLatitudeLongitude(Center_lat, Center_lon, StartBearing, Radius, &lat, &lon);

        _geopoints->emplace_back(lat, lon);

        StartBearing += Rotation * 5;
        arc_bearing_range -= 5;
    }
    FindLatitudeLongitude(Center_lat, Center_lon, EndBearing, Radius, &lat, &lon);
    _geopoints->emplace_back(lat, lon);
    return true;
}

void CAirspaceManager::AddGeodesicLine(CPoint2DArray &points, double lat, double lon) {
#ifdef _WGS84
    if (earth_model_wgs84) {
        AddGeodesicLine_WGS84(points, lat, lon);
    } 
    else 
#endif
    {
        AddGeodesicLine_FAI(points, lat, lon);
    }
}

#ifdef _WGS84
void CAirspaceManager::AddGeodesicLine_WGS84(CPoint2DArray &points, double lat, double lon) {

  if (!points.empty()) {
    const CPoint2D &prev = points.back();
    const Geodesic &geod = Geodesic::WGS84();

    constexpr unsigned mask = Geodesic::AZIMUTH|Geodesic::LATITUDE|Geodesic::LONGITUDE;
    constexpr double step = 2./60.; // ~ 2 nautical miles at equator

    GeodesicLine line = geod.InverseLine(prev.Latitude(), prev.Longitude(), lat, lon, mask);

    const unsigned numpoints = line.Arc() / step;
    for (unsigned i = 0; i < numpoints; ++i) {
        double i_lat, i_lon;
        line.ArcPosition(i * step, i_lat, i_lon);
        points.emplace_back(i_lat, i_lon);
    }
  }
  points.emplace_back(lat, lon);
}
#endif

void CAirspaceManager::AddGeodesicLine_FAI(CPoint2DArray &points, double lat, double lon) {

  if (!points.empty()) {
    const CPoint2D &prev = points.back();
    // ~ 2 nautical miles at equator
    const double step = DEG_TO_RAD * ((prev.Longitude() < lon) ? 2./60. : -2./60.);
    const double lat1 = DEG_TO_RAD * prev.Latitude();
    const double lon1 = DEG_TO_RAD * prev.Longitude();
    const double lat2 = DEG_TO_RAD * lat;
    const double lon2 = DEG_TO_RAD * lon;

    const double cos_lat1 = std::cos(lat1);
    const double cos_lat2 = std::cos(lat2);
    const double A = std::sin(lat2) * cos_lat1;
    const double B = std::sin(lat1) * cos_lat2;
    const double C = cos_lat1 * cos_lat2 * std::sin(lon1 - lon2);

    for (double i_lon = lon1; i_lon < lon2; i_lon += step) {
      double i_lat = std::atan((B * std::sin(i_lon - lon2) - A * std::sin(i_lon - lon1)) / C);
	  points.emplace_back(RAD_TO_DEG * i_lat, RAD_TO_DEG * i_lon);
    }
  }
  points.emplace_back(lat, lon);
}

// Correcting geopointlist
// All algorithms require non self-intersecting and closed polygons.
// Also the geopointlist last element have to be the same as first -> openair doesn't require this, we have to do it here
// Also delete adjacent duplicated vertexes
bool CAirspaceManager::CorrectGeoPoints(CPoint2DArray &points) {

    // Here we expect at least 3 points
    if(points.size() < MIN_AS_SIZE) {
        return false;
    }

    // First delete consecutive duplicated vertexes
    CPoint2DArray::iterator last = std::unique(std::next(points.begin()), points.end());
    points.erase(last, points.end());

    // Then close polygon if not already closed
    if (points.front() != points.back()) {
        points.push_back(points.front());
    }

    // For a valid closed polygon we need at least 3 points plus the closing one
    return points.size() > MIN_AS_SIZE;
}


static void AppendComment(tstring& Comment, const TCHAR* str) {
    if(!Comment.empty()) {
        if(Comment.back() != _T('\n')) {
            Comment += _T("\n");    
        }
        Comment += _T("\n");
    }
    Comment += str;
}

// Reading and parsing OpenAir airspace file
bool CAirspaceManager::FillAirspacesFromOpenAir(const TCHAR* szFile) {

    zzip_stream stream(szFile, "rt");
    if(!stream) {
      StartupStore(TEXT("... Unable to open airspace file : %s\n"), szFile);
      return false;
    }    

  
    TCHAR sTmp[READLINE_LENGTH + 1];
    int linecount = 0;
    int parsing_state = 0;
    // Variables to store airspace parameters
    tstring ASComment;
    tstring Name;
    CPoint2DArray points;
    double Radius = 0;
    GeoPoint Center;
    int Type = 0;
    unsigned int skiped_cnt =0;
    unsigned int accept_cnt =0;

    vertical_position Base;
    std::optional<vertical_position> Base2;

    vertical_position Top;
    std::optional<vertical_position> Top2;

    int Rotation = 1;
    double CenterX = 0;
    double CenterY = 0;
    double lat = 0, lon = 0;
    bool flyzone = false;
    bool enabled = true;

    bool except_saturday = false;
    bool except_sunday = false;

    short maxwarning=3; // max number of warnings to confirm, then automatic confirmation
    bool InsideMap = !(WaypointsOutOfRange > 1); // exclude
    StartupStore(TEXT(". Reading OpenAir airspace file%s"), NEWLINE);

    std::istream istream(&stream);
    std::string src_line;

    while (std::getline(istream, src_line)) {
        ++linecount;
        tstring Text = from_unknown_charset(src_line.c_str());
        TCHAR* p = Text.data();
        while (*p != 0 && isspace(*p)) {
            p++; // Skip whitespaces
        }
        if (*p == 0) {
            continue; // ignore empty string
        }

        if (*p == '*') {
            //  comment lines
            ++p; // Skip '*'
            // http://pascal.bazile.free.fr/paraglidingFolder/divers/GPS/OpenAir-Format/index.htm
            if(_tcsstr(p, _T("ADescr ")) == p) {
                // *ADescr - The 'full description' (*) of the air zone
                //      as officially provided by SIA-France via its website;
                //      or in the accompanying booklets for ICAO air maps
                AppendComment(ASComment, (p+_tcslen(_T("ADescr "))));
            } else if(_tcsstr(p, _T("AActiv ")) == p) {
                // *AActiv - Additional information concerning the 'activation methods' (*) of the area concerned
                //      as officially provided by SIA-France via its website;
                //      or in the accompanying booklets for ICAO air charts
                AppendComment(ASComment, (p+_tcslen(_T("AActiv "))));
            } else if(_tcsstr(p, _T("AMhz ")) == p) {
                // *AMzh - The 'list of radio frequencies' associated with the air zone and in the frequency band [118.00 to 136.00 Mhz].
                if(!ASComment.empty()) {
                    if(ASComment.back() != _T('\n')) {
                        ASComment += _T("\n");    
                    }
                    ASComment += _T("\n");
                }
                p += _tcslen(_T("AMhz "));

                
                picojson::value value;
                std::string err = picojson::parse(value, to_utf8(p));
                if (err.empty() && value.is<picojson::object>()) {
                    std::string str_radio;
                    const picojson::value::object& object = value.get<picojson::object>();
                    for(const auto& pair : object) {
                        str_radio += pair.first + " : ";
                        if(pair.second.is<picojson::array>()) {
                            for(const auto& value : pair.second.get<picojson::array>()) {
                                str_radio += value.get<std::string>();
                                str_radio += "\n";
                            }
                        } else if(pair.second.is<std::string>()) {
                            str_radio += pair.second.get<std::string>();
                        } else {
                            str_radio += pair.second.serialize();
                        }
                    }
                    AppendComment(ASComment, utf8_to_tstring(str_radio.c_str()).c_str());
                } else {
                    // data is not a valid json, put it "as is" in Comment.
                    AppendComment(ASComment, p);
                }
            } else if(_tcsstr(p, _T("ASeeNOTAM ")) == p) {
                //*ASeeNOTAM - Binary indicator 'See NOTAM': 'Yes' specifying that the zone can be activated by NOTAM
                enabled = (_tcscmp(p+_tcslen(_T("ASeeNOTAM ")), _T("Yes")) != 0);
            } else if(_tcsstr(p, _T("AExSAT ")) == p) {
                //*AExSAT - A calculated binary indicator 'Except SATurday': 'Yes' specifying that the zone cannot be activated on Saturday
                except_saturday = (_tcscmp(p+_tcslen(_T("AExSAT ")), _T("Yes")) == 0);
            } else if(_tcsstr(p, _T("AExSUN ")) == p) {
                //*AExSUN - Binary indicator 'Except SUNday': 'Yes' specifying that the zone cannot be activated on Sunday
                except_sunday = (_tcscmp(p+_tcslen(_T("AExSUN ")), _T("Yes")) == 0);
#if 0
            } else if(_tcsstr(p, _T("AExHOL ")) == p) {
                //*AExHOL - Binary indicator 'Except HOLiday': 'Yes' specifying that the zone cannot be activated on public holidays
                bool b = (_tcscmp(p+_tcslen(_T("AExHOL ")), _T("Yes")) == 0);
                // TODO : Ask to User if this day is Holiday
#endif
            } else if(_tcsstr(p, _T("AH2 ")) == p) {
                // Second Airspace Ceiling
                Top2 = vertical_position::parse_open_air(p + _tcslen(_T("AH2 ")));
            } else if(_tcsstr(p, _T("AL2 ")) == p) {
                // Second Airspace Floor
                Base2 = vertical_position::parse_open_air(p + _tcslen(_T("AL2 ")));
            }
            continue;
        }

        // Strip comments from end of line
        TCHAR* Comment = _tcschr(p, _T('*'));
        if (Comment != NULL) {
            *Comment = _T('\0'); // Truncate line
        }
        if (*p == 0) {  // always false, but do not remove in case of code change.
            continue; // ignore empty string
        }

        //    StartupStore(TEXT(".  %s%s"),p,NEWLINE);

        switch (*p) {
            case _T('A'):
                p++; // skip A
                switch (*p) {
                    case _T('C'): //AC
                        p++; // skip C
                        if (parsing_state == 10) { // New airspace begin, store the old one, reset parser
                            if (InsideMap) {
                              CreateAirspace(Name.c_str(), points, Radius, Center, Type,
                                            {Base, Base2}, {Top, Top2}, ASComment,
                                            flyzone, enabled, except_saturday, except_sunday);
                            }

                            Name.clear();
                            ASComment.clear();
                            Radius = 0;
                            Center = {0, 0};
                            points.clear();
                            Type = OTHER;
                            Base = {};
                            Base2 = {};
                            Top = {};
                            Top2 = {};
                            flyzone = false;
                            enabled = true;
                            except_saturday = false;
                            except_sunday = false;
                            parsing_state = 0;
                            InsideMap = !( WaypointsOutOfRange > 1); // exclude?
                        }
                        // New AC
                        p++; //Skip space
                        Type = type_table.get(p, OTHER);

                        if (Type == CLASSNOTAM) {
                            // notam class disabled by default
                            enabled = false;
                        }

                        Rotation = +1;
                        parsing_state = 10;
                        break;

                    case _T('N'): //AN - Airspace name
                        p++;
                        p++;

                        if (parsing_state == 10) {
                        	Name = p;
                        }
                        break;

                    case _T('L'): //AL - base altitude
                        p++;
                        p++;
                        if (parsing_state == 10) {
                            Base = vertical_position::parse_open_air(p);
                        }
                        break;

                    case _T('H'): //AH - top altitude
                        p++;
                        p++;
                        if (parsing_state == 10) {
                            Top = vertical_position::parse_open_air(p);
                        }
                        break;

                        //OpenAir non standard field - AF - define a fly zone
                    case _T('F'): // AF - Fly zone, no parameter
						if (parsing_state == 10) {
                            unsigned khz = ExtractFrequency(p);
                            unsigned name_khz = ExtractFrequency(Name.c_str());
                            if (khz != name_khz) {
                              lk::snprintf(sTmp, _T("%s %s"), Name.c_str(), p);
                              Name = sTmp;
                            }
						}
						else {
                          flyzone = true;
                        }
                        continue;

                    case _T('T'): // AT
                        // ignore airspace labels
                        // TODO: adding airspace labels
                        continue;

                    case _T('G'): //AG - Ground station name
                        if (parsing_state == 10) {
                            lk::snprintf(sTmp, TEXT("%s %s"),  Name.c_str(), p );
                            Name = sTmp;
                        }
                        continue;

                    case _T('Y'): // AY
                        if (*(++p) == ' ') {
                            ++p; //Skip Space ?
                        }
                        Type = update_type(Type, type_table.get(p, OTHER));
                        continue;

                    case _T('I'): // AI
                        if (*(++p) == ' ') {
                            ++p; //Skip Space ?
                        }
                        // TODO: use ID has hash replacement ?
                        DebugLog(_T("Airpsace : ignore field AI <%s : %s>"), Name.c_str(), ++p);
                        continue;

                    default:
                        if (maxwarning > 0) {
                            if (maxwarning == 1) {
                                lk::snprintf(sTmp, _T("Parse error 9 at line %d\r\n\"%s\"\r\nNO OTHER WARNINGS."), linecount, p);
                            } else {
                                lk::snprintf(sTmp, _T("Parse error 10 at line  %d\r\n\"%s\"\r\nLine skipped."), linecount, p);
                            }
                            maxwarning--;
                            // LKTOKEN  _@M68_ = "Airspace"
                            if (MessageBoxX(sTmp, MsgToken<68>(), mbOkCancel) == IdCancel) {
                                return false;
                            }
                        }
                        break;
                } //sw
                break;

            case _T('D'):
                p++; // Skip D
                switch (*p) {
                    case _T('A'): //DA - Sector
                        p++;
                        p++; // skip A and space
                        if (!CalculateSector(p, &points, CenterX, CenterY, Rotation)) {
                            lk::snprintf(sTmp, _T("Parse error 1 at line %d\r\n\"%s\"\r\nLine skipped."), linecount, p);
                            // LKTOKEN  _@M68_ = "Airspace"
                            if(!InsideMap) {
                              if (RasterTerrain::WaypointIsInTerrainRange(CenterY,CenterX)) {
                                InsideMap = true;
                              }
                            }
                            if (MessageBoxX(sTmp, MsgToken<68>(), mbOkCancel) == IdCancel) return false;
                        }
                        break;

                    case _T('B'): //DB - Arc
                        p++;
                        p++; // skip B and space
                        if (!CalculateArc(p, &points, CenterX, CenterY, Rotation)) {
                            lk::snprintf(sTmp, _T("Parse error 2 at line %d\r\n\"%s\"\r\nLine skipped."), linecount, p);
                            // LKTOKEN  _@M68_ = "Airspace"
                            if(!InsideMap) {
                              if (RasterTerrain::WaypointIsInTerrainRange(CenterY,CenterX)) {
                                InsideMap = true;
                              }
                            }
                            if (MessageBoxX(sTmp, MsgToken<68>(), mbOkCancel) == IdCancel) return false;
                        }
                        break;

                    case _T('C'): //DC - Circle
                        p++;
                        p++;
                        Radius = StrToDouble(p, NULL);
                        Radius = Units::From(unNauticalMiles, Radius);
                        Center = { CenterY, CenterX };
                        if(!InsideMap) {
                          if (RasterTerrain::WaypointIsInTerrainRange(CenterY,CenterX)) {
                            InsideMap = true;
                          } 
                        }
                        break;

                    case _T('P'): //DP - polygon point
                        p++;
                        p++; // skip P and space
                        if (ReadCoords(p, &lon, &lat)) {
                            if(!InsideMap) {
                              if (RasterTerrain::WaypointIsInTerrainRange(lat,lon))  {
                                InsideMap = true;
                              } 
                            }
                            AddGeodesicLine(points, lat, lon);
                        } else {
                            lk::snprintf(sTmp, _T("Parse error 3 at line %d\r\n\"%s\"\r\nLine skipped."), linecount, p);
                            // LKTOKEN  _@M68_ = "Airspace"
                            if (MessageBoxX(sTmp, MsgToken<68>(), mbOkCancel) == IdCancel) return false;
                        }
                        break;

                        // todo DY airway segment
                        // what about 'V T=' ?
                    default:
                        if (maxwarning > 0) {
                            if (maxwarning == 1) {
                                lk::snprintf(sTmp, _T("Parse error 4 at line %d\r\n\"%s\"\r\nNO OTHER WARNINGS"), linecount, p);
                            } else {
                                lk::snprintf(sTmp, _T("Parse error 5 at line %d\r\n\"%s\"\r\nLine skipped."), linecount, p);
                            }
                            maxwarning--;

                            // LKTOKEN  _@M68_ = "Airspace"
                            if (MessageBoxX(sTmp, MsgToken<68>(), mbOkCancel) == IdCancel) {
                                return false;
                            }
                        }
                        break;
                } //sw
                break;

            case _T('V'):
                p++; //skip V
                if (*p == ' ') p++; //skip space if present
                if (StartsWith(p, TEXT("X="))) {
                    if (ReadCoords(p + 2, &CenterX, &CenterY))
                        break;
                } else if (StartsWith(p, TEXT("D=-"))) {
                    Rotation = -1;
                    break;
                } else if (StartsWith(p, TEXT("D=+"))) {
                    Rotation = +1;
                    break;
                } else if (StartsWith(p, TEXT("Z"))) {
                    // ToDo Display Zool Level
                    break;
                } else if (StartsWith(p, TEXT("W"))) {
                    // ToDo width of an airway

                    break;
                } else if (StartsWith(p, TEXT("T"))) {
                    // ----- JMW THIS IS REQUIRED FOR LEGACY FILES
                    break;
                }

                lk::snprintf(sTmp, _T("Parse error 6 at line %d\r\n\"%s\"\r\nLine skipped."), linecount, p);
                // LKTOKEN  _@M68_ = "Airspace"
                if (MessageBoxX(sTmp, MsgToken<68>(), mbOkCancel) == IdCancel) return false;
                break;

            case _T('S'): // ignore the SB,SP ...
                p++;
                if (*p == _T('B')) continue;
                if (*p == _T('P')) continue;
                // if none of the above, then falling to default

            default:
                if (maxwarning > 0) {
                    if (maxwarning == 1) {
                        lk::snprintf(sTmp, TEXT("Parse error 7 at line %d\r\n\"%s\"\r\nNO OTHER WARNINGS."), linecount, p);
                    } else {
                        lk::snprintf(sTmp, TEXT("Parse error 8 at line %d\r\n\"%s\"\r\nLine skipped."), linecount, p);
                    }
                    maxwarning--;
                    // LKTOKEN  _@M68_ = "Airspace"
                    if (MessageBoxX(sTmp, MsgToken<68>(), mbOkCancel) == IdCancel) {
                        return false;
                    }
                }
                break;
        }//sw

    }//wh readline

    // Push last one to the list
    if (parsing_state == 10 && InsideMap) {
      CreateAirspace(Name.c_str(), points, Radius, Center, Type,
                     {Base, Base2}, {Top, Top2}, ASComment,
                     flyzone, enabled, except_saturday, except_sunday);
    }

    unsigned airspaces_count = 0;
    { // Begin Lock
        ScopeLock guard(_csairspaces);
        airspaces_count = _airspaces.size();
    }  // End Lock
    StartupStore(TEXT(". Now we have %u airspaces"), airspaces_count);
    TCHAR msgbuf[128];
    lk::snprintf(msgbuf, _T("OpenAir: %u airspaces of %u excluded by Terrain Filter"), skiped_cnt, skiped_cnt + accept_cnt);
 //   DoStatusMessage(msgbuf);
    StartupStore(TEXT(". %s"),msgbuf);
    OutsideAirspaceCnt += skiped_cnt;
    return true;
}

void CAirspaceManager::CreateAirspace(const TCHAR* Name, CPoint2DArray& Polygon, double Radius, const GeoPoint& Center, int Type,
                           vertical_bound&& Base, vertical_bound&& Top, const tstring& Comment, 
                           bool flyzone, bool enabled, bool except_saturday, bool except_sunday) {
    try {
        std::unique_ptr<CAirspace> airspace;
        if (Radius > 0) {
            // Was a circle
            airspace = std::make_unique<CAirspace_Circle>(Center, Radius);
        } else if (CorrectGeoPoints(Polygon)) {
            // Was an area
            // Skip it if we dont have minimum 3 points
            airspace = std::make_unique<CAirspace_Area>(std::move(Polygon));
        }
        Polygon.clear(); // required, otherwise vector state is undefined;

        if (airspace) {

            const std::basic_regex<TCHAR> re(_T(R"(.*((?:Lower)|(?:Upper))\((.*?)-(.*?)\).*)"));
            std::match_results<const TCHAR*> match;
            if (std::regex_match(Name, match, re)) {
                vertical_position Alt1 = vertical_position::parse_open_air(match[2].str().c_str());
                vertical_position Alt2 = vertical_position::parse_open_air(match[3].str().c_str());
                if (match[1].str() == _T("Lower")) {
                    Base.update(Alt1, Alt2);
                }
                else if (match[1].str() == _T("Upper")) {
                    Top.update(Alt1, Alt2);
                }
            }

            airspace->Init(Name, Type, std::move(Base), std::move(Top), flyzone , Comment.c_str());
            airspace->Enabled(enabled);
            airspace->ExceptSaturday(except_saturday);
            airspace->ExceptSunday(except_sunday);

            WithLock(_csairspaces, [&] {
                _airspaces.push_back(std::move(airspace));
            });
        }
    }
    catch(std::exception& e) {
        StartupStore(_T("failed to create airspace : %s"), to_tstring(e.what()).c_str());
    }
}


// Reads airspaces from an OpenAIP file
bool CAirspaceManager::FillAirspacesFromOpenAIP(const TCHAR* szFile) {
    
    StartupStore(TEXT(". Reading OpenAIP airspace file"));
    unsigned int skiped_cnt=0;
    unsigned int accept_cnt=0;

    std::string ss;
    xml_document xmldoc;
    try {
        zzip_stream file(szFile, "rt");
        if(!file) {
            StartupStore(TEXT("... Unable to open airspace file : %s"), szFile);
            return false;
        }
        std::istreambuf_iterator<char> it(&file), end;
        ss.assign(it, end);

        constexpr int Flags = rapidxml::parse_trim_whitespace | rapidxml::parse_normalize_whitespace;
        xmldoc.parse<Flags>(ss.data());
    } catch (std::exception& e) {
        StartupStore(TEXT(".. OPENAIP load failed : %s"), to_tstring(e.what()).c_str());
        return false;
    }

    const xml_node* root_node = xmldoc.first_node("OPENAIP");
    if(!root_node) {
        StartupStore(TEXT(".. OPENAIP tag not found.%s"), NEWLINE);
        return false;
    }

    // Check version of OpenAIP format
    const xml_attribute* data_format = root_node->first_attribute("DATAFORMAT");
    if(!data_format || strtod(data_format->value(), nullptr) != 1.1) {
        StartupStore(TEXT(".. DATAFORMAT attribute missing or not at the expected version: 1.1.%s"), NEWLINE);
        return false;
    }

    const xml_node* airspaces_node = root_node->first_node("AIRSPACES");
    if(!airspaces_node) {
        StartupStore(TEXT(".. AIRSPACES tag not found.%s"), NEWLINE);
        return FALSE;
    }

    for(const xml_node* asp_node = airspaces_node->first_node("ASP"); asp_node; asp_node = asp_node->next_sibling("ASP")) {

        // Airspace category
        const xml_attribute* category = asp_node->first_attribute("CATEGORY");
        if(!category) {
            StartupStore(TEXT(".. Skipping ASP with no CATEGORY attribute.%s"), NEWLINE);
            continue;
        }
        const char* dataStr = category->value();
        size_t len = strlen(dataStr);
        int Type = OTHER;
        if(len>0) switch(dataStr[0]) {
        case 'A':
            if(len==1) Type=CLASSA; // A class airspace
            break;
        case 'B':
            if(len==1) Type=CLASSB; // B class airspace
            break;
        case 'C':
            if(len==1) Type=CLASSC; // C class airspace
            else if (strcasecmp(dataStr,"CTR")==0) Type=CTR; // CTR airspace
            break;
        case 'D':
            if(len==1) Type=CLASSD; // D class airspace
            else if (strcasecmp(dataStr,"DANGER")==0) Type=DANGER; // Dangerous area
            break;
        case 'E':
            if(len==1) Type=CLASSE; // E class airspace
            break;
        case 'F':
            if(len==1) Type=CLASSF; // F class airspace
            //else if (_tcsicmp(dataStr,_T("FIR"))==0) continue; //TODO: FIR missing in LK8000
            break;
        case 'G':
            if(len==1) Type=CLASSG; // G class airspace
            else if (strcasecmp(dataStr, "GLIDING")==0) Type=GLIDERSECT;
            break;
        //case 'O':
            //if (_tcsicmp(dataStr,_T("OTH"))==0) continue; //TODO: OTH missing in LK8000
            //break;
        case 'P':
            if (strcasecmp(dataStr,"PROHIBITED")==0) Type=PROHIBITED; // Prohibited area
            break;
        case 'R':
            if (strcasecmp(dataStr,"RESTRICTED")==0) Type=RESTRICT; // Restricted area
            else if (strcasecmp(dataStr,"RMZ")==0) Type=CLASSRMZ; //RMZ
            break;
        case 'T':
            if(len==3 && dataStr[1]=='M') {
                //if(dataStr[2]=='A') continue; //TODO: TMA missing in LK8000
                /*else*/ if(dataStr[2]=='Z') Type=CLASSTMZ; //TMZ
            }
            break;
        case 'W':
            if (strcasecmp(dataStr,"WAVE")==0) Type=WAVE; //WAVE
            break;
        case 'U':
            if (strcasecmp(dataStr,"UIR")==0)
                Type = OTHER; //TODO: UIR missing in LK8000
            break;
        default:
            break;
        } else {
            StartupStore(TEXT(".. ASP with CATEGORY attribute empty.%s"), NEWLINE);
            if(dataStr == nullptr) continue;
            Type = OTHER;
        }
        if(Type < 0) {
            Type = OTHER;
        }

        // Airspace country
        //node=ASPnode.getChildNode(TEXT("COUNTRY"));
        //TODO: maybe do something with country

        // Airspace name
        const xml_node* node_name = asp_node->first_node("NAME");
        if(!node_name) {
            StartupStore(TEXT(".. Skipping ASP without NAME.%s"), NEWLINE);
            continue;
        }
        const char* szName = node_name->value();
        if(!szName || strlen(szName) == 0) {
            StartupStore(TEXT(".. Skipping ASP without NAME.%s"), NEWLINE);
            continue;
        }

#ifdef UNICODE
        TCHAR Name[NAME_SIZE + 1];
        from_utf8(szName, Name);
#else
        const TCHAR* Name = szName;
#endif

        vertical_position Top;
        vertical_position Base;
        try {
            // Airspace top altitude
            const xml_node* top_node = asp_node->first_node("ALTLIMIT_TOP");
            Top = vertical_position::parse_open_aip(top_node);

            // Airspace bottom altitude
            const xml_node* bottom_node = asp_node->first_node("ALTLIMIT_BOTTOM");
            Base = vertical_position::parse_open_aip(bottom_node);
        }
        catch(std::exception&) {
            StartupStore(TEXT(".. Skipping ASP with unparsable or missing Vertical limit."));
            continue;
        }

        //Geometry
        const xml_node* geometry_node = asp_node->first_node(("GEOMETRY"));
        if(!geometry_node) {
            StartupStore(TEXT(".. Skipping ASP without GEOMETRY.%s"), NEWLINE);
            continue;
        }

        // Polygon (the only one supported for now)
        const xml_node* polygon_node = geometry_node->first_node("POLYGON");
        if(!polygon_node) {
            StartupStore(TEXT(".. Skipping ASP without POLYGON inside GEOMETRY.%s"), NEWLINE);
            continue;
        }

        // Polygon point list
        CPoint2DArray points;
        lk::tokenizer<char> tokPoint(polygon_node->value());
        char* point = tokPoint.Next({','});
        bool InsideMap = !( WaypointsOutOfRange > 1); // exclude?
        bool error = (point==nullptr);
        while(point!=nullptr && !error) {
            lk::tokenizer<char> tokCoord(point);
            char* coord=tokCoord.Next({' '}, true);
            if ((error=(coord==nullptr))) break;
            double lon=strtod(coord,nullptr); // Beware that here the longitude comes first!
            if ((error=(lon<-180 || lon>180))) break;
            coord=tokCoord.Next({' '}, true);
            if ((error=(coord==nullptr))) break;
            double lat=strtod(coord,nullptr);
            if ((error=(lat<-90 || lat>90))) break;

            if(!InsideMap) {
              if (RasterTerrain::WaypointIsInTerrainRange(lat,lon)) {
                InsideMap = true;
              }
            }
            AddGeodesicLine(points, lat, lon);
            point = tokPoint.Next({','});
        }

        if(!InsideMap) {
#ifdef WORKBENCH
            StartupStore(TEXT(".. Skipping ASP because outside of loaded Terrain.%s"), NEWLINE);
#endif
            skiped_cnt++;
            continue;
        }

        if(error) {
            StartupStore(TEXT(".. Skipping ASP because failed to parse POLYGON points list.%s"), NEWLINE);
            continue;
        }

        // Ensure that the polygon is closed (it should be already) and skip it if we don't have minimum 3 points
        if (!CorrectGeoPoints(points)) {
            StartupStore(TEXT(".. Skipping ASP with POLYGON with less than 3 points.%s"), NEWLINE);
            continue;
        }

        // Build the new airspace
        auto newairspace = std::make_unique<CAirspace_Area>(std::move(points));
        points.clear(); // required, otherwise vector state is undefined;
        if(newairspace==nullptr) {
            StartupStore(TEXT(".. Failed to allocate new airspace.%s"), NEWLINE);
            return false;
        }
        bool flyzone=false; //by default all airspaces are no-fly zones!!!
        newairspace->Init(Name, Type, {Base, {}}, {Top, {}}, flyzone);

        // Add the new airspace
        { // Begin Lock
        ScopeLock guard(_csairspaces);
        _airspaces.push_back(std::move(newairspace));
        accept_cnt++;
        } // End Lock
    } // for each ASP

    unsigned airspaces_count = 0;
    { // Begin Lock
        ScopeLock guard(_csairspaces);
        airspaces_count = _airspaces.size();
    } // End Lock
        
    StartupStore(TEXT(". Now we have %u airspaces"), airspaces_count);
    TCHAR msgbuf[128];
    lk::snprintf(msgbuf, _T("OpenAIP: %u of %u airspaces excluded by Terrain Filer"), skiped_cnt, skiped_cnt + accept_cnt);
 //   DoStatusMessage(msgbuf);
    StartupStore(TEXT(".%s"),msgbuf);
    OutsideAirspaceCnt += skiped_cnt;
    return true;
}

void CAirspaceManager::ReadAirspaces() {
    int fileCounter=0;
  //  for (TCHAR* airSpaceFile : {szAirspaceFile, szAdditionalAirspaceFile}) {
    for(unsigned int i = 0; i < NO_AS_FILES; i++) {
        fileCounter++;
        if(_tcslen(szAirspaceFile[i]) > 0) { // Check if there is a filename present
            
            TCHAR szFile[MAX_PATH] = TEXT("\0");
            LocalPath(szFile, _T(LKD_AIRSPACES), szAirspaceFile[i]);
            LPCTSTR wextension = _tcsrchr(szFile, _T('.'));

            bool readOk=false;

            if(wextension != nullptr) { // Check if we have a file extension
                if(_tcsicmp(wextension,_T(".txt"))==0) { // TXT file: should be an OpenAir
                    readOk = FillAirspacesFromOpenAir(szFile);
                } else if(_tcsicmp(wextension,_T(".aip"))==0) { // AIP file: should be an OpenAIP
                    readOk=FillAirspacesFromOpenAIP(szFile);
                }  else {
                    StartupStore(TEXT("... Unknown airspace file %d extension: %s%s"), fileCounter, wextension, NEWLINE);
                }

                if(!readOk) { // if file was OK remember otherwise forget it
                    StartupStore(TEXT("... Failed to parse airspace file %d: %s%s"), fileCounter, szFile, NEWLINE);
                }
            } else {
                StartupStore(TEXT("... Airspace file %d without extension.%s"), fileCounter, NEWLINE);
            }
            if(!readOk) { // if file was OK remember otherwise forget it
                lk::strcpy(szAirspaceFile[i], _T(""));
            }

        } else {
            StartupStore(TEXT("... No airspace file %d%s"),fileCounter, NEWLINE);
        }
    }

    unsigned airspaces_count = 0;
    { // Begin Lock
        ScopeLock guard(_csairspaces);
        last_day_of_week = ~0;
        airspaces_count = _airspaces.size();
    } //

    if((OutsideAirspaceCnt > 0) && ( WaypointsOutOfRange > 1) )
    {
      TCHAR msgbuf[128];
      lk::snprintf(msgbuf, _T(" %u of %u (%u%%) %s"),
                        OutsideAirspaceCnt,
                        OutsideAirspaceCnt+airspaces_count,
                        (100*OutsideAirspaceCnt)/(OutsideAirspaceCnt +airspaces_count),
                        MsgToken<2347>());  //_@M2347  "airspaces excluded!"
      DoStatusMessage(msgbuf);
      StartupStore(TEXT(".%s"),msgbuf);

    }
#if ASPWAVEOFF
    AirspaceDisableWaveSectors();
#endif
    if (AspPermanentChanged > 0) LoadSettings();
}

void CAirspaceManager::CloseAirspaces() {
    ScopeLock guard(_csairspaces);
    if (_airspaces.size() == 0) return;
    SaveSettings();

    // need to cleanup, otherwise "Item.Pointer" still not null but invalid
    LKNumAirspaces = 0;
    for (LKAirspace_Nearest_Item& Item : LKAirspaces) {
        Item.Valid = false;
        Item.Pointer.reset();
    }

    // this is needed for avoid crash if airspaces configuration is changed
    // after Step 1 and before step 2 of multicalc inside AirspacesWarning
    CAirspace::ResetSideviewNearestInstance();

    _selected_airspace.reset();
    _sideview_nearest.reset();
    _user_warning_queue = {};
    _airspaces_near.clear();
    _airspaces_of_interest.clear();
    _airspaces_page24.clear();
    _airspaces.clear();

    StartupStore(TEXT(". CloseLKAirspace%s"), NEWLINE);
}

void CAirspaceManager::QnhChangeNotify() {
    ScopeLock guard(_csairspaces);
    for (auto& pAsp : _airspaces) {
        pAsp->QnhChangeNotify();
    }
}

int CAirspaceManager::ScanAirspaceLineList(const double (&lats)[AIRSPACE_SCANSIZE_X], const double (&lons)[AIRSPACE_SCANSIZE_X],
        const double (&terrain_heights)[AIRSPACE_SCANSIZE_X],
        AirSpaceSideViewSTRUCT (&airspacetype)[MAX_NO_SIDE_AS]) const {

    const int iMaxNoAs = std::size(airspacetype);

    int iNoFoundAS = 0; // number of found airspaces in scan line
    unsigned int iSelAS = 0; // current selected airspace for processing
    ScopeLock guard(_csairspaces);

    airspacetype[0].psAS = NULL;
    for (const auto& pAsp : _airspaces_near) {
        LKASSERT(pAsp->Type() < AIRSPACECLASSCOUNT);
        LKASSERT(pAsp->Type() >= 0);

        if (!MapWindow::aAirspaceMode[pAsp->Type()].display()) {
            continue;
        }
        if (iNoFoundAS >= (iMaxNoAs - 1)) {
            continue;
        }

        if (pAsp->CheckVisible()) {
            for (unsigned i = 0; i < AIRSPACE_SCANSIZE_X; i++) {
                if (pAsp->IsHorizontalInside(lons[i], lats[i])) {
                    BOOL bPrevIn = false;
                    if (i > 0)
                        if (pAsp->IsHorizontalInside(lons[i - 1], lats[i - 1]))
                            bPrevIn = true;

                    if (!bPrevIn)/* new AS section in this view*/ {
                        /*********************************************************************
                         * switch to next airspace section
                         *********************************************************************/
                        iSelAS = iNoFoundAS;
                        BUGSTOP_LKASSERT(iNoFoundAS < MAX_NO_SIDE_AS);
                        if (iNoFoundAS < MAX_NO_SIDE_AS - 1) {
                            iNoFoundAS++;
                        }
                        airspacetype[iNoFoundAS].psAS = nullptr; // increment and reset head
                        /*********************************************************************/
                        auto& SelAS = airspacetype[iSelAS];

                        SelAS.iIdx = iSelAS;
                        SelAS.psAS = pAsp;
                        SelAS.iType = pAsp->Type();

                        lk::strcpy(SelAS.szAS_Name, pAsp->Name(), NAME_SIZE - 1);

                        SelAS.bRectAllowed = true;
                        SelAS.bEnabled = pAsp->Enabled();
                        /**********************************************************************
                         * allow rectangular shape if no AGL reference
                         **********************************************************************/
                        if (pAsp->Top().agl() || pAsp->Base().agl()) {
                            SelAS.bRectAllowed = false;
                        }
                        /**********************************************************************
                         * init with minium rectangle right side may be extended
                         **********************************************************************/
                        SelAS.rc.left = i;
                        SelAS.rc.right = i + 1;
                        SelAS.rc.bottom = (unsigned int) pAsp->Base().altitude(0);
                        SelAS.rc.top = (unsigned int) pAsp->Top().altitude(0);
                        SelAS.iNoPolyPts = 0;

                    }

                    auto& SelAS = airspacetype[iSelAS];

                    SelAS.rc.right = i + 1;
                    if (i == AIRSPACE_SCANSIZE_X - 1) {
                        SelAS.rc.right = i + 3;
                    }

                    if ((SelAS.psAS) && (!SelAS.bRectAllowed)) {
                      int iHeight = SelAS.psAS->Base().altitude(terrain_heights[i]);

                      LKASSERT((SelAS.iNoPolyPts) < GC_MAX_POLYGON_PTS);
                      SelAS.apPolygon[SelAS.iNoPolyPts++] = (POINT){(LONG)i, (LONG)iHeight};

                      /************************************************************
                       *  resort and copy polygon array
                       **************************************************************/
                      bool bLast = false;
                      if (i == AIRSPACE_SCANSIZE_X - 1) {
                        bLast = true;
                      }
                      else {
                        bLast = !SelAS.psAS->IsHorizontalInside(lons[i + 1], lats[i + 1]);
                      }

                        if (bLast) {
                            SelAS.apPolygon[SelAS.iNoPolyPts].x = i + 1;
                            if (i == AIRSPACE_SCANSIZE_X - 1) {
                                SelAS.apPolygon[SelAS.iNoPolyPts].x = i + 3;
                            }

                            LKASSERT((SelAS.iNoPolyPts) < GC_MAX_POLYGON_PTS);
                            SelAS.apPolygon[SelAS.iNoPolyPts].y = SelAS.apPolygon[SelAS.iNoPolyPts - 1].y;
                            SelAS.iNoPolyPts++;
                            LKASSERT((SelAS.iNoPolyPts) < GC_MAX_POLYGON_PTS);
                            int iN = SelAS.iNoPolyPts;
                            int iCnt = SelAS.iNoPolyPts;


                            for (int iPt = 0; iPt < iN; iPt++) {
                                LKASSERT(iCnt >= 0);
                                LKASSERT(iCnt < GC_MAX_POLYGON_PTS);

                                SelAS.apPolygon[iCnt] = SelAS.apPolygon[iN - iPt - 1];
                                SelAS.apPolygon[iCnt].y = SelAS.psAS->Top().altitude(terrain_heights[SelAS.apPolygon[iCnt].x]);

                                LKASSERT(iCnt >= iPt);
                                LKASSERT(iPt >= 0);
                                LKASSERT(iPt < GC_MAX_POLYGON_PTS);
                                if (iCnt == 0) {
                                    SelAS.rc.bottom = SelAS.apPolygon[0].y;
                                    SelAS.rc.top = SelAS.apPolygon[0].y;
                                } else {
                                    SelAS.rc.bottom = min(SelAS.rc.bottom, SelAS.apPolygon[iCnt].y);
                                    SelAS.rc.top = max(SelAS.rc.top, SelAS.apPolygon[iCnt].y);
                                }
                                if (iCnt < GC_MAX_POLYGON_PTS - 1)
                                    iCnt++;
                            }
                            LKASSERT(iCnt < GC_MAX_POLYGON_PTS);
                            SelAS.apPolygon[iCnt++] = SelAS.apPolygon[0];
                            SelAS.iNoPolyPts = iCnt;
                        }
                    }
                    RECT rcs = SelAS.rc;
                    SelAS.iAreaSize = abs(rcs.right - rcs.left) * abs(rcs.top - rcs.bottom);
                } // inside
            } // finished scanning range
        } // if overlaps bounds
    } // for iterator

    return iNoFoundAS;
}

struct airspace_sorter {
  bool operator()(const CAirspacePtr& a, const CAirspacePtr& b) {
    // TODO : Check for ordering
    return (a->Top().altitude(0) < b->Top().altitude(0));
  }
};

void CAirspaceManager::SortAirspaces(void) {
    TestLog(_T(". SortAirspace"));

    // Sort by top altitude for drawing
    ScopeLock guard(_csairspaces);
    std::sort(_airspaces.begin(), _airspaces.end(), airspace_sorter());
}

bool CAirspaceManager::ValidAirspaces(void) const {
    ScopeLock guard(_csairspaces);
    return !_airspaces.empty();
}


// Special function call for calculating warnings on flyzones. Called from CAirspace, mutex already locked!

bool CAirspaceManager::AirspaceWarningIsGoodPosition(float longitude, float latitude, int alt, int agl) const {
    if (agl < 0) agl = 0; // Limit alt to surface
    for (const auto& pAsp : _airspaces_of_interest) {
        if ((!pAsp->Flyzone()) && (pAsp->WarningAckLevel() == awNone)) {
            continue;
        }
        // Check for altitude
        if (pAsp->IsAltitudeInside(alt, agl)) {
            if (pAsp->IsHorizontalInside(longitude, latitude)) {
                return true;
            }
        }
    }
    return false;
}

void CAirspaceManager::AirspaceWarning(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
    static int step = 0;
    static double bearing = 0;
    static double interest_radius = 0;
    static rectObj bounds = {};
    static double lon = 0;
    static double lat = 0;

    // We need a valid GPS fix in FLY mode
    if (Basic->NAVWarning && !SIMMODE) {
        return;
    }

    ScopeLock guard(_csairspaces);
    if (_airspaces.empty()) {
        return; // no airspaces no nothing to do
    }

    AutoDisable(*Basic);

    switch (step) {
        default:
        case 0:
            // MULTICALC STEP 1
            // Calculate area of interest
            interest_radius = Basic->Speed * WarningTime * 1.25; // 25% more than required
            if (interest_radius < 10000) interest_radius = 10000; // but minimum 15km
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
            if ((bounds.minx< -90) && (bounds.maxx > 90)) {
                std::swap(bounds.minx, bounds.maxx);
            }

            // Step1 Init calculations
            CAirspace::StartWarningCalculation(Basic, Calculated);

            // Step2 select airspaces in range, and do warning calculations on it, add to interest list
            _airspaces_of_interest.clear();
            for (const auto& pAsp : _airspaces_near) {
                // Check for warnings enabled for this class
                if (MapWindow::aAirspaceMode[pAsp->Type()].warning()) {
                    pAsp->ResetWarnings();
                    continue;
                }
                // Check if in interest area
                if (!msRectOverlap(&bounds, &pAsp->Bounds())) {
                    pAsp->ResetWarnings();
                    continue;
                }
                // Check if it enabled
                if (!pAsp->Enabled()) {
                    pAsp->ResetWarnings();
                    continue;
                }

                pAsp->CalculateWarning(Basic, Calculated);
                _airspaces_of_interest.push_back(pAsp);
            }
            ++step;
            break;

        case 1:
            // MULTICALC STEP 2
            // Step3 Run warning fsms, refine warnings in fly zones, collect user messages
            if (AIRSPACEWARNINGS) { // Pass warning messages only if warnings enabled
                for (const auto& pAsp : _airspaces_of_interest) {
                    if (pAsp->FinishWarning()) {
                        _user_warning_queue.push({ pAsp, pAsp->WarningEvent(), pAsp->WarningLevel() });
                    }
                }
            }

            // Give the sideview the nearest instance calculated
            _sideview_nearest = CAirspace::GetSideviewNearestInstance();

            // Fill infoboxes - Nearest horizontal
#ifndef LKAIRSP_INFOBOX_USE_SELECTED
            if (CAirspace::GetNearestHName() != NULL) {
                LK_tcsncpy(NearestAirspaceName, CAirspace::GetNearestHName(), NAME_SIZE);
                NearestAirspaceHDist = CAirspace::GetNearestHDistance();
            } else {
                NearestAirspaceName[0] = 0;
                NearestAirspaceHDist = 0;
            }
            // Fill infoboxes - Nearest vertical
            if (CAirspace::GetNearestVName() != NULL) {
                LK_tcsncpy(NearestAirspaceVName, CAirspace::GetNearestVName(), NAME_SIZE);
                NearestAirspaceVDist = CAirspace::GetNearestVDistance();
            } else {
                NearestAirspaceVName[0] = 0;
                NearestAirspaceVDist = 0;
            }
#endif

#ifdef LKAIRSP_INFOBOX_USE_SELECTED
            auto pAsp = _selected_airspace.lock();
            if (!pAsp) {
                pAsp = _sideview_nearest.lock();
            }
            if (pAsp) {
                pAsp->CalculateDistance(NULL, NULL, NULL);
                lk::strcpy(NearestAirspaceName, pAsp->Name());
                NearestAirspaceHDist = pAsp->LastCalculatedHDistance();
                lk::strcpy(NearestAirspaceVName, pAsp->Name());
                NearestAirspaceVDist = pAsp->LastCalculatedVDistance();
            }
            else {
                NearestAirspaceName[0] = 0;
                NearestAirspaceHDist = 0;
                NearestAirspaceVName[0] = 0;
                NearestAirspaceVDist = 0;
            }
#endif
            step = 0;
            break;

    } // sw step
}

CAirspaceList CAirspaceManager::GetVisibleAirspacesAtPoint(const double &lon, const double &lat) const {
    CAirspaceList res;
    ScopeLock guard(_csairspaces);
    for (const auto& pAsp : _airspaces) {
        if (pAsp->DrawStyle()) {
            if (pAsp->IsHorizontalInside(lon, lat)) {
                res.push_back(pAsp);
            }
        }
    }
    return res;
}

CAirspaceList CAirspaceManager::GetNearAirspacesAtPoint(const double &lon, const double &lat, long searchrange) const {
    CAirspaceList res;
    ScopeLock guard(_csairspaces);
    for (const auto& pAsp : _airspaces) {
        if (pAsp->DrawStyle() || !pAsp->Top().below_msl()) {
            int HorDist;
            pAsp->CalculateDistance(&HorDist, nullptr, nullptr, lon, lat);
            if (HorDist < searchrange) {
                res.push_back(pAsp);
            }
        }
    }
    return res;
}

void CAirspaceManager::SetFarVisible(const rectObj &bounds_active) {
    ScopeLock guard(_csairspaces);
    _airspaces_near.clear();
    for (const auto& pAsp : _airspaces) {
        // Check if airspace overlaps given bounds
        if ((msRectOverlap(&bounds_active, &(pAsp->Bounds())) == MS_TRUE)) {
            _airspaces_near.push_back(pAsp);
        }
    }
}

void CAirspaceManager::CalculateScreenPositionsAirspace(const rectObj &screenbounds_latlon, const airspace_mode_array& aAirspaceMode, const int iAirspaceBrush[], const RECT& rcDraw, const ScreenProjection& _Proj) {
    ScopeLock guard(_csairspaces);
    for (auto asp : _airspaces_near) {
        asp->CalculateScreenPosition(screenbounds_latlon, aAirspaceMode, iAirspaceBrush, rcDraw, _Proj);
    }
}

const CAirspaceList& CAirspaceManager::GetNearAirspacesRef() const {
    return _airspaces_near;
}

const CAirspaceList CAirspaceManager::GetAllAirspaces() const {
    ScopeLock guard(_csairspaces);
    return _airspaces;
}

// Comparer to sort airspaces based on label priority for drawing labels
struct airspace_label_priority_sorter {
  bool operator()(const CAirspacePtr& a, const CAirspacePtr& b) {
    return a->LabelPriority() > b->LabelPriority();
  }
};

// Get airspaces list for label drawing

const CAirspaceList& CAirspaceManager::GetAirspacesForWarningLabels() {
    ScopeLock guard(_csairspaces);
    std::sort(_airspaces_of_interest.begin(), _airspaces_of_interest.end(), airspace_label_priority_sorter());
    return _airspaces_of_interest;
}

// Feedback from mapwindow DrawAirspaceLabels to set a round-robin priority

void CAirspaceManager::AirspaceWarningLabelPrinted(CAirspace &airspace, bool success) {
    ScopeLock guard(_csairspaces);
    if (success) airspace.LabelPriorityZero();
    else airspace.LabelPriorityInc();
}

// Gets a list of airspaces which has a warning or an ack level different than awNone

CAirspaceList CAirspaceManager::GetAirspacesInWarning() const {
    CAirspaceList res;
    ScopeLock guard(_csairspaces);
    for (const auto& pAsp : _airspaces_near) {
        if (pAsp->WarningLevel() > awNone || pAsp->WarningAckLevel() > awNone) {
            res.push_back(pAsp);
        }
    }
    return res;
}

// Gets an airspace object instance copy for a given airspace
// to display instance attributes
// NOTE: virtual methods don't work on copied instances!
//       they have to be mapped through airspacemanager class because of the mutex

CAirspaceBase CAirspaceManager::GetAirspaceCopy(const CAirspacePtr& airspace) const {
    if(airspace) {
        ScopeLock guard(_csairspaces);
        return *airspace;
    }
    return {};
}

// Calculate distances from a given airspace

bool CAirspaceManager::AirspaceCalculateDistance(const CAirspacePtr& airspace, int *hDistance, int *Bearing, int *vDistance) {
    ScopeLock guard(_csairspaces);
    return airspace->CalculateDistance(hDistance, Bearing, vDistance);
}

// Gets an airspace warning message to show

bool CAirspaceManager::PopWarningMessage(AirspaceWarningMessage *msg) {
    if (msg == NULL) return false;

    ScopeLock guard(_csairspaces);
    if(_user_warning_queue.empty()) {
      return false;
    }

    *msg = _user_warning_queue.top();
    _user_warning_queue.pop(); // remove message from queue
    return true;
}


void CAirspaceManager::AirspaceApply(CAirspace &airspace, std::function<void(CAirspace&)> func) {
    ScopeLock guard(_csairspaces);
    if (!AirspaceAckAllSame) {
        func(airspace);
    }
    else {
        for (const auto& pAsp : _airspaces) {
            if (pAsp->IsSame(airspace)) {
                func(*pAsp);
            }
        }
    }
}

// Ack an airspace for a given ack level and acknowledgement timeout
void CAirspaceManager::AirspaceSetAckLevel(CAirspace &airspace, AirspaceWarningLevel_t ackstate) {
    AirspaceApply(airspace, [&](CAirspace& asp) {
        asp.WarningAckLevel(ackstate);
        asp.SetAckTimeout();
    });
}

// Ack an airspace for a current level
void CAirspaceManager::AirspaceAckWarn(CAirspace &airspace) {
    AirspaceSetAckLevel(airspace, airspace.WarningLevel());
}

// Ack an airspace for all future warnings
void CAirspaceManager::AirspaceAckSpace(CAirspace &airspace) {
    AirspaceSetAckLevel(airspace, awRed);
}

// Disable an airspace
void CAirspaceManager::AirspaceDisable(CAirspace &airspace) {
    AirspaceApply(airspace, [&](CAirspace& asp) {
        asp.Enabled(false);
    });
}

// Enable an airspace
void CAirspaceManager::AirspaceEnable(CAirspace &airspace) {
    AirspaceApply(airspace, [&](CAirspace& asp) {
        asp.Enabled(true);
    });
}

// Toggle flyzone on an airspace
void CAirspaceManager::AirspaceFlyzoneToggle(CAirspace &airspace) {
    AirspaceApply(airspace, [&](CAirspace& asp) {
        asp.FlyzoneToggle();
    });
}

// Centralized function to get airspace type texts

const TCHAR* CAirspaceManager::GetAirspaceTypeText(int type) {
    switch (type) {
        case RESTRICT:
            // LKTOKEN  _@M565_ = "Restricted"
            return MsgToken<565>();
        case PROHIBITED:
            // LKTOKEN  _@M537_ = "Prohibited"
            return MsgToken<537>();
        case DANGER:
            // LKTOKEN  _@M213_ = "Danger Area"
            return MsgToken<213>();
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
            return MsgToken<464>();
        case CTR:
            return TEXT("CTR");
        case WAVE:
            // LKTOKEN  _@M794_ = "Wave"
            return MsgToken<794>();
        case AATASK:
            return TEXT("AAT");
        case CLASSTMZ:
            return TEXT("TMZ");
        case CLASSRMZ:
            return TEXT("RMZ");
        case CLASSNOTAM:
            return TEXT("NOTAM");
        case GLIDERSECT:
            return TEXT("GldSect");
        case OTHER:
            // LKTOKEN  _@M765_ = "Unknown"
            return MsgToken<765>();
        default:
            return TEXT("");
    }
}

// Centralized function to get airspace type texts in short form

const TCHAR* CAirspaceManager::GetAirspaceTypeShortText(int type) {
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
        case CLASSRMZ:
            return TEXT("RMZ");
        case CLASSNOTAM:
            return TEXT("Notam");
        case GLIDERSECT:
            return TEXT("GldSec");
        default:
            return TEXT("?");
    }
}

// Operations for nearest page 2.4
// Because of the multicalc approach, we need to store multicalc state inside airspacemanager
// in this case not need to have a notifier facility if airspace list changed during calculations

void CAirspaceManager::SelectAirspacesForPage24(const double latitude, const double longitude, const double interest_radius) {

    ScopeLock guard(_csairspaces);
    if (_airspaces.size() < 1) return;

    // Calculate area of interest
    double lon = longitude;
    double lat = latitude;
    rectObj bounds = { lon, lon, lat, lat };

    {
        FindLatitudeLongitude(latitude, longitude, 0, interest_radius, &lat, &lon);
        bounds.minx = min(lon, bounds.minx);
        bounds.maxx = max(lon, bounds.maxx);
        bounds.miny = min(lat, bounds.miny);
        bounds.maxy = max(lat, bounds.maxy);
    }

    {
        FindLatitudeLongitude(latitude, longitude, 90, interest_radius, &lat, &lon);
        bounds.minx = min(lon, bounds.minx);
        bounds.maxx = max(lon, bounds.maxx);
        bounds.miny = min(lat, bounds.miny);
        bounds.maxy = max(lat, bounds.maxy);
    }

    {
        FindLatitudeLongitude(latitude, longitude, 180, interest_radius, &lat, &lon);
        bounds.minx = min(lon, bounds.minx);
        bounds.maxx = max(lon, bounds.maxx);
        bounds.miny = min(lat, bounds.miny);
        bounds.maxy = max(lat, bounds.maxy);
    }

    {
        FindLatitudeLongitude(latitude, longitude, 270, interest_radius, &lat, &lon);
        bounds.minx = min(lon, bounds.minx);
        bounds.maxx = max(lon, bounds.maxx);
        bounds.miny = min(lat, bounds.miny);
        bounds.maxy = max(lat, bounds.maxy);
    }

    // JMW detect airspace that wraps across 180
    if ((bounds.minx< -90) && (bounds.maxx > 90)) {
        std::swap(bounds.minx, bounds.maxx);
    }

    // Select nearest ones (based on bounds)
    _airspaces_page24.clear();
    for (const auto& pAsp : _airspaces) {
        if (msRectOverlap(&bounds, &(pAsp->Bounds())) == MS_TRUE) {
            _airspaces_page24.push_back(pAsp);
        }
    }
}

void CAirspaceManager::CalculateDistancesForPage24() {
    ScopeLock guard(_csairspaces);
    for (const auto& pAsp : _airspaces_page24) {
        pAsp->CalculateDistance(nullptr, nullptr, nullptr);
    }
}

// Set or change or deselect selected airspace
void CAirspaceManager::AirspaceSetSelect(const CAirspacePtr &airspace) {
    ScopeLock guard(_csairspaces);
    auto old_selected = _selected_airspace.lock();
    // Deselect if we get the same asp
    if (old_selected == airspace) {
        old_selected->Selected(false);
        _selected_airspace.reset();
        return;
    }

    if (old_selected) {
        old_selected->Selected(false);
    }

    _selected_airspace = airspace;
    airspace->Selected(true);
}

// Save airspace settings

void CAirspaceManager::SaveSettings() const {
    FILE *f;
    TCHAR szFileName[MAX_PATH];
    char buf[MAX_PATH + 1];
    TCHAR ubuf[(MAX_PATH * 2) + 1];

    LocalPath(szFileName, TEXT(LKF_AIRSPACE_SETTINGS));
    f = _tfopen(szFileName, TEXT("w"));
    if (f != NULL) {
        // File header
        fprintf(f, "# LK8000 AIRSPACE SETTINGS\n");
        fprintf(f, "# THIS FILE IS GENERATED AUTOMATICALLY ON LK SHUTDOWN - DO NOT ALTER BY HAND, DO NOT COPY BEETWEEN DEVICES!\n");

        ScopeLock guard(_csairspaces);
        for (const auto& pAsp : _airspaces) {
            std::string hash = pAsp->Hash();
            //Asp hash
            fprintf(f, "%32s ", hash.c_str());

            //Settings chr1 - Flyzone or not
            if (pAsp->Flyzone()) fprintf(f, "F");
            else fprintf(f, "-");
            //Settings chr2 - Enabled or not
            if (pAsp->Enabled()) fprintf(f, "E");
            else fprintf(f, "-");
            //Settings chr3 - Selected or not
            if (pAsp->Selected()) fprintf(f, "S");
            else fprintf(f, "-");

            //Comment
            _stprintf(ubuf, TEXT(" #%s"), pAsp->Name());
            to_utf8(ubuf, buf);
            fprintf(f, "%s", buf);
            //Newline
            fprintf(f, "\n");
        }
        StartupStore(TEXT(". Settings for %u airspaces saved to file <%s>"), (unsigned)_airspaces.size(), szFileName);
        fclose(f);
    } 
    else {
        StartupStore(TEXT("Failed to save airspace settings to file <%s>"), szFileName);
    }
}

// Load airspace settings

void CAirspaceManager::LoadSettings() {
    char linebuf[MAX_PATH + 1];
    char hash[MAX_PATH + 1];
    char flagstr[MAX_PATH + 1];
    TCHAR szFileName[MAX_PATH];

    unsigned int retval;
    unsigned int airspaces_restored = 0;

    LocalPath(szFileName, TEXT(LKF_AIRSPACE_SETTINGS));
    FILE *f = _tfopen(szFileName, TEXT("r"));
    if (f != NULL) {
        // Generate hash map on loaded airspaces
        ScopeLock guard(_csairspaces);

        std::map<std::string, CAirspacePtr> map;
        for (const auto& pAsp : _airspaces) {
            map.emplace(pAsp->Hash(), pAsp);
        }

        while (fgets(linebuf, MAX_PATH, f) != NULL) {
            //Parse next line
            retval = sscanf(linebuf, "%32s %3s", hash, flagstr);
            if (retval == 2 && hash[0] != '#') {

                auto it = map.find(hash);
                if (it != map.end()) {
                    it->second->Flyzone(flagstr[0] == 'F');
                    it->second->Enabled(flagstr[1] == 'E');
                    if (flagstr[2] == 'S') {
                        AirspaceSetSelect(it->second);
                    }
                    map.erase(it);
                    airspaces_restored++;
                }
            }
        }
        StartupStore(TEXT(". Settings for %d of %u airspaces loaded from file <%s>"), airspaces_restored, (unsigned)_airspaces.size(), szFileName);
        fclose(f);
    } 
    else {
        StartupStore(TEXT(". Failed to load airspace settings from file <%s>"), szFileName);
    }
}


#if ASPWAVEOFF

void CAirspaceManager::AirspaceDisableWaveSectors() {
    ScopeLock guard(_csairspaces);

    for (const auto& pAsp : _airspaces) {
        if (pAsp->Type() == WAVE) {
            pAsp->Enabled(false);
            pAsp->Flyzone(true);

            DebugLog(TEXT("LKAIRSP: %s AirspaceDisable()"), pAsp->Name());
        }
    }

}
#endif


void CAirspaceManager::AutoDisable(const NMEA_INFO& info) {
    if (info.NAVWarning) {
        // no valid date without valid gps fix
        last_day_of_week = ~0;
        return;
    }

    unsigned current = day_of_week(to_time_t(info), GetUTCOffset());
    if (last_day_of_week != current) {
        last_day_of_week = current;

        for (auto asp : _airspaces) {
            if (asp->ExceptSaturday()) {
                asp->Enabled(current != 5); // Saturday
            }
            if (asp->ExceptSunday()) {
                asp->Enabled(current != 6); // Sunday
            }
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
// Draw Picto methods
//  this methods are NEVER used at same time of airspace loading
//  therefore we can be considered is thread safe.

void CAirspace::DrawPicto(LKSurface& Surface, const RECT &rc) const {
    ScopeLock guard(CAirspaceManager::Instance().MutexRef());

    RasterPointList screenpoints_picto;
    CalculatePictPosition(rc, 0.9, screenpoints_picto);

    size_t Length = screenpoints_picto.size();
    if (Length > 2) {

        const RasterPoint * ptOut = &(*screenpoints_picto.begin());

        LKPen FramePen(PEN_SOLID, IBLSCALE(1), TypeColor());

        const auto oldColor = Surface.SetTextColor(TypeColor());

        const auto oldPen = Surface.SelectObject(FramePen);
        bool Fill = Enabled();
        if(Acknowledged()) {
          Fill = false;
        };

        const auto oldBrush = Surface.SelectObject(Fill ? TypeBrush() : LKBrush_Hollow);

        Surface.Polygon(ptOut, Length);

        Surface.SelectObject(oldBrush);
        Surface.SelectObject(oldPen);
        Surface.SetTextColor(oldColor);
    }
}

void CAirspace::CalculatePictPosition(const RECT& rcDraw, double zoom, RasterPointList& screenpoints_picto) const {
    LKASSERT(FALSE); // never call this function on base class instance !
}

void CAirspace_Circle::CalculatePictPosition(const RECT& rcDraw, double zoom, RasterPointList& screenpoints_picto) const {
    const int cx = rcDraw.right - rcDraw.left;
    const int cy = rcDraw.bottom - rcDraw.top;
    const int radius = iround(((double) ((cy < cx) ? cy : cx) / 2.0) * zoom);
    const RasterPoint center = {rcDraw.left + cx / 2, rcDraw.top + cy / 2};

    LKSurface::buildCircle(center, radius, screenpoints_picto);
}

void CAirspace_Area::CalculatePictPosition(const RECT& rcDraw, double zoom, RasterPointList& screenpoints_picto) const {
    screenpoints_picto.clear();

    const double dlon = _bounds.maxx - _bounds.minx;
    const double dlat = _bounds.maxy - _bounds.miny;
    if (dlon == 0. || dlat == 0.) {
        BUGSTOP_LKASSERT(FALSE); // wrong aispaces shape ( bounding box is line or point )
        return;
    }

    const double PanLatitudeCenter = _bounds.miny + dlat / 2.;
    if (fastcosine(PanLatitudeCenter) == 0) {
        BUGSTOP_LKASSERT(FALSE); // wrong aispaces shape ( center of airspace at the pole ? )
        return;
    }

    const double PanLongitudeCenter = _bounds.minx + dlon / 2.;

    const int cx = (rcDraw.right - rcDraw.left);
    const int cy = (rcDraw.bottom - rcDraw.top);
    const int xoff = rcDraw.left + cx / 2;
    const int yoff = rcDraw.top + cy / 2;

    const double scaleX = cx / dlon * zoom / fastcosine(PanLatitudeCenter);
    const double scaleY = cy / dlat * zoom;
    const double scale = (scaleX < scaleY) ? scaleX : scaleY;

    screenpoints_picto.reserve(_geopoints.size());
    for (const auto point : _geopoints) {
        screenpoints_picto.emplace_back(
            xoff - iround((PanLongitudeCenter - point.Longitude()) * fastcosine(point.Latitude()) * scale),
            yoff + iround((PanLatitudeCenter - point.Latitude()) * scale));
    }
}
////////////////////////////////////////////////////////////////////////////////

LKAirspace_Nearest_Item& LKAirspace_Nearest_Item::operator=(const CAirspacePtr& pAsp) {
    Pointer = pAsp;
    if (pAsp) {
        Valid = true;
        Distance = std::max(pAsp->LastCalculatedHDistance(), 0);
        Bearing = pAsp->LastCalculatedBearing();
        Enabled = pAsp->Enabled();
        Selected = pAsp->Selected();
        Flyzone = pAsp->Flyzone();
        WarningLevel = pAsp->WarningLevel();
        WarningAckLevel = pAsp->WarningAckLevel();
        // Copy name and type
        lk::strcpy(Name, pAsp->Name());
        lk::strcpy(Type, pAsp->TypeNameShort());
    } else {
        Valid = false;
    }
    return *this; 
}
