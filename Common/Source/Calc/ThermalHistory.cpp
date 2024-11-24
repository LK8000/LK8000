/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: ThermalHistory.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "ThermalHistory.h"
#include "McReady.h"
#include "Waypointparser.h"
#include "Library/TimeFunctions.h"
#include "Time/GPSClock.hpp"
#include "utils/printf.h"

//
// Thermal History functions
//

namespace {

Mutex lst_mutex;

// The Thermal History internal database
std::vector<THERMAL_HISTORY> ThermalHistory;

// This is holding the thermal index selected for multitarget
std::optional<size_t> ThermalMultitarget;

std::function<double(const THERMAL_HISTORY&)>
sort_mode_to_value_getter(const NMEA_INFO* Basic) {
  switch (SortedMode[MSM_THERMALS]) {
    case 0:
      return [](const THERMAL_HISTORY& Therm) {
        return Therm.Time * -1;
      };
    default:
    case 1:
      return [](const THERMAL_HISTORY& Therm) {
        return Therm.Distance;
      };
    case 2:
      if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
        return [](const THERMAL_HISTORY& Therm) {
          return Therm.Bearing;
        };
      }
      else {
        return [&](const THERMAL_HISTORY& Therm) {
          return std::abs(AngleLimit180(Therm.Bearing - Basic->TrackBearing));
        };
      }
    case 3:
      return [](const THERMAL_HISTORY& Therm) {
        return Therm.Lift * -1;
      };
    case 4:
      return [](const THERMAL_HISTORY& Therm) {
        return Therm.Arrival * -1;
      };
  }
}

const TCHAR* NearestWaypointName(const GeoPoint& position) {
  int j = FindNearestFarVisibleWayPoint(position.longitude, position.latitude, 15000, WPT_UNKNOWN);
  if (j > 0) {
    return WayPointList[j].Name;
  }
  return _T("");
}

} // namespace


void InitThermalHistory() {
  WithLock(lst_mutex, [&] {
    ThermalHistory.clear();
    ThermalMultitarget.reset();
  });

  CopyThermalHistory.clear();

  // Number of Thermals updated from DoThermalHistory
  LKNumThermals = 0;
  LKSortedThermals.clear();
}

// This function is called upon exiting a thermal, just before input event to cruise mode is issued
// Functions using the ThermalHistory array should always check for Valid flag at each step,
// and in any case never use the array while Circling. Because anytime we may change a value, in case
// (very rare) the 100 limit is passed and we are reusing a value.
//
// When this function is called, the L> thermal is automatically selected as multitarget L
//
void InsertThermalHistory(double ThTime, const GeoPoint& position, double ThBase, double ThTop, double ThAvg, bool SetMultiTarget) {
  TCHAR timestr[10];
  Units::TimeToTextSimple(timestr, LocalTime(ThTime));

  auto idx = WithLock(lst_mutex, [&] {

    ThermalHistory.push_back({
        tstring(_T("th")) + timestr,
        tstring(NearestWaypointName(position)),
        ThTime,
        position,
        ThBase,
        ThTop,
        ThAvg,
        0., 0., 0.
    });

    return ThermalHistory.size() - 1;
  });

  if (SetMultiTarget) {
    // Update holder selected L>
    SetThermalMultitarget(idx, MsgToken<1320>());
  }
}

bool IsThermalExist(const GeoPoint& position, double radius) {
  ScopeLock lock(lst_mutex);
  auto it = std::find_if(ThermalHistory.begin(), ThermalHistory.end(), [&](auto& th) {
    return position.Distance(th.position) < radius;
  });
  return it != ThermalHistory.end();
}

