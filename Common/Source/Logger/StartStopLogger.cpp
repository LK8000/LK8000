/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"
#include "Dialogs.h"

void guiStartLogger(bool noAsk) {
  int i;

  if (!LoggerActive)
  {
	if (ReplayLogger::IsEnabled()) { 
		if (LoggerActive) 
			guiStopLogger(true); 
		return;
	}
	TCHAR TaskMessage[1024];
	_tcscpy(TaskMessage,MsgToken(876)); // Start Logger With Declaration\r\n")); 
	_tcscat(TaskMessage,_T("\r\n"));
	for(i=0;i<MAXTASKPOINTS;i++)
	{
		if(Task[i].Index == -1)
		{
			if(i==0) _tcscat(TaskMessage,MsgToken(479)); // None
			Debounce(); 
			break;
		}
		_tcscat(TaskMessage,WayPointList[ Task[i].Index ].Name);
		_tcscat(TaskMessage,TEXT(" - "));
		if (i==1) {
			_tcscat(TaskMessage,TEXT("\r\n ..."));
			break;
		}
	}

	// LKTOKEN  _@M637_ = "Start Logger" 
	if(noAsk || (MessageBoxX(TaskMessage,MsgToken(637), mbYesNo) == IdYes))
	{
		IGCWriteLock=true; // Lock ASAP
		if (LoggerClearFreeSpace()) {
	  
			StartLogger();
				LoggerHeader();
				// THIS IS HAPPENING TOO EARLY, and we still have concurrency with F record!
				// LoggerActive = true; // start logger after Header is completed.  Concurrency
	  
				int ntp=0;
				for(i=0;i<MAXTASKPOINTS;i++)
				{
					if(Task[i].Index == -1)
					{
						break;
					}
					ntp++;
				}
				StartDeclaration(ntp);
				for(i=0;i<MAXTASKPOINTS;i++)
				{
					if(Task[i].Index == -1) {
						Debounce(); 
						break;
					}
					AddDeclaration(WayPointList[Task[i].Index].Latitude, WayPointList[Task[i].Index].Longitude, 
					WayPointList[Task[i].Index].Name );
				}
				EndDeclaration();
				LoggerActive = true; // start logger now
				#if LOGFRECORD
				ResetFRecord(); // reset timer & lastRecord string so if logger is restarted, FRec appears at top of file
				#endif
		} else {

	// LKTOKEN  _@M408_ = "Logger inactive, insufficient storage!" 
			MessageBoxX(MsgToken(408),
	// LKTOKEN  _@M404_ = "Logger Error" 
			MsgToken(404), mbOk);
			StartupStore(TEXT("------ Logger not started: Insufficient Storage%s"),NEWLINE);
		}
		IGCWriteLock=false; 
	}
	FullScreen();
  }
}


void guiStopLogger(bool noAsk) {
  if (LoggerActive) {
    if(noAsk || 
	// LKTOKEN  _@M669_ = "Stop Logger" 
       (MessageBoxX(MsgToken(669),
	// LKTOKEN  _@M669_ = "Stop Logger" 
		    MsgToken(669),
		    mbYesNo) == IdYes)) {
      StopLogger();
      if (!noAsk) {
	// force landing for paragliders..
	if ( (ISPARAGLIDER) && CALCULATED_INFO.Flying && 
		((GPS_INFO.Speed <= TakeOffSpeedThreshold) || GPS_INFO.NAVWarning) ) {
			// force landing event from TakeoffLanding
			LKSW_ForceLanding=true;
			StartupStore(_T(". Logger stopped manually, landing is forced%s"),NEWLINE);
	}
		
        FullScreen();
      }
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
