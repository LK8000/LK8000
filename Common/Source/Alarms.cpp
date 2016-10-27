/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

// #define DEBUG_LKALARMS	1

void InitAlarms(void) {

  #if DEBUG_LKALARMS
  StartupStore(_T("...... Alarms: InitAlarms\n"));
  #endif
  int i;
  for (i=0; i<MAXLKALARMS; i++) {
	LKalarms[i].triggervalue=0;
	LKalarms[i].lastvalue=0;
	LKalarms[i].lasttriggertime=0.0;
	LKalarms[i].triggerscount=0;
  }
	/* Test values
	LKalarms[0].triggervalue=500;
	LKalarms[1].triggervalue=0;
	LKalarms[2].triggervalue=1200;
	*/
}

#if DEBUG_LKALARMS
#undef LKALARMSINTERVAL
#undef MAXLKALARMSTRIGGERS
#define LKALARMSINTERVAL 10
#define MAXLKALARMSTRIGGERS 3
#endif

// alarms in range 0-(MAXLKALARMS-1), that is  0-2
bool CheckAlarms(unsigned short al) {

  int i;

  // safe check
  if (al>=MAXLKALARMS) return false;

  // Alarms are working only with a valid GPS fix. No navigator, no alarms.
  if (GPS_INFO.NAVWarning) return false;

  // do we have a valid alarm request?
  if ( LKalarms[al].triggervalue == 0) return false;

  // We check for duplicates. We could do it only when config is changing, right now.
  // However, maybe we can have LK set automatically alarms in the future.
  // Duplicates filter is working giving priority to the lowest element in the list
  // We don't want more than 1 alarm for the same trigger value
  for (i=0; i<=al && i<MAXLKALARMS ; i++) {
	if (i==al) continue; // do not check against ourselves
	// if a previous alarm has the same value, we are a duplicate
	if (LKalarms[al].triggervalue == LKalarms[i].triggervalue) {
		#if DEBUG_LKALARMS
		StartupStore(_T("...... Alarms: duplicate value [%d]=[%d] =<%d>\n"), al, i, LKalarms[i].triggervalue);
		#endif
		return false;
	}
  }

  // ok so this is not a duplicated alarm, lets check if we have overcounted
  if (LKalarms[al].triggerscount >= MAXLKALARMSTRIGGERS) {
	#if DEBUG_LKALARMS
	StartupStore(_T("...... Alarms: count exceeded for [%d]\n"),al);
	#endif
	return false;
  }

  // if too early we ignore it in any case
  if (GPS_INFO.Time < (LKalarms[al].lasttriggertime + LKALARMSINTERVAL)) {
	#if DEBUG_LKALARMS
	StartupStore(_T("...... Alarms: too early for [%d], still %.0f seconds to go\n"),al,
	(LKalarms[al].lasttriggertime + LKALARMSINTERVAL)- GPS_INFO.Time);
	#endif
	return false;
  }

  // So this is a potentially valid alarm to check

  //
  // First we check for altitude alarms , 0-2
  //
  if (al<3) {

	int navaltitude=(int)CALCULATED_INFO.NavAltitude;

	// is this is the first valid sample?
	if (LKalarms[al].lastvalue==0) {
		LKalarms[al].lastvalue= navaltitude;
		#if DEBUG_LKALARMS
		StartupStore(_T("...... Alarms: init lastvalue [%d] = %d\n"),al,LKalarms[al].lastvalue);
		#endif
		return false;
	}

	// if we were previously below trigger altitude
	if (LKalarms[al].lastvalue< LKalarms[al].triggervalue) {
		#if DEBUG_LKALARMS
		StartupStore(_T("...... Alarms: armed lastvalue [%d] = %d < trigger <%d>\n"),al,
		LKalarms[al].lastvalue,LKalarms[al].triggervalue);
		#endif
		// if we are now over the trigger altitude
		if (navaltitude >= LKalarms[al].triggervalue) {
			#if DEBUG_LKALARMS
			StartupStore(_T("...... Alarms: RING [%d] = %d\n"),al,navaltitude);
			#endif
			// bingo. first reset last value , update lasttime and counter
			LKalarms[al].lastvalue=0;
			LKalarms[al].triggerscount++;
			LKalarms[al].lasttriggertime = GPS_INFO.Time;
			return true;
		}
	}

	// otherwise simply update lastvalue
	LKalarms[al].lastvalue=navaltitude;
	return false;

  } // end altitude alarms


  // other alarms here, or failed
  return false;

}
