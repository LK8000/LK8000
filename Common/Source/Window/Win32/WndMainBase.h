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

#if defined(UNDER_CE) && ((UNDER_CE >= 300) || (_WIN32_WCE >= 0x0300))
    #include <aygshell.h>
    #define HAVE_ACTIVATE_INFO
#endif

#include "WndPaint.h"
#include "Compiler.h"

class Event;

class WndMainBase : public WndPaint {
public:
    WndMainBase();
    virtual ~WndMainBase();

    using WndPaint::Create;

    bool Create(const RECT& rect);

    void Fullscreen();
    void RunModalLoop();

    void Resize(int width, int height) {
        ::SetWindowPos(_hWnd, nullptr, 0, 0, width, height,
                   SWP_NOMOVE | SWP_NOZORDER |
                   SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    }

    inline void PostQuit() { 
        
    }

    /**
     * Check if the specified event should be allowed.  An event may be
     * rejected when a modal dialog is active, and the event should go
     * to a window outside of the dialog.
     */
    gcc_pure
    bool FilterEvent(const Event &event, Window *allowed) const;

    inline void UnGhost() {}

protected:
    virtual LRESULT CALLBACK WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual void OnCreate();
    virtual void OnDestroy();

private:
    HWND _hWndFocus;

#ifdef HAVE_ACTIVATE_INFO
    SHACTIVATEINFO s_sai;
    bool api_has_SHHandleWMActivate;
    bool api_has_SHHandleWMSettingChange;
#endif

};

#endif	/* WndMainBase_H */
