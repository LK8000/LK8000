/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"
#include "RGB.h"
#include "Sound/Sound.h"
//
// Called by DrawThread
//
void MapWindow::DrawLKAlarms(LKSurface& Surface, const RECT& rc) {

  static unsigned short displaycounter=0;
  static short oldvalidalarm=-1;
  short validalarm=-1;

  // Alarms are working only with a valid GPS fix. No navigator, no alarms.
  if (DrawInfo.NAVWarning) return;

  // give priority to the lowest alarm in list
  if (CheckAlarms(2)) validalarm=2;
  if (CheckAlarms(1)) validalarm=1;
  if (CheckAlarms(0)) validalarm=0;

  // If we have a new alarm, play sound if available and enabled
  if (validalarm>=0) {
    switch (validalarm) {
        case 0:
            LKSound(_T("LK_ALARM_ALT1.WAV"));
            break;
        case 1:
            LKSound(_T("LK_ALARM_ALT2.WAV"));
            break;
        case 2:
            LKSound(_T("LK_ALARM_ALT3.WAV"));
            break;
        default:
            break;
    }
	displaycounter=12; // seconds to display alarm on screen, resetting anything set previously
  }

  // Now paint message, even for passed alarms
  if (displaycounter) {

	if (--displaycounter>60) displaycounter=0; // safe check 

	TCHAR textalarm[100];
	short currentalarm=0;
	if (validalarm>=0) {
		currentalarm=validalarm;
		oldvalidalarm=validalarm;
	} else
		if (oldvalidalarm>=0) currentalarm=oldvalidalarm; // safety check

	switch(currentalarm) {
		case 0:
		case 1:
		case 2:
			_stprintf(textalarm,_T("%s %d: %s %d"),
			MsgToken(1650), currentalarm+1, MsgToken(1651),  // ALARM ALTITUDE
			((int)((double)LKalarms[currentalarm].triggervalue*ALTITUDEMODIFY)));
			break;
		default:
			break;
	}

	const auto oldfont = Surface.SelectObject(ScreenLandscape
                                                  ? LK8TargetFont
                                                  : LK8MediumFont);

	TextInBoxMode_t TextInBoxMode = {0};
	TextInBoxMode.Color = RGB_WHITE;
	TextInBoxMode.NoSetFont=1;
	TextInBoxMode.AlligneCenter = 1;
	TextInBoxMode.WhiteBorder = 1;
	TextInBoxMode.Border = 1;

	// same position for gps warnings: if navwarning, then no alarms. So no overlapping.
        TextInBox(Surface, &rc, textalarm , (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, &TextInBoxMode);

	Surface.SelectObject(oldfont);
  }

}


