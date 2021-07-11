/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
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
  if (playsound) {
    PlayResource(TEXT("IDR_WAV_DRIP"));
  }

  tstring temp_string; // for store [text + " " + data] if needed.
  const TCHAR* szText = LKGetText(text); // get translated text

  if (data && _tcslen(data) > 0) {
    // if concat translated text and data if data is not enpty string
    temp_string = szText;
    temp_string += _T(" ");
    temp_string += data;

    szText = temp_string.c_str(); // replace translated text by concat string
  }
  Message::AddMessage(1500, 1, szText); // message time 1.5s
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
