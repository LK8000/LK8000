/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKInterface.h"
#include "DoInits.h"
#include "Sound/Sound.h"

// return current overtarget waypoint index, or -1 if not available
int GetOvertargetIndex(void) {
  int index;
  switch (OvertargetMode) {
	case OVT_TASK: // task
		if ( ValidTaskPoint(ActiveTaskPoint) != false ) {
			if (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute())
				index=RESWP_OPTIMIZED;
			else {
				index = Task[ActiveTaskPoint].Index;
			}
			if ( index >=0 ) return index;
		}
		return -1;
		break;
	case OVT_TASKCENTER: // task Center
		if ( ValidTaskPoint(ActiveTaskPoint) != false ) {
            return Task[ActiveTaskPoint].Index;
        }
		return -1;
		break;
	case OVT_ALT1: // alternate 1
		if ( ValidWayPoint(Alternate1) != false ) {
			index = Alternate1;
			if ( index >=0 ) return index;
		}
		return -1;
		break;
	case OVT_ALT2: // alternate 2
		if ( ValidWayPoint(Alternate2) != false ) {
			index = Alternate2;
			if ( index >=0 ) return index;
		}
		return -1;
		break;
	case OVT_BALT: // bestalternate
		if ( ValidWayPoint(BestAlternate) != false ) {
			index = BestAlternate;
			if ( index >=0 ) return index;
		}
		return -1;
		break;
	case OVT_HOME: // home waypoint
		if (ValidWayPoint(HomeWaypoint)) {
			index = HomeWaypoint;
			if ( index >=0 ) return index;
		}
		return -1;
		break;

	case OVT_THER:
		index=RESWP_LASTTHERMAL;
		if (ValidResWayPoint(index)) return index;
		return -1;
		break;

	case OVT_MATE:
		index=RESWP_TEAMMATE;
		if (ValidResWayPoint(index)) return index;
		return -1;
		break;

	case OVT_XC:
		index=RESWP_FAIOPTIMIZED;
	    if (ValidResWayPoint(index)) return index;
	    return -1;
	    break;

	case OVT_FLARM:
		index=RESWP_FLARMTARGET;
		if (ValidResWayPoint(index)) return index;
		return -1;
		break;


	// 4: home, 5: traffic, 6: mountain pass, last thermal, etc.
	default:
		return -1;
		break;
  }
}

// return current overtarget waypoint name with leading identifier, even if empty
// exception for TEAM MATE: always report OWN CODE if available
void GetOvertargetName(TCHAR *overtargetname) {
  int index;
  if (OvertargetMode == OVT_MATE) {
	if (ValidWayPoint(TeamCodeRefWaypoint)) {
		if (TeammateCodeValid)
			_stprintf(overtargetname,_T("%s> %s"), GetOvertargetHeader(),CALCULATED_INFO.OwnTeamCode);
		else
			_stprintf(overtargetname,_T("%s: %s"), GetOvertargetHeader(),CALCULATED_INFO.OwnTeamCode);
	} else
		_stprintf(overtargetname,_T("%s ---"),GetOvertargetHeader());
	return;
  }
  LockTaskData();
  index=GetOvertargetIndex();
  if (index<0)
	_stprintf(overtargetname,_T("%s ---"),GetOvertargetHeader());
  else
	_stprintf(overtargetname,_T("%s%s"), GetOvertargetHeader(),WayPointList[index].Name);
  UnlockTaskData();
}

