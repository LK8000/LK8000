/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Dialogs.cpp,v 8.3 2010/12/10 23:57:13 root Exp root $

*/

#include "externs.h"
#include "resource.h"
#include "Message.h"
#include "Sound/Sound.h"

// DoMessage is designed to delegate what to do for a message
// The "what to do" can be defined in a configuration file
// Defaults for each message include:
//	- Text to display (including multiple languages)
//	- Text to display extra - NOT multiple language
//		(eg: If Airspace Warning - what details - airfield name is in data file, already
//		covers multiple languages).
//	- ShowStatusMessage - including font size and delay
//	- Sound to play - What sound to play
//	- Log - Keep the message on the log/history window (goes to log file and history)
//
// TODO code: (need to discuss) Consider moving almost all this functionality into AddMessage ?

void DoStatusMessage(const TCHAR* text, const TCHAR *data, const bool playsound) {
  Message::Lock();

  if (playsound)
    PlayResource(TEXT("IDR_WAV_DRIP"));

  // TODO code: consider what is a sensible size?
  TCHAR msgcache[1024];
	LK_tcsncpy(msgcache,gettext(text),800);
	if (data != NULL) {
		_tcscat(msgcache, TEXT(" "));
		_tcscat(msgcache, data);
	}
  Message::AddMessage(1500, 1, msgcache); // message time 1.5s

  Message::Unlock();
}


#ifdef WIN32
static HCURSOR oldCursor = NULL;
static unsigned CursorCount = 0;
#endif

void StartHourglassCursor(void) {
#ifdef WIN32
    if(CursorCount == 0) {
        HCURSOR newc = LoadCursor(NULL, IDC_WAIT);
        if (newc) {
            oldCursor = (HCURSOR) SetCursor(newc);
        }
    }
    ++CursorCount;
#endif
}

void StopHourglassCursor(void) {
#ifdef WIN32
    --CursorCount;
    if(CursorCount == 0) {
        SetCursor(oldCursor);
        oldCursor = NULL;
    }
#endif
}
