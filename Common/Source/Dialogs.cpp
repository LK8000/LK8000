/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Dialogs.cpp,v 8.3 2010/12/10 23:57:13 root Exp root $

*/

#include "externs.h"

#include <commdlg.h>
#include <commctrl.h>
#include "aygshell.h"
#include "resource.h"
#include "Message.h"



// ARH: Status Message functions
// Used to show a brief status message to the user
// Could be used to display debug messages
// or info messages like "Map panning OFF"

// Each instance of the StatusMessage window has some
// unique data associated with it, rather than using
// global variables.  This allows multiple instances
// in a thread-safe manner.
class CStatMsgUserData {
public:
  HFONT   hFont;
  WNDPROC fnOldWndProc;
  BOOL    bCapturedMouse;
  DWORD   texpiry; //
  
  // Initialize to sensible values
  CStatMsgUserData()
    : hFont(NULL), fnOldWndProc(NULL), bCapturedMouse(FALSE), texpiry(0) {};
  
  // Clean up mess
  ~CStatMsgUserData() {
    if (hFont) {
      DeleteObject(hFont);
      hFont = NULL;
    }
    fnOldWndProc = NULL;
  }
};


bool forceDestroyStatusMessage = false;

void ClearStatusMessages(void) {
  forceDestroyStatusMessage = true;
}


// Intercept messages destined for the Status Message window
LRESULT CALLBACK StatusMsgWndTimerProc(HWND hwnd, UINT message, 
				       WPARAM wParam, LPARAM lParam)
{

  CStatMsgUserData *data;
  POINT pt;
  RECT  rc;

  // Grab hold of window specific data
  data = (CStatMsgUserData*) GetWindowLong(hwnd, GWL_USERDATA);

/*
  if (data==NULL) {
    // Something wrong here!
    DestroyWindow(hwnd);  // ups
    return 1;
  }
*/

  switch (message) {
  case WM_LBUTTONDOWN:

    if (data==NULL) {
      // Something wrong here!
      DestroyWindow(hwnd);  // ups
      return 1;
    }

    // Intercept mouse messages while stylus is being dragged
    // This is necessary to simulate a WM_LBUTTONCLK event
    SetCapture(hwnd);
    data->bCapturedMouse = TRUE;
    return 0;
  case WM_LBUTTONUP :

    if (data==NULL) {
      // Something wrong here!
      DestroyWindow(hwnd);  // ups
      return 1;
    }

    //if (data->bCapturedMouse) ReleaseCapture();
    ReleaseCapture();

    if (!data->bCapturedMouse) return 0;

    data->bCapturedMouse = FALSE;

    // Is stylus still within this window?
    pt.x = LOWORD(lParam);
    pt.y = HIWORD(lParam);
    GetClientRect(hwnd, &rc);

    if (!PtInRect(&rc, pt)) return 0;

    DestroyWindow(hwnd);
    return 0;

  case WM_TIMER :

    // force destruction of window...
    if (forceDestroyStatusMessage) {
      DestroyWindow(hwnd);
      return 0;
    }

    if ((data->texpiry>0) && (::GetTickCount()>data->texpiry)) {
      DestroyWindow(hwnd);
    }
    return 0;
  case WM_DESTROY :

    // Clean up after ourselves
    if (data != NULL){
      delete data;
      // hack ... try to find execption point
      data = NULL;
      // Attach window specific data to the window
      SetWindowLong(hwnd, GWL_USERDATA, (LONG) data);
    }
    MapWindow::RequestFastRefresh();
    // JMW do this so airspace warning gets refreshed

    return 0;
  }

  // Pass message on to original window proc
  if (data != NULL)
    return CallWindowProc(data->fnOldWndProc, hwnd, message, wParam, lParam);
  else
    return(0);

}


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

  StatusMessageSTRUCT LocalMessage;
  LocalMessage = StatusMessageData[0];

  int i;
  // Search from end of list (allow overwrites by user)
  for (i=StatusMessageData_Size - 1; i>0; i--) {
    #if BUGSTOP
    LKASSERT(i>=0);
    #else
    if (i<0) break;
    #endif
    if (wcscmp(text, StatusMessageData[i].key) == 0) {
      LocalMessage = StatusMessageData[i];
      break;
    }
  }

  // doSound always true, to be removed the StatusFile entirely 
  if (EnableSoundModes && LocalMessage.doSound &&playsound)
    PlayResource(LocalMessage.sound);
  
  // TODO code: consider what is a sensible size?
  TCHAR msgcache[1024];
  if (LocalMessage.doStatus) {
    
    LK_tcsncpy(msgcache,gettext(text),800);
    // wcscpy(msgcache, gettext(text));
    if (data != NULL) {
      wcscat(msgcache, TEXT(" "));
      wcscat(msgcache, data);
    }
    
    Message::AddMessage(LocalMessage.delay_ms, 1, msgcache);
  }

  Message::Unlock();
}



static HCURSOR oldCursor = NULL;

void StartHourglassCursor(void) {
  HCURSOR newc = LoadCursor(NULL, IDC_WAIT);
  oldCursor = (HCURSOR)SetCursor(newc);
  #if 0
  SetCursorPos(160,120);
  #endif
}

void StopHourglassCursor(void) {
  SetCursor(oldCursor);
  #if 0
  SetCursorPos(640,480);
  #endif
  oldCursor = NULL;
}