// Running every n seconds ONLY when the thermal history page is active and we are not drawing map.
// Returns true if did calculations, false if ok to use old values.
// Warning, this function is run by Draw thread.
bool DoThermalHistory(NMEA_INFO* Basic, DERIVED_INFO* Calculated) {
  static GPSClock clock_runtime;

  if (LastDoThermalH > 0) {
    /* Wait for n seconds before updating again, to avoid data change too often
       distracting the pilot. */
    if (!clock_runtime.CheckAdvance(Basic->Time, NEARESTUPDATETIME)) {
      return false;
    }
  }

  // Consider replay mode...
  if (LastDoThermalH > Basic->Time) {
    LastDoThermalH = Basic->Time;
  }
  // Dont recalculate while user is using the virtual keys
  if (Basic->Time < (LastDoThermalH + PAGINGTIMEOUT)) {
    return false;
  }

  LastDoThermalH = Basic->Time;

  CopyThermalHistory = WithLock(lst_mutex, []() {
    return ThermalHistory;
  });

  LKNumThermals = CopyThermalHistory.size();
  LKSortedThermals.clear();
  if (CopyThermalHistory.empty()) {
    return true;
  }

  LKSortedThermals.reserve(LKNumThermals);

  std::vector<double> sortedValue;
  sortedValue.reserve(LKNumThermals);

  for (THERMAL_HISTORY& Therm : CopyThermalHistory) {
    DistanceBearing(Basic->Latitude, Basic->Longitude,
                    Therm.position.latitude, Therm.position.longitude,
                    &Therm.Distance, &Therm.Bearing);

    double altReqd = GlidePolar::MacCreadyAltitude(MACCREADY,
                            Therm.Distance, Therm.Bearing,
                            Calculated->WindSpeed, Calculated->WindBearing,
                            0, 0, true, nullptr);

    Therm.Arrival = Calculated->NavAltitude + Calculated->EnergyHeight - altReqd - Therm.HBase;
  }

  auto get_value = sort_mode_to_value_getter(Basic);

  // We know there is at least one thermal
  for (size_t i = 0; i < CopyThermalHistory.size(); ++i) {
    double sortvalue = get_value(CopyThermalHistory[i]);

    auto it = std::upper_bound(sortedValue.begin(), sortedValue.end(), sortvalue);
    sortedValue.insert(it, sortvalue);

    auto insert_index = std::distance(sortedValue.begin(), it);
    auto insert_iterator = std::next(LKSortedThermals.begin(), insert_index);
    LKSortedThermals.insert(insert_iterator, i);
  }  // for i

  return true;
}

bool IsThermalMultitarget(size_t idx) {
  ScopeLock lock(lst_mutex);
  if (ThermalMultitarget && idx < ThermalHistory.size()) {
    return ThermalMultitarget.value() == idx;
  }
  return false;
}

void SetThermalMultitarget(size_t idx, const TCHAR* Comment) {
  try {
    auto thermal = WithLock(lst_mutex, [&] {
      if (idx >= ThermalHistory.size()) {
        throw std::runtime_error("error : invalid thermal index");
      }
      ThermalMultitarget = idx;
      return ThermalHistory[idx];
    });

    TestLog(_T("... Selected thermal n.%zu <%s>"), idx, thermal.Name.c_str());

    WithLock(CritSec_TaskData, [&] {
      WayPointList[RESWP_LASTTHERMAL].Latitude  = thermal.position.latitude;
      WayPointList[RESWP_LASTTHERMAL].Longitude = thermal.position.longitude;
      WayPointList[RESWP_LASTTHERMAL].Altitude  = thermal.HBase;
      lk::snprintf(WayPointList[RESWP_LASTTHERMAL].Name, _T("%s"), thermal.Name.c_str());
      SetWaypointComment(WayPointList[RESWP_LASTTHERMAL], Comment);
    });
  }
  catch (std::exception& e) {
    TestLog(_T("%s"), to_tstring(e.what()).c_str());
  }
}

std::optional<THERMAL_HISTORY> GetThermalHistory(size_t idx) {
  ScopeLock lock(lst_mutex);
  if (idx < ThermalHistory.size()) {
    return ThermalHistory[idx];
  }
  return {};
}

std::optional<THERMAL_HISTORY> GetThermalMultitarget() {
  ScopeLock lock(lst_mutex);
  auto idx = ThermalMultitarget.value();
  if (ThermalMultitarget && idx < ThermalHistory.size()) {
    return ThermalHistory[idx];
  }
  return {};
}

std::vector<THERMAL_HISTORY> CopyThermalHistory;

int LKNumThermals = 0;
std::vector<int> LKSortedThermals;
