/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: Message.cpp,v 8.3 2010/12/12 15:48:25 root Exp root $
 */

#include "externs.h"
#include "Message.h"
#include "Screen/Point.hpp"
#include "Screen/LKSurface.h"
#include "Window/WndTextEdit.h"
#include "Event/Event.h"
#include <chrono>
#include <optional>

class WndMessage : public WndTextEdit {
public:
    using WndTextEdit::WndTextEdit;

protected:
    bool OnLButtonDown(const POINT& Pos) override {
        // requiered otherwise on Win32 this windows capture all event and never release.
        return true;
    }

    bool OnLButtonUp(const POINT& Pos) override {
        Message::Acknowledge(0);
        return true;
    }

    bool OnKeyDown(unsigned KeyCode) override {
        if(KeyCode == KEY_RETURN) {
            Message::Acknowledge(0);
            return true;
        }
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

Mutex  CritSec_Messages; // Recusive Mutex Needed

PixelRect Message::rcmsg;
WndMessage Message::WndMsg;

Message::messages_t Message::messages; // from older to newer
Message::messages_t Message::messagesHistory; // from newer to older

bool Message::hidden = false;

tstring Message::msgText;

int Message::ScopeBlockRender::_Block = 0;

void Message::Initialize(PixelRect rc) {
    hidden = true;
    rcmsg = rc; // default; message window can be full size of screen

    WndMsg.Create(main_window.get(), rc);

    InitFont();
}

void Message::InitFont() {
    // change message font for different resolutions
    // Caution, remember to set font also in Resize..
    WndMsg.SetFont(ScreenLandscape ? LK8InfoBigFont : MapWindowBoldFont);
    WndMsg.SetTextColor(RGB_BLACK);
    WndMsg.SetBkColor(RGB_WHITE);
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
  const size_t size = msgText.size();

  if (size==0) {
    if (!hidden) {
        WndMsg.SetVisible(false);
    }
    hidden = true;
  }
  else {
    WndMsg.SetWndText(msgText.c_str());

    LKWindowSurface Surface(WndMsg);
    const auto oldfont = Surface.SelectObject(ScreenLandscape
                                              ? LK8InfoBigFont
                                              : MapWindowBoldFont);

    PixelSize tsize;
    Surface.GetTextSize(msgText.c_str(), &tsize);
    assert(tsize.cx > 0);
    assert(tsize.cy > 0);

    Surface.SelectObject(oldfont); // 100215

    const int linecount = std::max(1, WndMsg.GetLineCount());

    const auto center = rcmsg.GetCenter();
    const auto size = rcmsg.GetSize();

    const auto width = static_cast<PixelScalar>(size.cx * 0.9);
    const auto height = std::min<PixelScalar>(size.cy * 0.8,
                                              tsize.cy * (linecount + 1));
    const auto h1 = static_cast<PixelScalar>(height / 2);
    const auto h2 = static_cast<PixelScalar>(height - h1);


    const PixelRect rthis = {
      center.x - width / 2,
      center.y - h1,
      center.x + width / 2,
      center.y + h2
    };

    WndMsg.SetTopWnd();
    WndMsg.Move(rthis);
    WndMsg.SetVisible(true);
    hidden = false;
  }
#ifndef USE_GDI
  main_window->Refresh();
#endif
}


void Message::Render() {
    if (!GlobalRunning || ScopeBlockRender::isBlocked()) {
        return;
    }

    // this has to be done quickly, since it happens in GUI thread
    // at subsecond interval
    const tstring previousText = std::move(msgText);
    msgText.clear();

    {
        const std::lock_guard lock(CritSec_Messages);
        const auto now = steady_clock::now();

        messages_t::iterator It = messages.begin();
        while (It != messages.end()) {
            if (It->type == 0) {
                // ignore unknown messages, remove it.
                messages.erase(It++);
                continue;
            }

            // if message is pending (not yet rendered), start its timer now
            if (It->pending()) {
                It->tstart = now;
            }

            // check if message has expired: current time > start time + show duration
            if (It->expire() < now) {
                // this message has expired, move to history list
                messagesHistory.splice(messagesHistory.begin(), messages, It++);
                continue;
            }

            if (!msgText.empty()) {
                msgText += _T("\r\n"); // add a line separator
            }
            msgText += It->text; // append text

            ++It; // advance to next
        }
        while(messagesHistory.size() > 20) {
            // don't save more than 20 message into history.
            messagesHistory.pop_back();
        }
    } // lock_guard released here

    if (msgText != previousText) {
        Resize();
    }
}

void Message::AddMessage(unsigned tshow, int type, const TCHAR* text) {
    if (!GlobalRunning || !text) {
        return;
    }
    TestLog(_T("Message::AddMessage: %s"), text);

    const std::lock_guard lock(CritSec_Messages);

    auto It = std::find_if(messages.begin(), messages.end(),
                [&](const Message_t& Item) {
                    return (Item.type == type && Item.text == text);
                });

    if (It != messages.end()) {
        // this message is already visible, restart its timer now
        It->tshow = duration_ms(tshow);
        if (!It->pending()) {
            It->tstart = steady_clock::now();
        }
        messages.splice(messages.end(), messages, It);
    }
    else {
        // new message: timer starts on first render
        messages.emplace_back(text, type, duration_ms(tshow));
    }
}

void Message::Repeat(int type) {
    const std::lock_guard lock(CritSec_Messages);

    // copy most recent message from history to active message.
    auto It = std::find_if(messagesHistory.begin(), messagesHistory.end(),
                [&](const Message_t& item) {
                    return type == 0 || item.type == type;
                });

    if (It != messagesHistory.end()) {
        // Reset message state: timer starts on first render (pending=true)
        It->tstart = std::nullopt;  // timer hasn't started yet
        messages.splice(messages.end(), messagesHistory, It);
    }
}

bool Message::Acknowledge(int type) {
    const std::lock_guard lock(CritSec_Messages);
    bool ret = false; // did we acknowledge?

    messages_t::iterator It = messages.begin();
    while (It != messages.end()) {
        if (type == 0 || It->type == type) {
            messagesHistory.splice(messagesHistory.begin(), messages, It++);
            ret = true;
        }
        else {
            ++It;
        }
    }

    //  Render(); NO! this can cause crashes
    return ret;
}
