/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: DoRecent.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"

#ifndef GTL2
extern void DoAlternates(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int AltWaypoint);
#endif


// Load and save the recent list
bool LoadRecentList() {

   TCHAR buffer[100];
   FILE *fp;
   char st[100];
   int i=0, nwp;
   unsigned int csum;

  LocalPath(buffer,TEXT(LKD_CONF));
  _tcscat(buffer,TEXT("\\"));
  _tcscat(buffer,_T(LKF_RECENTS)); // 091101


   RecentNumber=0;
   for (i=0; i<MAXCOMMON; i++) {
	RecentIndex[i]= -1;
	RecentChecksum[i]=0;
   }

   if (!WayPointList) {
   	StartupStore(_T("... Load RecentList: No Waypoint List available%s"),NEWLINE);
        return false;
   }

   if ( (fp=_tfopen(buffer, _T("r"))) == NULL ) {
	StartupStore(_T("... Cannot load recent wp history%s"),NEWLINE);
	return false;
   }

   i=0;
   while ( (fgets(st, 80, fp))!=NULL) {
	if (st[0]=='#') continue; // skip comments
	nwp=atoi(st);
	if (!ValidWayPoint(nwp)) {
		wsprintf(buffer,_T("---- Loading history. Found an invalid wp: <%d>%s"),nwp,NEWLINE); // BUGFIX 091122
		StartupStore(buffer);
		wsprintf(buffer,_T("---- History file could be corrupted or wp file changed abruptly. Discarded!%s"),NEWLINE);
		StartupStore(buffer);
		break;
	}
	if (i >=(MAXCOMMON)) {
		StartupStore(_T("---- Too many history waypoints to load!%s"),NEWLINE);
		break;
	}
	if ( (fgets(st,80,fp))==NULL) {
		StartupStore(_T("---- Incomplete or broken history file%s"),NEWLINE);
		break;
	}
	csum=atoi(st);
	// takeoff recent is different from others and can be forced on
	if (nwp!=RESWP_TAKEOFF) {
		if ( GetWpChecksum(nwp) != csum ) {
			StartupStore(_T("---- Loading history. Found an invalid checksum, aborting.%s"),NEWLINE);
			break;
		}
	}
	RecentIndex[i]=nwp;
	RecentChecksum[i++]=csum;
   }
   fclose(fp);
   RecentNumber=i;
   wsprintf(buffer,_T(". LoadRecentList: loaded %d recent waypoints%s"),i,NEWLINE);
   StartupStore(buffer);

   return true;
}

bool SaveRecentList() {

   TCHAR buffer[100];
   FILE *fp;
   int i;

  LocalPath(buffer,TEXT(LKD_CONF));
  _tcscat(buffer,TEXT("\\"));
  _tcscat(buffer,_T(LKF_RECENTS)); // 091101

   StartupStore(_T(". Save history to <%s>%s"),buffer,NEWLINE);  // 091122
   if (!WayPointList) {
   	StartupStore(_T(". No Waypoint List is available, cannot save history%s"),NEWLINE);
        return false;
   }
   if (RecentNumber <0 || RecentNumber >(MAXCOMMON+1)) {
	StartupStore(_T("---- Save history: RecentNumber=%d exceeds limit, something is wrong. Aborting%s"),RecentNumber,NEWLINE);
	return false;
   }
   if ( (fp=_tfopen(buffer, _T("w"))) == NULL ) {
	StartupStore(_T("---- SaveRecentList: OPEN FILE FAILED. Cannot save recent wp history%s"),NEWLINE);
	return false;
   }

   fprintf(fp,"### LK8000 History of Goto Waypoints - DO NOT MODIFY THIS FILE! ###\r\n");
   fprintf(fp,"### WPRECENT FORMAT 01T \r\n");
   for (i=0; i<RecentNumber; i++)  {
	if ( !ValidNotResWayPoint(RecentIndex[i])) {
		StartupStore(_T("---- SaveHistory: invalid wp, maybe file has changed. Aborting.%s"),NEWLINE);
		break;
	}
	if ( WayPointList[RecentIndex[i]].FileNum == -1) continue; // 100219
	if ( GetWpChecksum(RecentIndex[i]) != RecentChecksum[i] ) {
		StartupStore(_T("---- SaveHistory: invalid checksum for wp=%d checksum=%d oldchecksum=%d, maybe file has changed. Aborting.%s"),
		i,GetWpChecksum(RecentIndex[i]), RecentChecksum[i],NEWLINE);
		break;
	}

	fprintf(fp,"%d\n%d\n",RecentIndex[i], RecentChecksum[i]);
   }
   fclose(fp);

   wsprintf(buffer,_T(". SaveRecentList: saved %d recent waypoints%s"),RecentNumber,NEWLINE);
   StartupStore(buffer);
   return true;
}

