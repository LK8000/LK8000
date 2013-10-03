/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Message.cpp,v 8.3 2010/12/12 15:48:25 root Exp root $
*/

#include "externs.h"
#include "Message.h"
#include "InfoBoxLayout.h"
#include "TraceThread.h"


/*

  - Single window, created in GUI thread.
     -- hidden when no messages for display
     -- shown when messages available
     -- disappear when touched
     -- disappear when return clicked
     -- disappear when timeout
     -- disappear when extern event triggered
  - Message properties
     -- have a start time (seconds)
     -- timeout (start time + delta)
  - Messages stay in a circular buffer can be reviewed
  - Optional logging of all messages to file
  - Thread locking so available from any thread

*/

CRITICAL_SECTION  CritSec_Messages;

extern HWND hWndMainWindow; // Main Windows
extern HINSTANCE hInst;      // The current instance

RECT Message::rcmsg;
HWND Message::hWndMessageWindow;
HDC Message::hdc;
struct singleMessage Message::messages[MAXMESSAGES];
bool Message::hidden=false;
int Message::nvisible=0;

TCHAR Message::msgText[2000];

// Get start time to reduce overrun errors
DWORD startTime = ::GetTickCount();

// Intercept messages destined for the Status Message window
LRESULT CALLBACK MessageWindowProc(HWND hwnd, UINT message, 
				   WPARAM wParam, LPARAM lParam)
{

  POINT pt;
  RECT  rc;
  static bool capturedMouse = false;

  SHOWTHREAD(_T("MessageWindowProc"));

  switch (message) {
  case WM_LBUTTONDOWN:

    // Intercept mouse messages while stylus is being dragged
    // This is necessary to simulate a WM_LBUTTONCLK event
    SetCapture(hwnd);
    capturedMouse = TRUE;

    return 0;
  case WM_LBUTTONUP :

    ReleaseCapture();
    if (!capturedMouse) return 0;
    capturedMouse = FALSE;

    // Is stylus still within this window?
    pt.x = LOWORD(lParam);
    pt.y = HIWORD(lParam);
    GetClientRect(hwnd, &rc);

    if (!PtInRect(&rc, pt)) return 0;

    DestroyWindow(hwnd);
    return 0;
  }
  return DefWindowProc(hwnd, message, wParam, lParam);
}


int Message::block_ref = 0;

void Message::Initialize(RECT rc) {

  InitializeCriticalSection(&CritSec_Messages);

  block_ref = 0;

  rcmsg = rc; // default; message window can be full size of screen

#if 100215
#if 0
  hWndMessageWindow = CreateWindowEx( WS_EX_STATICEDGE|WS_EX_OVERLAPPEDWINDOW,
				     TEXT("EDIT"), TEXT(" "),
				   WS_VISIBLE|WS_CHILD|ES_MULTILINE|ES_CENTER
				   |WS_BORDER|ES_READONLY | WS_CLIPCHILDREN 
				   | WS_CLIPSIBLINGS| WS_THICKFRAME,
				   0,0,0,0,hWndMainWindow,NULL,hInst,NULL);
#else
  hWndMessageWindow = CreateWindow( TEXT("EDIT"), TEXT(" "),
				   WS_VISIBLE|WS_CHILD|ES_MULTILINE|ES_CENTER
				   |WS_BORDER|ES_READONLY | WS_CLIPCHILDREN 
				   | WS_CLIPSIBLINGS,
				   0,0,0,0,hWndMainWindow,NULL,hInst,NULL);


#endif
#else
  hWndMessageWindow = CreateWindow(// WS_EX_CLIENTEDGE,
				     TEXT("EDIT"), TEXT(" "),
				   WS_VISIBLE|WS_CHILD|ES_MULTILINE|ES_CENTER
				   |WS_BORDER|ES_READONLY | WS_CLIPCHILDREN 
				   | WS_CLIPSIBLINGS,
				   0,0,0,0,hWndMainWindow,NULL,hInst,NULL);

#endif

  SetWindowPos(hWndMessageWindow, HWND_TOP,
	       rcmsg.left, rcmsg.top, 
	       rcmsg.right-rcmsg.left, rcmsg.bottom-rcmsg.top,
	       SWP_SHOWWINDOW);
  ShowWindow(hWndMessageWindow, SW_HIDE);

  // change message font for different resolutions
  // Caution, remember to set font also in Resize..
  if (ScreenSize > sslandscape )
	SendMessage(hWndMessageWindow, WM_SETFONT, (WPARAM)LK8InfoBigFont,MAKELPARAM(TRUE,0));
  else
	SendMessage(hWndMessageWindow, WM_SETFONT, (WPARAM)MapWindowBoldFont,MAKELPARAM(TRUE,0));

  /*
  SetWindowLong(hWndMessageWindow, GWL_WNDPROC, 
		(LONG) MessageWindowProc);
  EnableWindow(hWndMessageWindow, FALSE); // prevent window receiving
					  // keyboard/mouse input
  */

  hdc = GetDC(hWndMessageWindow);

  hidden = false;
  nvisible = 0;
  
  //  for (x=0; TabStops[x] != 0 && x < 10; x++);
  //  SendMessage(hWnd, EM_SETTABSTOPS, (WPARAM)x, (LPARAM)TabStops);

  int i;
  for (i=0; i<MAXMESSAGES; i++) {
    messages[i].text[0]= _T('\0');
    messages[i].tstart = 0;
    messages[i].texpiry = 0;
    messages[i].type = 0;
  }

}


