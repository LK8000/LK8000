/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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
#else
        style.Resizable();
#endif
        __super::Create(_T("LK8000"), rect.GetSize(), style);
        return this->IsDefined();
    }

    void Redraw(const RECT& Rect) override {
        __super::Redraw(Rect);
        PostRedrawEvent();
    }

    void Redraw() override {
        __super::Redraw();
        PostRedrawEvent();
    }

    void SetWndText(const TCHAR* lpszText) override { assert(false); }
    const TCHAR* GetWndText() const  override { assert(false); return _T(""); }

    void RunModalLoop() {
        this->RunEventLoop();
    }

    static void PostRedrawEvent() {
#ifndef ENABLE_SDL
        event_queue->Purge(Event::NOP);
        event_queue->Push(Event(Event::NOP));
#endif
    }
};

#endif	/* WndMainBase_H */

