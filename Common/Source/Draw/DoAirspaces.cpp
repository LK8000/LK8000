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
    int da = a->LastCalculatedHDistance();
    int db = b->LastCalculatedHDistance();

    if (beara != bearb) {
      return beara < bearb;
    }
    // if bearing is the same, get closer first
    return da < db;
  }
};

// During cruise, we sort bearing diff and use bearing diff in DrawAsp
struct airspace_bearing_diff_sorter {
  bool operator()(const CAirspacePtr& a, const CAirspacePtr& b) const {
    int beara = a->LastCalculatedBearing();
    int bearb = b->LastCalculatedBearing();
    int da = a->LastCalculatedHDistance();
    int db = b->LastCalculatedHDistance();

    int beardiffa = beara - a->LastTrackBearing();
    if (beardiffa < -180) {
      beardiffa += 360;
    } else if (beardiffa > 180) {
      beardiffa -= 360;
    }
    if (beardiffa < 0) {
      beardiffa *= -1;
    }

    int beardiffb = bearb - b->LastTrackBearing();
    if (beardiffb < -180) {
      beardiffb += 360;
    } else if (beardiffb > 180) {
      beardiffb -= 360;
    }
    if (beardiffb < 0) {
      beardiffb *= -1;
    }

    if (beardiffa != beardiffb) {
      return beardiffa < beardiffb;
    }

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
bool DoAirspaces(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{

   static int step = 0;
   bool ret = false;

   // DoAirspaces is called from MapWindow, in real time. We have enough CPU power there now
   // Consider replay mode...

   // We need a valid GPS fix in FLY mode
   if (Basic->NAVWarning && !SIMMODE) return true;

   
   #ifdef DEBUG_LKT
   StartupStore(_T("... DoAirspaces step%d started\n"),step);
   #endif


   switch (step) {
     // MULTICALC STEP0 - select airspaces in range based on bounds
     default:
     case 0:
        CAirspaceManager::Instance().SelectAirspacesForPage24(Basic->Latitude, Basic->Longitude, 100000.0);        // 100km
        ++step;
        break;
          
     // MULTICALC STEP1 - Do distance calculations on selected airspaces
     case 1:
        CAirspaceManager::Instance().CalculateDistancesForPage24();
        ++step;
        break;
        
     // MULTICALC STEP2 - Sort by different keys, and fill up result struct array
     case 2:
        // Lock airspace instances externally
        ScopeLock guard(CAirspaceManager::Instance().MutexRef());
        // Get selected list from airspacemanager
        CAirspaceList nearest_airspaces = CAirspaceManager::Instance().GetAirspacesForPage24();
        // Sort selected airspaces by distance first
        std::sort(nearest_airspaces.begin(), nearest_airspaces.end(), airspace_distance_sorter());
        // get first MAXNEARAIRSPACES to a new list
        if(nearest_airspaces.size() > MAXNEARAIRSPACES) {
            nearest_airspaces.resize(MAXNEARAIRSPACES);
        }

        //sort by given key
          switch (SortedMode[MSM_AIRSPACES]) {
              case 0: 
                  // ASP NAME
                  std::sort(nearest_airspaces.begin(), nearest_airspaces.end(), airspace_name_sorter());
                  break;
              case 1:
                  // ASP TYPE
                  std::sort(nearest_airspaces.begin(), nearest_airspaces.end(), airspace_type_sorter());
                  break;
                  
              default:
              case 2:
                  // ASP DISTANCE
                  // we don't need sorting, already sorted by distance
                  break;
              case 3:
                  // ASP BEARING
                  if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
                    std::sort(nearest_airspaces.begin(), nearest_airspaces.end(), airspace_bearing_sorter());
                  } else {
                    std::sort(nearest_airspaces.begin(), nearest_airspaces.end(), airspace_bearing_diff_sorter());
                  }
                  break;
              case 4:
                  // ACTIVE / NOT ACTIVE
                  std::sort(nearest_airspaces.begin(), nearest_airspaces.end(), airspace_enabled_sorter());
                  break;
          } //sw

          // Clear results
          LKNumAirspaces=0;
          //copy result data to interface structs
          // we dont need LKSortedAirspaces[] array, every item will be
          // in correct order in airspaces list, thanks to std::sort,
          // we just fill up LKAirspaces[] array in the right order.

          std::transform(nearest_airspaces.begin(), nearest_airspaces.end(), std::begin(LKAirspaces), [](CAirspacePtr& pAsp) {
              LKAirspace_Nearest_Item OutItem = {};

              // copy name
              LK_tcsncpy(OutItem.Name, pAsp->Name(), NAME_SIZE);
              // copy type string (type string comes from centralized type->str conversion function of CAirspaceManager)
              LK_tcsncpy(OutItem.Type, CAirspaceManager::GetAirspaceTypeShortText(pAsp->Type()), 4);

              // copy distance
              OutItem.Distance = max(pAsp->LastCalculatedHDistance(),0);
              // copy bearing
              OutItem.Bearing = pAsp->LastCalculatedBearing();
              // copy Enabled()
              OutItem.Enabled = pAsp->Enabled();
              // copy Selected()
              OutItem.Selected = pAsp->Selected();
              // copy Flyzone()
              OutItem.Flyzone = pAsp->Flyzone();
              // copy WarningLevel()
              OutItem.WarningLevel = pAsp->WarningLevel();
              // copy WarningAckLevel()
              OutItem.WarningAckLevel = pAsp->WarningAckLevel();

              // copy pointer
              OutItem.Pointer = pAsp;

              // Record is valid now.
              OutItem.Valid = true;

              LKNumAirspaces++;

              return OutItem;
          });

          std::for_each(std::next(LKAirspaces, LKNumAirspaces), std::end(LKAirspaces), [](LKAirspace_Nearest_Item& Item) {
              Item.Valid = false;
              Item.Pointer.reset();
          });

          ret = true;       // ok to use new values.
          step = 0;
          break;
   } //sw step

   return ret;
}

