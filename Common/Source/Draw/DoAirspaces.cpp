/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: DoAirspaces.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "DoInits.h"
#include <functional>

// Comparer to sort airspaces based on distance
struct airspace_distance_sorter {
  bool operator()(const CAirspacePtr& a, const CAirspacePtr& b) const {
    // nearest first
    return (a->LastCalculatedHDistance() < b->LastCalculatedHDistance());
  }
};

// Comparer to sort airspaces based on name
struct airspace_name_sorter {
  bool operator()(const CAirspacePtr& a, const CAirspacePtr& b) const {
    const int res = _tcscmp(a->Name(), b->Name());
    if (res) {
      return res < 0;
    }
    // if name is the same, get closer first
    return (a->LastCalculatedHDistance() < b->LastCalculatedHDistance());
  }
};

// Comparer to sort airspaces based on type
struct airspace_type_sorter {
  bool operator()(const CAirspacePtr& a, const CAirspacePtr& b) const {
    if (a->Type() != b->Type()) {
      return a->Type() < b->Type();
    }

    // if type is the same, get closer first
    return (a->LastCalculatedHDistance() < b->LastCalculatedHDistance());
  }
};

// Comparer to sort airspaces based on enabled
struct airspace_enabled_sorter {
  bool operator()(const CAirspacePtr& a, const CAirspacePtr& b) const {
    if (a->Enabled() != b->Enabled()) {
      return a->Enabled() < b->Enabled();
    }

    // if enabled is the same, get closer first
    return (a->LastCalculatedHDistance() < b->LastCalculatedHDistance());
  }
};

// Comparer to sort airspaces based on bearing
struct airspace_bearing_sorter {
  bool operator()(const CAirspacePtr& a, const CAirspacePtr b) const {
    int beara = a->LastCalculatedBearing();
    int bearb = b->LastCalculatedBearing();
    if (beara != bearb) {
      return beara < bearb;
    }

    int da = a->LastCalculatedHDistance();
    int db = b->LastCalculatedHDistance();
    // if bearing is the same, get closer first
    return da < db;
  }
};

// During cruise, we sort bearing diff and use bearing diff in DrawAsp
struct airspace_bearing_diff_sorter {
  bool operator()(const CAirspacePtr& a, const CAirspacePtr& b) const {
    int beara = a->LastCalculatedBearing();
    int bearb = b->LastCalculatedBearing();

    int beardiffa = std::abs(AngleLimit180(beara - a->LastTrackBearing()));
    int beardiffb = std::abs(AngleLimit180(bearb - b->LastTrackBearing()));
    // if bearing difference is different, sort by it
    if (beardiffa != beardiffb) {
      return beardiffa < beardiffb;
    }

    int da = a->LastCalculatedHDistance();
    int db = b->LastCalculatedHDistance();

    // if bearing difference is the same, get closer first
    return da < db;
  }
};

// OBSOLETED comment..
// Running every n seconds ONLY when the nearest airspace page is active and we are not drawing map.
// Returns true if did calculations, false if ok to use old values
//
// This is long since called by LKDrawNearestAsp page, using multicalc
//
bool DoAirspaces(NMEA_INFO* Basic, DERIVED_INFO* Calculated) {
  static int step = 0;

  // DoAirspaces is called from MapWindow, in real time. We have enough CPU power there now
  // Consider replay mode...

  // We need a valid GPS fix in FLY mode
  if (Basic->NAVWarning && !SIMMODE) {
    return true;
  }

  switch (step++) {
    // MULTICALC STEP0 - select airspaces in range based on bounds
    default:
    case 0:
      CAirspaceManager::Instance().SelectAirspacesForPage24(Basic->Latitude, Basic->Longitude, 100000.0);  // 100km
      break;

      // MULTICALC STEP1 - Do distance calculations on selected airspaces
    case 1:
      CAirspaceManager::Instance().CalculateDistancesForPage24();
      break;

    // MULTICALC STEP2 - Sort by different keys, and fill up result struct array
    case 2:
      // Lock airspace instances externally
      ScopeLock guard(CAirspaceManager::Instance().MutexRef());
      // Get selected list from airspacemanager
      CAirspaceList& nearest_airspaces = CAirspaceManager::Instance().GetAirspacesForPage24();

      // We need to sort the airspaces by distance first, then by name, type, etc.
      auto sorted_end = std::end(nearest_airspaces);
      if (nearest_airspaces.size() > MAXNEARAIRSPACES) {
        sorted_end = std::next(std::begin(nearest_airspaces), MAXNEARAIRSPACES);
      }
      std::partial_sort(std::begin(nearest_airspaces), std::end(nearest_airspaces),
                        sorted_end, airspace_distance_sorter());

      // now we have the nearest airspaces at the beginning of the list (between begin() and sorted_end)

      // sort it by the given key
      switch (SortedMode[MSM_AIRSPACES]) {
        case 0:
          // ASP NAME
          std::sort(std::begin(nearest_airspaces), sorted_end, airspace_name_sorter());
          break;
        case 1:
          // ASP TYPE
          std::sort(std::begin(nearest_airspaces), sorted_end, airspace_type_sorter());
          break;
        default:
        case 2:
          // ASP DISTANCE
          // we don't need sorting, already sorted by distance
          break;
        case 3:
          // ASP BEARING
          if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
            std::sort(std::begin(nearest_airspaces), sorted_end, airspace_bearing_sorter());
          } else {
            std::sort(std::begin(nearest_airspaces), sorted_end, airspace_bearing_diff_sorter());
          }
          break;
        case 4:
          // ACTIVE / NOT ACTIVE
          std::sort(std::begin(nearest_airspaces), sorted_end, airspace_enabled_sorter());
          break;
      }  // sw

      // Clear results
      LKNumAirspaces = 0;
      // copy result data to interface structs
      //  we dont need LKSortedAirspaces[] array, every item will be
      //  in correct order in airspaces list, thanks to std::sort,
      //  we just fill up LKAirspaces[] array in the right order.

      std::copy(std::begin(nearest_airspaces), sorted_end, std::begin(LKAirspaces));

      // set the rest of the array to invalid
      size_t valid_count = std::distance(std::begin(nearest_airspaces), sorted_end);
      std::fill(std::next(LKAirspaces, valid_count), std::end(LKAirspaces), nullptr);

      // set the number of valid airspaces
      LKNumAirspaces = valid_count;

      step = 0;

      return true;  // ok to use new values.
  }  // sw step

  return false;
}
