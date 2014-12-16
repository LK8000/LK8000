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

#include "Screen/SingleWindow.hpp"
#include "WndPaint.h"

class WndMainBase : public virtual WndPaint<SingleWindow> {
    typedef WndPaint<SingleWindow> __super;
public:
    WndMainBase() { }

    bool Create(const RECT& rect) {
        TopWindowStyle style;
        style.EnableDoubleClicks();
        const SIZE size = rect.GetSize();
        __super::Create(_T("LK8000"), size, style);
        return this->IsDefined();
    }

    virtual void Redraw(const RECT& Rect) { 
        __super::Redraw(Rect);
    }

    virtual void Redraw() {
        __super::Redraw();
    }
    
    virtual void SetWndText(const TCHAR* lpszText) { assert(false); }
    virtual const TCHAR* GetWndText() const { assert(false); return _T(""); }

    void RunModalLoop() {
        this->RunEventLoop();
    }
};

#endif	/* WndMainBase_H */

