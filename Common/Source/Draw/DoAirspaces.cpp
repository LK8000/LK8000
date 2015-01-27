/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: DoAirspaces.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "DoInits.h"



// Comparer to sort airspaces based on distance
static bool airspace_distance_sorter( CAirspace *a, CAirspace *b )
{
  int da = a->LastCalculatedHDistance();
  int db = b->LastCalculatedHDistance();
  return da<db;     //nearest first
}

// Comparer to sort airspaces based on name
static bool airspace_name_sorter( CAirspace *a, CAirspace *b )
{
  int res = _tcscmp(a->Name(), b->Name());
  if (res) return res < 0;
  
  // if name is the same, get closer first
  int da = a->LastCalculatedHDistance();
  int db = b->LastCalculatedHDistance();
  return da<db;
}

// Comparer to sort airspaces based on type
static bool airspace_type_sorter( CAirspace *a, CAirspace *b )
{
  if (a->Type() != b->Type()) return a->Type() < b->Type();
  
  // if type is the same, get closer first
  int da = a->LastCalculatedHDistance();
  int db = b->LastCalculatedHDistance();
  return da<db;
}

// Comparer to sort airspaces based on enabled
static bool airspace_enabled_sorter( CAirspace *a, CAirspace *b )
{

  if (a->Enabled() != b->Enabled()) return a->Enabled() < b->Enabled();

  // if enabled is the same, get closer first
  int da = a->LastCalculatedHDistance();
  int db = b->LastCalculatedHDistance();
  return da<db;
}