void Message::Destroy() {
  // destroy window
  ReleaseDC(hWndMessageWindow, hdc);
  DestroyWindow(hWndMessageWindow);
  DeleteCriticalSection(&CritSec_Messages);
}


void Message::Lock() {
  EnterCriticalSection(&CritSec_Messages);
}

void Message::Unlock() {
  LeaveCriticalSection(&CritSec_Messages);
}



void Message::Resize() {
  SIZE tsize;
  int size = _tcslen(msgText);
  RECT rthis;
  //  RECT mRc;

  if (size==0) {
    if (!hidden) {
      ShowWindow(hWndMessageWindow, SW_HIDE);

      MapWindow::RequestFastRefresh();      
    }
    hidden = true;
  } else {
    SetWindowText(hWndMessageWindow, msgText);

    HFONT oldfont;
    if (ScreenSize > sslandscape )
	oldfont=(HFONT)SelectObject(hdc,LK8InfoBigFont);
    else
	oldfont=(HFONT)SelectObject(hdc,MapWindowBoldFont);

    GetTextExtentPoint(hdc, msgText, size, &tsize);

    SelectObject(hdc,oldfont); // 100215

    int linecount = max(nvisible,max(1,
			(int)SendMessage(hWndMessageWindow, 
				    EM_GETLINECOUNT, 0, 0)));

    int width =// min((rcmsg.right-rcmsg.left)*0.8,tsize.cx);
      (int)((rcmsg.right-rcmsg.left)*0.9);
    int height = (int)min((rcmsg.bottom-rcmsg.top)*0.8,(double)tsize.cy*(linecount+1));
    int h1 = height/2;
    int h2 = height-h1;

    int midx = (rcmsg.right+rcmsg.left)/2;
    int midy = (rcmsg.bottom+rcmsg.top)/2;

    rthis.left = midx-width/2;
    rthis.right = midx+width/2;
    rthis.top = midy-h1;
    rthis.bottom = midy+h2;

    SetWindowPos(hWndMessageWindow, HWND_TOP,
		 rthis.left, rthis.top,
		 rthis.right-rthis.left, 
		 rthis.bottom-rthis.top,
		 SWP_SHOWWINDOW);
    hidden = false;

    // window has resized potentially, so redraw map to reduce artifacts
    MapWindow::RequestFastRefresh();

  }

}


void Message::BlockRender(bool doblock) {
  //Lock();
  if (doblock) {
    block_ref++;
  } else {
    block_ref--;
  }
  // TODO code: add blocked time to messages' timers so they come
  // up once unblocked.
  //Unlock();
}


