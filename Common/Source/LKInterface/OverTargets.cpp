/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKInterface.h"
#include "DoInits.h"

// return current overtarget waypoint index, or -1 if not available
int GetOvertargetIndex(void) {
  int index;
  switch (OvertargetMode) {
	case OVT_TASK: // task 
		if ( ValidTaskPoint(ActiveWayPoint) != false ) {
			if (DoOptimizeRoute())
				index=RESWP_OPTIMIZED;
			else {
				index = Task[ActiveWayPoint].Index;
			}
			if ( index >=0 ) return index;
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
  index=GetOvertargetIndex();
  if (index<0)
	_stprintf(overtargetname,_T("%s ---"),GetOvertargetHeader());
  else 
	_stprintf(overtargetname,_T("%s%s"), GetOvertargetHeader(),WayPointList[index].Name);
}

#define OVERTARGETHEADER_MAX 3
// return current overtarget header name
TCHAR *GetOvertargetHeader(void) {
  static bool doinit=true;
  Assign_DoInits(&doinit,MDI_GETOVERTARGETHEADER);

  // Maxmode + 1 because maxmode does not account pos 0
  static TCHAR targetheader[OVT_MAXMODE+1][OVERTARGETHEADER_MAX+1];

  if (doinit) {
	// LKTOKEN _@M1323_ "T>"
	_tcsncpy(targetheader[OVT_TASK], gettext(TEXT("_@M1323_")), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1324_ "B>"
	_tcsncpy(targetheader[OVT_BALT], gettext(TEXT("_@M1324_")), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1325_ "1>"
	_tcsncpy(targetheader[OVT_ALT1], gettext(TEXT("_@M1325_")), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1326_ "2>"
	_tcsncpy(targetheader[OVT_ALT2], gettext(TEXT("_@M1326_")), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1327_ "H>"
	_tcsncpy(targetheader[OVT_HOME], gettext(TEXT("_@M1327_")), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1328_ "L>"
	_tcsncpy(targetheader[OVT_THER], gettext(TEXT("_@M1328_")), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1329_ "M"
	_tcsncpy(targetheader[OVT_MATE], gettext(TEXT("_@M1329_")), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1330_ "F>"
	_tcsncpy(targetheader[OVT_FLARM], gettext(TEXT("_@M1330_")), OVERTARGETHEADER_MAX);

	for (int i=0; i<OVT_MAXMODE+1; i++) targetheader[i][OVERTARGETHEADER_MAX]='\0';
	doinit=false;
  }

  return(targetheader[OvertargetMode]);
}

void RotateOvertarget(void) {

  OvertargetMode++;

  // For PG/HG, skip BALT overtarget if nothing valid. 
  // We assume that this means no landables ever seen around, because
  // the BA function would keep the old one even if invalid.
  if (ISPARAGLIDER && OvertargetMode==OVT_BALT) {
	if (!ValidWayPoint(BestAlternate)) OvertargetMode++;
  }

  if (OvertargetMode==OVT_FLARM) {
	if (!GPS_INFO.FLARM_Available) OvertargetMode++;
  }

  if (OvertargetMode>OVT_ROTATE) {
	OvertargetMode=OVT_TASK;
  }
  #ifndef DISABLEAUDIO
  if (EnableSoundModes) {
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
  #endif
  return;

}

