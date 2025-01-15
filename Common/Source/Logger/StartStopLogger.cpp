/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"
#include "Dialogs.h"

void guiStartLogger(bool noAsk) {
	if (LoggerActive) {
		return;
	}

	if (ReplayLogger::IsEnabled()) {
		guiStopLogger(true);
		return;
	}

	if (!noAsk) {
		TCHAR TaskMessage[1024];
		lk::strcpy(TaskMessage,MsgToken<876>()); // Start Logger With Declaration\r\n;
		_tcscat(TaskMessage,_T("\r\n"));

		if(ValidTaskPoint(0)) {
			_tcscat(TaskMessage, WayPointList[Task[0].Index].Name);
		}

		if(ValidTaskPoint(1)) {
			_tcscat(TaskMessage,TEXT(" - "));
			_tcscat(TaskMessage, WayPointList[Task[0].Index].Name);
			_tcscat(TaskMessage,TEXT("\r\n ..."));
		}

		// LKTOKEN  _@M637_ = "Start Logger"
		if (MessageBoxX(TaskMessage,MsgToken<637>(), mbYesNo) != IdYes) {
			return;
		}
	}

	if (LoggerClearFreeSpace()) {
		StartLogger();
	} else {
		// LKTOKEN  _@M408_ = "Logger inactive, insufficient storage!"
		// LKTOKEN  _@M404_ = "Logger Error"
		MessageBoxX(MsgToken<408>(), MsgToken<404>(), mbOk);
		StartupStore(_T("------ Logger not started: Insufficient Storage"));
	}

	FullScreen();
}


void guiStopLogger(bool noAsk) {
	if (!LoggerActive) {
		return;
	}

	// LKTOKEN  _@M669_ = "Stop Logger"
	if(noAsk || (MessageBoxX(MsgToken<669>(), MsgToken<669>(), mbYesNo) == IdYes)) {
		StopLogger();
		if (!noAsk) {
			// force landing for paragliders..
			if ((ISPARAGLIDER) && CALCULATED_INFO.Flying && ((GPS_INFO.Speed <= TakeOffSpeedThreshold) || GPS_INFO.NAVWarning) ) {
				// force landing event from TakeoffLanding
				LKSW_ForceLanding=true;
				StartupStore(_T(". Logger stopped manually, landing is forced%s"),NEWLINE);
			}
			FullScreen();
		}
	}
}


void guiToggleLogger(bool noAsk) {
	if (LoggerActive) {
		guiStopLogger(noAsk);
	} else {
		guiStartLogger(noAsk);
	}
}
