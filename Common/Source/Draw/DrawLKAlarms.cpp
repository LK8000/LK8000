/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"
#include "RGB.h"
#include "Sound/Sound.h"
#include "Message.h"
//
// Called by DrawThread
//
void MapWindow::DrawLKAlarms() {

	const TCHAR*  AlarmsSounds[] = {
		_T("LK_ALARM_ALT1.WAV"),
		_T("LK_ALARM_ALT2.WAV"),
		_T("LK_ALARM_ALT3.WAV")
	};

  // Alarms are working only with a valid GPS fix. No navigator, no alarms.
  if (DrawInfo.NAVWarning) return;

	unsigned active_alarm = 0U; // no new alarms 
	
	// check all alarms in reverse order, give priority to the lowest alarm in list
	for( unsigned i = MAXLKALARMS; i > 0; --i) {
		if(CheckAlarms(i-1)) {
			active_alarm = i;
		}
	}
	
	// new alarms ? 
	if(active_alarm > 0) {
		// ack previous alarms message
		Message::Acknowledge(MSG_ALARM);

		// Display new alarms message
		TCHAR textalarm[100];
		_stprintf(textalarm,_T("%s %d: %s %d"), MsgToken(1650), active_alarm, MsgToken(1651),  // ALARM ALTITUDE
										((int)((double)LKalarms[active_alarm-1].triggervalue*ALTITUDEMODIFY)));

		Message::AddMessage(12000, MSG_ALARM, textalarm); // Message for 12 sec.

		LKSound(AlarmsSounds[active_alarm-1]);
	}
}