// Only insert if not already existing in the list
// Insert a wp into the list: always put it on top. If already existing in the list, 
// remove old existing one from the old position.
void InsertRecentList(int newwp) {
    int i,j;
    int TmpIndex[MAXCOMMON+1];
    unsigned int TmpChecksum[MAXCOMMON+1];

    // TCHAR buffer[100];
    // wsprintf(buffer,_T(". Insert WP=%d into recent waypoints%s"),newwp,NEWLINE);
    // StartupStore(buffer);

    // j holding number of valid recents, excluded the new one to be put in position 0
    for (i=0,j=0; i<RecentNumber; i++) {
	if ( j >=(MAXCOMMON-2) ) break;
	if (RecentIndex[i]==newwp) continue;
	TmpIndex[++j]=RecentIndex[i];
	TmpChecksum[j]=RecentChecksum[i];
    }
    RecentIndex[0]=newwp;
    
    RecentChecksum[0]= GetWpChecksum(newwp);
    for (i=1; i<=j; i++) {
	RecentIndex[i]=TmpIndex[i];
	RecentChecksum[i]=TmpChecksum[i];
    }
    RecentNumber=j+1;
}

void RemoveRecentList(int newwp) {
    int i,j;
    int TmpIndex[MAXCOMMON+1];
    unsigned int TmpChecksum[MAXCOMMON+1];

    for (i=0,j=0;i<RecentNumber; i++) {
	if (RecentIndex[i]==newwp) continue;
	TmpIndex[j]=RecentIndex[i];
	TmpChecksum[j++]=RecentChecksum[i];
    }
    RecentNumber=j;
    for (i=0; i<=j; i++) {
	RecentIndex[i]=TmpIndex[i];
	RecentChecksum[i]=TmpChecksum[i];
    }
}

void ResetRecentList() {
   int i;

   RecentNumber=0;
   for (i=0; i<MAXCOMMON; i++) {
	RecentIndex[i]= -1;
	RecentChecksum[i]= 0;
   }
   StartupStore(_T(". ResetRecentList%s"),NEWLINE);

}


// Recent informations: called from Calculation task
void DoRecent(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
   int i;
   static bool DoInit=true;
   static double LastRunTime=0;

   // Safe initialisation, passthrough mode
   if (DoInit) {
        //for (i=0; i<MAXCOMMON; i++) RecentIndex[i]=-1;
	//RecentNumber=0;
	//LoadRecentList();
	// load recent file
        DoInit=false;
   }

   if (!WayPointList) return;

   // Consider replay mode...
   if (  LastRunTime > Basic->Time ) LastRunTime=Basic->Time;
   if (  (Basic->Time < (LastRunTime+NEARESTUPDATETIME)) && !LKForceDoRecent) {
        return;
   }
   LKForceDoRecent=false;
reload:
   //DoRecentList(Basic,Calculated);  NO NEED TO RELOAD...
   if (RecentNumber==0) return;

   for (i=0; i<RecentNumber; i++) {
	if (!ValidWayPoint(RecentIndex[i])) {
		RemoveRecentList(RecentIndex[i]);
		goto reload;
	}
	else
		DoAlternates(Basic, Calculated, RecentIndex[i]);
   }

   LastRunTime=Basic->Time;
   RecentDataReady=true;

}


