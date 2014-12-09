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

#include "WndPaint.h"
#include "DoubleClick.hpp"

class Event;

class WndMainBase : public WndPaint {
public:
    WndMainBase() { }
    virtual ~WndMainBase() { }

    bool Create(const RECT& rect, const TCHAR* szName) {
        return WndPaint::Create(NULL, rect, szName);
    }

    void FullScreen();
    
    void Refresh();

    bool OnEvent(const Event &event);
    
    /**
     * Check if the specified event should be allowed.  An event may be
     * rejected when a modal dialog is active, and the event should go
     * to a window outside of the dialog.
     */
    gcc_pure
    bool FilterEvent(const Event &event, Window *allowed) const;
    
    bool FilterMouseEvent(RasterPoint pt, Window *allowed) const;
    
protected:
    DoubleClick double_click;
};

#endif	/* WndMainBase_H */