// Comparer to sort airspaces based on bearing
// During cruise, we sort bearing diff and use bearing diff in DrawAsp
static bool airspace_bearing_sorter( CAirspace *a, CAirspace *b )
{
  int beara = a->LastCalculatedBearing();
  int bearb = b->LastCalculatedBearing();
  int beardiffa,beardiffb;
  int da = a->LastCalculatedHDistance();
  int db = b->LastCalculatedHDistance();

  if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
    if (beara != bearb) return beara < bearb;
    // if bearing is the same, get closer first
    return da<db;
  }
 
  beardiffa = beara - a->LastTrackBearing();
  if (beardiffa < -180) beardiffa += 360;
    else if (beardiffa > 180) beardiffa -= 360;
  if (beardiffa<0) beardiffa*=-1;
  
  beardiffb = bearb - b->LastTrackBearing();
  if (beardiffb < -180) beardiffb += 360;
    else if (beardiffb > 180) beardiffb -= 360;
  if (beardiffb<0) beardiffb*=-1;
 
  if (beardiffa != beardiffb) return beardiffa < beardiffb; 
  // if bearing difference is the same, get closer first
  return da<db;
}


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
   
   if (DoInit[MDI_DOAIRSPACES]) {
    CCriticalSection::CGuard guard(CAirspaceManager::Instance().MutexRef());
	LKNumAirspaces=0;
	memset(LKSortedAirspaces, -1, sizeof(LKSortedAirspaces));
    for (int i=0; i<MAXNEARAIRSPACES; i++) {
      LKAirspaces[i].Valid = false;
      LKAirspaces[i].Pointer = NULL;
    }
	DoInit[MDI_DOAIRSPACES]=false;
	return true;
   }

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
        CCriticalSection::CGuard guard(CAirspaceManager::Instance().MutexRef());
        // Get selected list from airspacemanager
        CAirspaceList airspaces = CAirspaceManager::Instance().GetAirspacesForPage24();
        // Sort selected airspaces by distance first
        std::sort(airspaces.begin(), airspaces.end(), airspace_distance_sorter);
        // get first MAXNEARAIRSPACES to a new list
        CAirspaceList nearest_airspaces;
        CAirspaceList::iterator it = airspaces.begin();
        for (int i=0; (i<MAXNEARAIRSPACES) && (it!=airspaces.end()); ++i, ++it) nearest_airspaces.push_back(*it);
        airspaces.clear();

        //sort by given key
          switch (SortedMode[MSM_AIRSPACES]) {
              case 0: 
                  // ASP NAME
                  std::sort(nearest_airspaces.begin(), nearest_airspaces.end(), airspace_name_sorter);
                  break;
              case 1:
                  // ASP TYPE
                  std::sort(nearest_airspaces.begin(), nearest_airspaces.end(), airspace_type_sorter);
                  break;
                  
              default:
              case 2:
                  // ASP DISTANCE
                  // we don't need sorting, already sorted by distance
                  break;
              case 3:
                  // ASP BEARING
                  std::sort(nearest_airspaces.begin(), nearest_airspaces.end(), airspace_bearing_sorter);
                  break;
              case 4:
                  // ACTIVE / NOT ACTIVE
                  std::sort(nearest_airspaces.begin(), nearest_airspaces.end(), airspace_enabled_sorter);
                  break;
          } //sw

          // Clear results
          memset(LKSortedAirspaces, -1, sizeof(LKSortedAirspaces));
          LKNumAirspaces=0;
          for (int i=0; i<MAXNEARAIRSPACES; i++) {
              LKAirspaces[i].Valid = false;
              LKAirspaces[i].Pointer = NULL;
          }
          //copy result data to interface structs
          // we dont need LKSortedAirspaces[] array, every item will be
          // in correct order in airspaces list, thanks to std::sort,
          // we just fill up LKAirspaces[] array in the right order.
          int i = 0;
          for (it=nearest_airspaces.begin(); it!=nearest_airspaces.end(); ++it) {
              // sort key not used, iterator goes in right order after std::sort
              LKSortedAirspaces[i] = i;
              // copy name
              LK_tcsncpy(LKAirspaces[i].Name, (*it)->Name(), NAME_SIZE);
              // copy type string (type string comes from centralized type->str conversion function of CAirspaceManager)
              LK_tcsncpy(LKAirspaces[i].Type, CAirspaceManager::Instance().GetAirspaceTypeShortText((*it)->Type()), 4);

              // copy distance
              LKAirspaces[i].Distance = max((*it)->LastCalculatedHDistance(),0);
              // copy bearing
              LKAirspaces[i].Bearing = (*it)->LastCalculatedBearing();
              // copy Enabled()
              LKAirspaces[i].Enabled = (*it)->Enabled();
              // copy Selected()
              LKAirspaces[i].Selected = (*it)->Selected();
              // copy Flyzone()
              LKAirspaces[i].Flyzone = (*it)->Flyzone();
              // copy WarningLevel()
              LKAirspaces[i].WarningLevel = (*it)->WarningLevel();
              // copy WarningAckLevel()
              LKAirspaces[i].WarningAckLevel = (*it)->WarningAckLevel();
              
              // copy pointer
              LKAirspaces[i].Pointer = (*it);
              
              // Record is valid now.
              LKAirspaces[i].Valid = true;
              
              i++;
              if (i>=MAXNEARAIRSPACES) break;
          }
          LKNumAirspaces=i;
          ret = true;       // ok to use new values.
          step = 0;
          break;
   } //sw step
   
   #ifdef DEBUG_LKT
   StartupStore(_T("... DoAirspaces finished, LKNumAirspaces=%d :\n"), LKNumAirspaces);
   // For sorting debug only
   /*for (i=0; i<MAXNEARAIRSPACES; i++) {
	if (LKSortedAirspaces[i]>=0)
		StartupStore(_T("... LKSortedAirspaces[%d]=LKAirspaces[%d] Name=<%s> Type=<%s> Dist=%2.0f beardiff=%2.0f enabled=%s\n"), i, 
			LKSortedAirspaces[i],
			LKAirspaces[LKSortedAirspaces[i]].Name,
			LKAirspaces[LKSortedAirspaces[i]].Type,
            LKAirspaces[LKSortedAirspaces[i]].Distance,
            LKAirspaces[LKSortedAirspaces[i]].Bearing_difference,
            LKAirspaces[LKSortedAirspaces[i]].Enabled ? _TEXT("true"):_TEXT("false"));
   }*/
   #endif

   return ret;
}