bool Message::Render() {
  if (!GlobalRunning) return false;
  if (block_ref) return false;

  Lock();
  DWORD	fpsTime = ::GetTickCount() - startTime;

  // this has to be done quickly, since it happens in GUI thread
  // at subsecond interval

  // first loop through all messages, and determine which should be
  // made invisible that were previously visible, or
  // new messages

  bool changed = false;
  int i;
  for (i=0; i<MAXMESSAGES; i++) {
    if (messages[i].type==0) continue; // ignore unknown messages

    if (
	(messages[i].texpiry <= fpsTime)
	&&(messages[i].texpiry> messages[i].tstart)
	) {
      // this message has expired for first time
      changed = true;
      continue;
    }

    // new message has been added
    if (messages[i].texpiry== messages[i].tstart) {
      // set new expiry time.
      messages[i].texpiry = fpsTime + messages[i].tshow;
      // this is a new message..
      changed = true;
    }

  }

  static bool doresize= false;
  
  if (!changed) { 
    if (doresize) {
      doresize = false;
      // do one extra resize after display so we are sure we get all
      // the text (workaround bug in getlinecount)
      Resize();
    }
    Unlock(); return false; 
  }

  // ok, we've changed the visible messages, so need to regenerate the
  // text box

  doresize = true;
  msgText[0]= 0;
  nvisible=0;
  for (i=0; i<MAXMESSAGES; i++) {
    if (messages[i].type==0) continue; // ignore unknown messages

    if (messages[i].texpiry< fpsTime) {
      messages[i].texpiry = messages[i].tstart-1; 
      // reset expiry so we don't refresh
      continue;
    }

    _tcscat(msgText, messages[i].text);
    // this is a line separator, so we don't need it if it is the last line
    _tcscat(msgText, TEXT("\r\n"));
    nvisible++;

  }
  // remove \r\n from last line of message
  i=_tcslen(msgText);
  if (i>3) {
	msgText[i-1]=_T('\0');
	msgText[i-2]=_T('\0');
  }


  Resize();

  Unlock();
  return true;
}



int Message::GetEmptySlot() {
  // find oldest message that is no longer visible

  // todo: make this more robust with respect to message types and if can't
  // find anything to remove..
  int i;
  DWORD tmin=0;
  int imin=0;
  for (i=0; i<MAXMESSAGES; i++) {
    if ((i==0) || (messages[i].tstart<tmin)) {
      tmin = messages[i].tstart;
      imin = i; 
    }
  }
  return imin;
}


void Message::AddMessage(DWORD tshow, int type, const TCHAR* Text) {

  Lock();

  int i;
  DWORD	fpsTime = ::GetTickCount() - startTime;
  i = GetEmptySlot();

  messages[i].type = type;
  messages[i].tshow = tshow;
  messages[i].tstart = fpsTime;
  messages[i].texpiry = fpsTime;
  _tcscpy(messages[i].text, Text);

  Unlock();
  //  Render(); // NO this causes crashes (don't know why..)
}

void Message::Repeat(int type) {
  int i;
  DWORD tmax=0;
  int imax= -1;

  Lock();

  DWORD	fpsTime = ::GetTickCount() - startTime;

  // find most recent non-visible message

  for (i=0; i<MAXMESSAGES; i++) {

    if ((messages[i].texpiry < fpsTime)
	&&(messages[i].tstart > tmax)
	&&((messages[i].type == type)|| (type==0))) {
      imax = i;
      tmax = messages[i].tstart;
    }

  }

  if (imax>=0) {
    messages[imax].tstart = fpsTime;
    messages[imax].texpiry = messages[imax].tstart;
  }

  Unlock();
}


void Message::CheckTouch(HWND wmControl) {
  if (wmControl == hWndMessageWindow) {
    // acknowledge with click/touch
    Acknowledge(0);
  }
}


bool Message::Acknowledge(int type) {
  Lock();
  int i;
  bool ret = false;	// Did we acknowledge?
  DWORD	fpsTime = ::GetTickCount() - startTime;

  for (i=0; i<MAXMESSAGES; i++) {
    if ((messages[i].texpiry> messages[i].tstart)
	&& ((type==0)||(type==messages[i].type))) {
      // message was previously visible, so make it expire now.
      messages[i].texpiry = fpsTime-1;
	  ret = true;
    }
  }

  Unlock();
  //  Render(); NO! this can cause crashes
  return ret;
}

