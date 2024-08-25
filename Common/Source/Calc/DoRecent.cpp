/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: DoRecent.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"

namespace {

   /**
    * @brief Get the Recent Index object
    * 
    * @param index : turnpoint index in WaypointList
    * @return : index of tp in recent list or first available index
    */
   size_t GetRecentIndex(int index) {
      // check if newwp is already in list.
      const auto begin = std::begin(RecentIndex);
      const auto end = std::next(RecentIndex, RecentNumber);
      return std::distance(begin, std::find(begin, end, index));
   }

   unsigned GetWpChecksum(unsigned int index) { //@ 101018
      if (index < NUMRESWP || index > WayPointList.size()) {
         // it is ok to insert a reserved wp in the history, but not to save it. 
         // So we get this error, which is not an error for reswp..
         if (index>=NUMRESWP) {
	         StartupStore(_T("...... Impossible waypoint number=%d for Checksum"), index);
         }
         return 0;
      }
      int clon = std::abs(WayPointList[index].Longitude);
      int clat = std::abs(WayPointList[index].Latitude); 
      int csum = WayPointList[index].Name[0] + ((int)WayPointList[index].Altitude*100) + (clat*1000*clon);
      return csum;
   }

} // namespace


// Load and save the recent list
void LoadRecentList() {

   TCHAR filepath[MAX_PATH];
   LocalPath(filepath, _T(LKD_CONF), _T(LKF_RECENTS));

   ResetRecentList();

   if (WayPointList.empty()) {
      StartupStore(_T("... Load RecentList: No Waypoint List available"));
      return;
   }

   FILE *fp;
   if ( (fp=_tfopen(filepath, _T("r"))) == NULL ) {
      StartupStore(_T("... Cannot load recent wp history"));
      return;
   }

   char st[80];
   int load_count = 0;
   while ( (fgets(st, 80, fp))!=NULL) {

      if (st[0]=='#') {
         continue; // skip comments
      }
      int nwp=atoi(st);
      if (!ValidWayPoint(nwp)) {
         StartupStore(_T("---- Loading history. Found an invalid wp: <%d>"), nwp); // BUGFIX 091122
         StartupStore(_T("---- History file could be corrupted or wp file changed abruptly. Discarded!"));
         break;
      }
      if (load_count >= MAXCOMMON) {
         StartupStore(_T("---- Too many history waypoints to load!"));
         break;
      }
      if ( (fgets(st,80,fp))==NULL) {
         StartupStore(_T("---- Incomplete or broken history file"));
         break;
      }
      unsigned int csum=atoi(st);
      // pre-v5: takeoff recent is different from others and can be forced on
      // v5: we dont save reswp to history, so we dont load them too
      if ( GetWpChecksum(nwp) != csum ) {
         StartupStore(_T("---- Loading history. Found an invalid checksum, aborting"));
         break;
      }
      RecentIndex[load_count] = nwp;
      RecentChecksum[load_count] = csum;
      ++load_count;
   }
   fclose(fp);
   RecentNumber = load_count;
   StartupStore(_T(". LoadRecentList: loaded %d recent waypoints"), load_count);
}

