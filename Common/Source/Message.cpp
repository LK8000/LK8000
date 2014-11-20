/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Message.cpp,v 8.3 2010/12/12 15:48:25 root Exp root $
*/

#include "externs.h"
#include "Message.h"
#include "TraceThread.h"
#include "Screen/LKSurface.h"

class WndMessage : public WndTextEdit {
public:
    WndMessage() : WndTextEdit() { }
    
protected:
    virtual bool OnLButtonUp(const POINT& Pos) {
        Message::Acknowledge(0);
        return false;
    }
};

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

Poco::Mutex  CritSec_Messages;

RECT Message::rcmsg;
WndMessage Message::WndMsg;
struct singleMessage Message::messages[MAXMESSAGES];
bool Message::hidden=false;
int Message::nvisible=0;

TCHAR Message::msgText[2000];

// Get start time to reduce overrun errors
Poco::Timestamp startTime;

int Message::block_ref = 0;

void Message::Initialize(RECT rc) {

  block_ref = 0;
    hidden = true;
    nvisible = 0;
    rcmsg = rc; // default; message window can be full size of screen

    WndMsg.Create(&MainWindow, rc);

    // change message font for different resolutions
    // Caution, remember to set font also in Resize..
    WndMsg.SetFont(ScreenLandscape ? LK8InfoBigFont : MapWindowBoldFont);
    WndMsg.SetTextColor(LKColor(0x00,0x00,0x00));
    WndMsg.SetBkColor(LKColor(0xFF,0xFF,0xFF));

  for (unsigned i=0; i<MAXMESSAGES; i++) {
    messages[i].text[0]= _T('\0');
    messages[i].tstart = 0;
    messages[i].texpiry = 0;
    messages[i].type = 0;
  }

}


void Message::Destroy() {
  // destroy window
    WndMsg.Destroy();
}


void Message::Lock() {
  CritSec_Messages.lock();
}

void Message::Unlock() {
  CritSec_Messages.unlock();
}



void Message::Resize() {
  SIZE tsize;
  const int size = _tcslen(msgText);
  RECT rthis;
  //  RECT mRc;

  if (size==0) {
    if (!hidden) {
            WndMsg.Visible(false);
    }
    hidden = true;
  } else {
    
    WndMsg.SetText(msgText);

    LKWindowSurface Surface(WndMsg);
    const auto oldfont = Surface.SelectObject(ScreenLandscape
                                              ? LK8InfoBigFont
                                              : MapWindowBoldFont);

    Surface.GetTextSize(msgText, size, &tsize);

    Surface.SelectObject(oldfont); // 100215

    const int linecount = max(nvisible, max(1, WndMsg.GetLineCount()));

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

    WndMsg.SetTop();
    WndMsg.Move(rthis, true);
    WndMsg.Visible(true);
    hidden = false;
    

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
  Poco::Timespan fpsTime = startTime.elapsed();

  // this has to be done quickly, since it happens in GUI thread
  // at subsecond interval

  // first loop through all messages, and determine which should be
  // made invisible that were previously visible, or
  // new messages

  bool changed = false;
  for (unsigned i=0; i<MAXMESSAGES; i++) {
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
  for (unsigned i=0; i<MAXMESSAGES; i++) {
    if (messages[i].type==0) continue; // ignore unknown messages

    if (messages[i].texpiry< fpsTime) {
      messages[i].texpiry = messages[i].tstart-1; 
      // reset expiry so we don't refresh
      continue;
    }
    if(nvisible > 0) {
        // this is a line separator
        _tcscat(msgText, TEXT("\r\n"));
    }

    _tcscat(msgText, messages[i].text);
    nvisible++;
  }

  Resize();

  Unlock();
  return true;
}



unsigned Message::GetEmptySlot() {
  // find oldest message that is no longer visible

  // todo: make this more robust with respect to message types and if can't
  // find anything to remove..
  Poco::Timespan tmin=0;
  unsigned imin=0;
  for (unsigned i=0; i<MAXMESSAGES; i++) {
    if ((i==0) || (messages[i].tstart<tmin)) {
      tmin = messages[i].tstart;
      imin = i; 
    }
  }
  return imin;
}


void Message::AddMessage(DWORD tshow, int type, const TCHAR* Text) {

  Lock();

  Poco::Timespan fpsTime = startTime.elapsed();
  unsigned i = GetEmptySlot();

  messages[i].type = type;
  messages[i].tshow = tshow;
  messages[i].tstart = fpsTime;
  messages[i].texpiry = fpsTime;
  _tcscpy(messages[i].text, Text);

  Unlock();
  //  Render(); // NO this causes crashes (don't know why..)
}

void Message::Repeat(int type) {
  Poco::Timespan tmax=0;
  int imax= -1;

  Lock();

  Poco::Timespan fpsTime = startTime.elapsed();

  // find most recent non-visible message

  for (unsigned i=0; i<MAXMESSAGES; ++i) {

    if (((messages[i].type == type)|| (type==0))
            &&(messages[i].texpiry < fpsTime)
            &&(messages[i].tstart > tmax)) {
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

bool Message::Acknowledge(int type) {
    Lock();
    bool ret = false; // Did we acknowledge?
    const Poco::Timespan fpsTime = startTime.elapsed() - 1;

    for (unsigned i = 0; i < MAXMESSAGES; i++) {
        if (type == 0 || (type != messages[i].type)) {
            if (messages[i].texpiry > messages[i].tstart) {
                // message was previously visible, so make it expire now.
                messages[i].texpiry = fpsTime - 1;
                ret = true;
            }
        }
    }

    Unlock();
    //  Render(); NO! this can cause crashes
    return ret;
}