#define OVERTARGETHEADER_MAX 3
// return current overtarget header name
TCHAR *GetOvertargetHeader(void) {

  // Maxmode + 1 because maxmode does not account pos 0
  static TCHAR targetheader[OVT_MAXMODE+1][OVERTARGETHEADER_MAX+2];

  if (DoInit[MDI_GETOVERTARGETHEADER]) {
	// LKTOKEN _@M1323_ "T>"
	LK_tcsncpy(targetheader[OVT_TASK], MsgToken(1323), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1323_ "T>"
	LK_tcsncpy(targetheader[OVT_TASKCENTER], MsgToken(1323), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1324_ "B>"
	LK_tcsncpy(targetheader[OVT_BALT], MsgToken(1324), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1325_ "1>"
	LK_tcsncpy(targetheader[OVT_ALT1], MsgToken(1325), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1326_ "2>"
	LK_tcsncpy(targetheader[OVT_ALT2], MsgToken(1326), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1327_ "H>"
	LK_tcsncpy(targetheader[OVT_HOME], MsgToken(1327), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1328_ "L>"
	LK_tcsncpy(targetheader[OVT_THER], MsgToken(1328), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1329_ "M"
	LK_tcsncpy(targetheader[OVT_MATE], MsgToken(1329), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1330_ "F>"
	LK_tcsncpy(targetheader[OVT_FLARM], MsgToken(1330), OVERTARGETHEADER_MAX);

	LK_tcsncpy(targetheader[OVT_XC], TEXT("X>"), OVERTARGETHEADER_MAX); // No need for translation here. X international code for Cross Country !

	for (int i=0; i<OVT_MAXMODE+1; i++) targetheader[i][OVERTARGETHEADER_MAX]='\0';
	DoInit[MDI_GETOVERTARGETHEADER]=false;
  }

  return(targetheader[OvertargetMode]);
}

void RotateOvertarget(void) {

_tryagain:
  OvertargetMode++;

  // OVT_TASKCENTER multitarget only exist if PG optimized task is defined
  // Skip it in all other case
  if (OvertargetMode == OVT_TASKCENTER && (!ACTIVE_WP_IS_AAT_AREA && !DoOptimizeRoute())) {
    goto _tryagain;
  }

  // For PG/HG, skip BALT overtarget if nothing valid.
  // We assume that this means no landables ever seen around, because
  // the BA function would keep the old one even if invalid.
  if (ISPARAGLIDER && OvertargetMode==OVT_BALT) {
	if (!ValidWayPoint(BestAlternate)) goto _tryagain;
  }

  // Skip Alternate 1 if Not defined
  if(OvertargetMode==OVT_ALT1 && (!ValidWayPoint(Alternate1))) {
	goto _tryagain;
  }

  // Skip Alternate 2 if Not defined
  if(OvertargetMode==OVT_ALT2 && (!ValidWayPoint(Alternate2))) {
	goto _tryagain;
  }

  // Skip Last Thermal if no thermal is detected ...
  if(OvertargetMode==OVT_THER && (!ValidResWayPoint(RESWP_LASTTHERMAL))) {
	goto _tryagain;
  }

  	// Skip Cross Country triangle closing if no Addition Contest active or not valid optimized point ...
  if( OvertargetMode==OVT_XC &&
      ( AdditionalContestRule==static_cast<int>(CContestMgr::ContestRule::NONE)  || !ValidResWayPoint(RESWP_FAIOPTIMIZED))) {
		goto _tryagain;
  }

  // Skip F rotation if no flarm or no valid flarm target
  if (OvertargetMode==OVT_FLARM) {
	if ( (!GPS_INFO.FLARM_Available)|| (!ValidResWayPoint(RESWP_FLARMTARGET)) )
		goto _tryagain;
  }

  if (OvertargetMode==OVT_MATE) {
	if (!ValidResWayPoint(RESWP_TEAMMATE))
		goto _tryagain;
  }

  if (OvertargetMode>OVT_ROTATE) {
	OvertargetMode=OVT_TASK;
  }
	switch(OvertargetMode) {
		case 0:
			PlayResource(TEXT("IDR_WAV_OVERTONE7"));
			break;
		case 1:
			PlayResource(TEXT("IDR_WAV_OVERTONE0"));
			break;
		case 2:
			PlayResource(TEXT("IDR_WAV_OVERTONE1"));
			break;
		case 3:
			PlayResource(TEXT("IDR_WAV_OVERTONE2"));
			break;
		case 4:
			PlayResource(TEXT("IDR_WAV_OVERTONE3"));
			break;
		case 5:
			PlayResource(TEXT("IDR_WAV_OVERTONE4"));
			break;
		case 6:
			PlayResource(TEXT("IDR_WAV_OVERTONE5"));
			break;
		case 7:
			PlayResource(TEXT("IDR_WAV_OVERTONE6"));
			break;
		case 8:
			PlayResource(TEXT("IDR_WAV_OVERTONE7"));
			break;
		default:
			PlayResource(TEXT("IDR_WAV_OVERTONE5"));
			break;
	}
}
