/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: DoCommon.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "DoInits.h"


extern void InsertCommonList(int newwp);
#ifndef GTL2
extern void DoAlternates(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int AltWaypoint);
#endif



bool DoCommonList(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

   int i;


   CommonNumber=0;
   for (i=0; i<MAXCOMMON; i++) {
	CommonIndex[i]= -1;
   }

   if (!WayPointList) {
        return false;
   }

   LockTaskData(); 

   // first we look for home and place it in position 0
   if (ValidWayPoint(HomeWaypoint)) {
	CommonIndex[CommonNumber++]=HomeWaypoint;
   }

   // Then we update the alternates in the common list
   InsertCommonList(BestAlternate);
   InsertCommonList(Alternate1);
   InsertCommonList(Alternate2);

   // Then we insert active destination and task points
   for (i=0; i< MAXTASKPOINTS; i++) {
	if (ValidTaskPoint(i))
		InsertCommonList(Task[i].Index);
   }

   extern int CurrentMarker;

   // We insert Markers in common list in reverse order, Last in first out
   if (CurrentMarker>=RESWP_FIRST_MARKER || CurrentMarker<=RESWP_LAST_MARKER) {
	for (short j=0, i=CurrentMarker; j<NUMRESMARKERS; j++) {

		if (WayPointList[i].Latitude!=RESWP_INVALIDNUMBER)
   			InsertCommonList(i);

		if (--i<RESWP_FIRST_MARKER) i=RESWP_LAST_MARKER;
	}
   }
   UnlockTaskData();
   return true;
}

// Only insert if not already existing in the list
// and valid waypoint
void InsertCommonList(int newwp) {
    int i;

    if ((CommonNumber+1) >=MAXCOMMON) return;
    if ( !ValidWayPoint(newwp) ) return;

    for (i=0; i<CommonNumber; i++) {
	if ( CommonIndex[i] == newwp) return;
    }
    CommonIndex[CommonNumber++]=newwp;
}


// Common informations: home, best, alt1, alt2 and recent waypoints
void DoCommon(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{

   int i;
   static double LastRunTime=0;

   // Safe initialisation, passthrough mode
   if (DoInit[MDI_DOCOMMON]) {
        for (i=0; i<MAXCOMMON; i++) CommonIndex[i]=-1;
	CommonNumber=0;
	DoCommonList(Basic,Calculated);
        DoInit[MDI_DOCOMMON]=false;
   }

   if (!WayPointList) return;

   // Consider replay mode...
   if (  LastRunTime > Basic->Time ) LastRunTime=Basic->Time;
   if (  (Basic->Time < (LastRunTime+NEARESTUPDATETIME)) && !LKForceDoCommon) {
        return;
   }
   if (  LastDoCommon > Basic->Time ) LastDoCommon=Basic->Time;
   if ( Basic->Time < (LastDoCommon+PAGINGTIMEOUT)) {
	return;
   }
   LastDoCommon = 0;
   LKForceDoCommon=false;

   DoCommonList(Basic,Calculated);
   if (CommonNumber==0) return;

   for (i=0; i<CommonNumber; i++) {
	DoAlternates(Basic, Calculated, CommonIndex[i]);
   }

   LastRunTime=Basic->Time;
   CommonDataReady=true;
} 

