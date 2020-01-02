/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Message.h"
#include "Sound/Sound.h"

/** [Alarm Max Altitude] 
 * 
 * An alarm will be triggered everytime this altitude is reached and crossed.
 * 
 * The alarm will not be repeated within the next 60 seconds.
 * Each alarm can be repeated max 30 times. After 30 times, an alarm will automatically self disable for the current flight.
 * 
 * The visual alarm message automatically disappears after 12 seconds.

 * After 1 minute, if altitude has been lower than Max Altitude and then becomes higher again, alarm will sound again.
 * 
 * To disable this alarm, set it to 0. ( Default Value )
 * The altitude used for alarm is the one chosen in Config menu 5 "Use baro altitude".
 * Multiple alarms set at the same altitude will have no effect.
 * 
 * Each alarm has a custom sound that can be changed inside _System _Sounds folder. They are named LK_ALARM_ALTx.WAV
 */

// #define DEBUG_LKALARMS	1


static_assert(std::size(LKalarms) == MAXLKALARMS, "Wrong MAXLKALARMS and LKalarms array");

void InitAlarms(void) {
  for ( auto &alarm : LKalarms) {
    alarm.triggervalue = 0;
    alarm.lastvalue = 0;
    alarm.lasttriggertime = 0.0;
    alarm.triggerscount = 0;
  }
}

#if DEBUG_LKALARMS
#undef LKALARMSINTERVAL
#undef MAXLKALARMSTRIGGERS
#define LKALARMSINTERVAL 10
#define MAXLKALARMSTRIGGERS 3
#endif

// alarms in range 0-(MAXLKALARMS-1), that is  0-2
static
bool CheckAlarms(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated, unsigned short al) {

  // safe check
  if (al>=MAXLKALARMS) return false;

  // Alarms are working only with a valid GPS fix. No navigator, no alarms.
  if (Basic.NAVWarning) return false;

  // do we have a valid alarm request?
  if ( LKalarms[al].triggervalue == 0) return false;

  // We check for duplicates. We could do it only when config is changing, right now.
  // However, maybe we can have LK set automatically alarms in the future.
  // Duplicates filter is working giving priority to the lowest element in the list
  // We don't want more than 1 alarm for the same trigger value
  
  // i < MAXLKALARMS always true : already check line 56.
  for (unsigned i = 0; i < al && i < MAXLKALARMS ; i++) {
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
  if (Basic.Time < (LKalarms[al].lasttriggertime + LKALARMSINTERVAL)) {
#if DEBUG_LKALARMS
    StartupStore(_T("...... Alarms: too early for [%d], still %.0f seconds to go\n"),
                          al, (LKalarms[al].lasttriggertime + LKALARMSINTERVAL)- Basic.Time);
#endif
    return false;
  }

  // So this is a potentially valid alarm to check

  //
  // First we check for altitude alarms , 0-2
  //
  const int navaltitude=(int)Calculated.NavAltitude;
  // is this is the first valid sample?
  if (LKalarms[al].lastvalue==0) {
    LKalarms[al].lastvalue= navaltitude;
#if DEBUG_LKALARMS
    StartupStore(_T("...... Alarms: init lastvalue [%d] = %d\n"),al,LKalarms[al].lastvalue);
#endif
    return false;
  }

  
  bool bTrigger = false;
  // if we were previously below trigger altitude
  if (LKalarms[al].lastvalue < LKalarms[al].triggervalue) {
#if DEBUG_LKALARMS
    StartupStore(_T("...... Alarms: armed lastvalue [%d] = %d < trigger <%d>\n"),
            al, LKalarms[al].lastvalue,LKalarms[al].triggervalue);
#endif
    // if we are now over the trigger altitude
    if (navaltitude >= LKalarms[al].triggervalue) {
#if DEBUG_LKALARMS
      StartupStore(_T("...... Alarms: RING [%d] = %d\n"),al,navaltitude);
#endif
      // bingo : update lasttime and counter
      LKalarms[al].triggerscount++;
      LKalarms[al].lasttriggertime = Basic.Time;
      bTrigger = true;
    }
  } // end altitude alarms

  // otherwise simply update lastvalue
  LKalarms[al].lastvalue=navaltitude;

  return bTrigger;

}

static PeriodClock LKAlarmTick;
//
// Called by CalculationThread
//
void CheckAltitudeAlarms(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated) {

  if(!LKAlarmTick.CheckUpdate(1000)) {
    // don't check Alarms faster than 1Hz
    return;
  }
  
	const TCHAR*  AlarmsSounds[] = {
		_T("LK_ALARM_ALT1.WAV"),
		_T("LK_ALARM_ALT2.WAV"),
		_T("LK_ALARM_ALT3.WAV")
	};

  // Alarms are working only with a valid GPS fix. No navigator, no alarms.
  if (Basic.NAVWarning) return;

	unsigned active_alarm = 0U; // no new alarms 
	
	// check all alarms in reverse order, give priority to the lowest alarm in list
	for( unsigned i = MAXLKALARMS; i > 0; --i) {
		if(CheckAlarms(Basic, Calculated, i-1)) {
			active_alarm = i;
		}
	}
	
	// new alarms ? 
	if(active_alarm > 0) {
		// ack previous alarms message
		Message::Acknowledge(MSG_ALARM);

		// Display new alarms message
		TCHAR textalarm[100];
		_stprintf(textalarm,_T("%s %d: %s %d"), MsgToken<1650>(), active_alarm, MsgToken<1651>(),  // ALARM ALTITUDE
										((int)((double)LKalarms[active_alarm-1].triggervalue*ALTITUDEMODIFY)));

		Message::AddMessage(12000, MSG_ALARM, textalarm); // Message for 12 sec.

		LKSound(AlarmsSounds[active_alarm-1]);
	}
}