void SaveRecentList() {

   TCHAR filepath[MAX_PATH];
   LocalPath(filepath, _T(LKD_CONF), _T(LKF_RECENTS));

   StartupStore(_T(". Save history to <%s>"), filepath);  // 091122
   if (WayPointList.empty()) {
      StartupStore(_T(". No Waypoint List is available, cannot save history"));
      return;
   }
   if (RecentNumber <= 0 || RecentNumber >= MAXCOMMON) {
      StartupStore(_T("---- Save history: RecentNumber=%d exceeds limit, something is wrong. Aborting"), RecentNumber);
      return;
   }


   FILE *fp;
   if ( (fp=_tfopen(filepath, _T("w"))) == NULL ) {
      StartupStore(_T("---- SaveRecentList: OPEN FILE FAILED. Cannot save recent wp history"));
      return;
   }

   fprintf(fp,"### LK8000 History of Goto Waypoints - DO NOT MODIFY THIS FILE! ###\r\n");
   fprintf(fp,"### WPRECENT FORMAT 01T \r\n");

   int save_count = 0;
   for (int i=0; i<RecentNumber; i++)  {
      if ( RecentIndex[i] <= RESWP_END ) {
         TestLog(_T(".... SaveHistory: ignoring reserved waypoint %d"),RecentIndex[i]);
         continue;
      }
      if ( !ValidWayPoint(RecentIndex[i])) {
         StartupStore(_T("---- SaveHistory: ignoring invalid waypoint %d"),RecentIndex[i]);
         continue;
      }
      if ( WayPointList[RecentIndex[i]].FileNum == -1) {
         // this waypoint does not come from files...
         continue; // 100219
      }
      if ( GetWpChecksum(RecentIndex[i]) != RecentChecksum[i] ) {
         StartupStore(_T("---- SaveHistory: invalid checksum for wp=%d checksum=%d oldchecksum=%d, maybe file has changed. Aborting."),
                     i, GetWpChecksum(RecentIndex[i]), RecentChecksum[i]);
         continue;
      }
      fprintf(fp,"%d\n%d\n",RecentIndex[i], RecentChecksum[i]);
      ++save_count;
   }
   fclose(fp);

   StartupStore(_T(". SaveRecentList: saved %d recent waypoints"), save_count);
}

// Only insert if not already existing in the list
// Insert a wp into the list: always put it on top. If already existing in the list, 
// remove old existing one from the old position.
//
// Notice v5: if a recent wp is reserved, it is inserted in the list with a checksum 0, and in any case
//   it will not be saved (nor loaded) from history.txt file
// SIDENOTE: if we are doing a go-to to a reswp, it is saved as a task on exit. This is a real waypoint,
//   even with the same name of -for example- FREEFLIGHT. No problems..
void InsertRecentList(int wp_idx) {

   // check if newwp is already in list.
   int idx = GetRecentIndex(wp_idx);

   if(idx >= RecentNumber) {
      // not found in list
      if(RecentNumber < MAXCOMMON) {
        ++RecentNumber; // list not full
      } else {
        --idx; // list full : remove last.
      }
   }

    // make room in front of list to store index
   std::copy_backward(std::begin(RecentIndex), std::next(RecentIndex, idx), std::next(RecentIndex, RecentNumber));
   std::copy_backward(std::begin(RecentChecksum), std::next(RecentChecksum, idx), std::next(RecentChecksum, RecentNumber));

   RecentIndex[0] = wp_idx;
   RecentChecksum[0] = GetWpChecksum(wp_idx);
}

void RemoveRecentList(int wp_idx) {

   int idx = GetRecentIndex(wp_idx);

   std::copy(std::next(RecentIndex, idx + 1), std::next(RecentIndex, RecentNumber), std::next(RecentIndex, idx));
   std::copy(std::next(RecentChecksum, idx + 1), std::next(RecentChecksum, RecentNumber), std::next(RecentChecksum, idx));

   --RecentNumber;
}

void ResetRecentList() {
   RecentNumber = 0;
   std::fill(std::begin(RecentIndex), std::end(RecentIndex), -1);
   std::fill(std::begin(RecentChecksum), std::end(RecentChecksum), 0);
}


// Recent informations: called from Calculation task
void DoRecent(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
   static double LastRunTime=0;

   if (WayPointList.empty()) {
      return;
   }

   // Consider replay mode...
   if (  LastRunTime > Basic->Time ) {
      LastRunTime=Basic->Time;
   }
   if (  (Basic->Time < (LastRunTime+NEARESTUPDATETIME)) && !LKForceDoRecent) {
        return;
   }
   LKForceDoRecent=false;

   if (RecentNumber==0) {
      return;
   }

   // reverse iterator is required to allow to remove item from list inside loop.
   std::reverse_iterator<int*> end(std::rend(RecentIndex));
   std::reverse_iterator<int*> begin(std::next(RecentIndex, RecentNumber));

   std::for_each(begin, end, [&](int idx) {
      if (!ValidWayPoint(idx)) {
         RemoveRecentList(idx);
      } else {
         DoAlternates(Basic, Calculated, idx);
      }
   });

   LastRunTime=Basic->Time;
   RecentDataReady=true;
}
