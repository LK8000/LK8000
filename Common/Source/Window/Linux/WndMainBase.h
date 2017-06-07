/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndMainBase.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 9 novembre 2014, 14:49
 */

#ifndef WndMainBase_H
#define	WndMainBase_H

#include "Event/Globals.hpp"
#include "Event/Queue.hpp"
#include "Event/Event.h"
#include "Screen/SingleWindow.hpp"
#include "WndPaint.h"

class WndMainBase : public WndPaint<SingleWindow> {
    typedef WndPaint<SingleWindow> __super;
public:
    WndMainBase() { }

    bool Create(const RECT& rect) {
        TopWindowStyle style;
        style.EnableDoubleClicks();
#ifdef USE_FULLSCREEN
        style.FullScreen();
#endif
        __super::Create(_T("LK8000"), rect.GetSize(), style);
        return this->IsDefined();
    }

    virtual void Redraw(const RECT& Rect) { 
        __super::Redraw(Rect);
        PostRedrawEvent();
    }

    virtual void Redraw() {
        __super::Redraw();
        PostRedrawEvent();
    }

    virtual void SetWndText(const TCHAR* lpszText) { assert(false); }
    virtual const TCHAR* GetWndText() const { assert(false); return _T(""); }

    void RunModalLoop() {
        this->RunEventLoop();
    }

    static void PostRedrawEvent() {
#if defined(ENABLE_SDL)
#if !(SDL_MAJOR_VERSION >= 2)
        SDL_Event event;
        event.type = SDL_VIDEOEXPOSE;
        ::SDL_PushEvent(&event);
#endif
#else
        Event event(Event::NOP);
        event_queue->Push(event);
#endif
    }
};

#endif	/* WndMainBase_H */

