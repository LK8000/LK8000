/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Window.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 9 novembre 2014, 14:49
 */

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <assert.h>
#include <tchar.h>
#include <list>
#include "utils/stl_utils.h"
#include "utils/tstring.h"
#include "Screen/FontReference.h"
#include "ScreenCoordinate.h"
#include "Screen/Point.hpp"
#include "Compiler.h"

class LKSurface;

class Window {
    friend class EventLoop;
public:
    // just so you can change the window caption...
    virtual void SetWndText(const TCHAR* lpszText) {
        _szWindowText = lpszText?lpszText:_T("");
    }

    const TCHAR* GetWndText() const { return _szWindowText.c_str(); }

    virtual bool Create(Window* pOwner, const RECT& rect, const TCHAR* szName);

    Window* GetOwner() const { return _Owner; }
    bool IsDefined() const { return _bDefined; }
    
    const TCHAR* GetWndName() const {
        return _szWindowName.c_str();
    }

    void Move(const RECT& rect, bool bRepaint);
    
    void SetVisible(bool Visible) { _bVisible = Visible; }
    bool IsVisible() const { return _bVisible; }
    
    void Enable(bool Enable) { _bEnabled = Enable; }
    bool IsEnabled() const { return _bEnabled; }
    
    RECT GetClientRect() const { return _ClientRect; }
    
    void Close();
    void Destroy();
    void SetFont(FontReference Font) { _Font = Font; }
    
    void SetFocus() { _FocusedWindow = this; }
    bool HasFocus() const { return _FocusedWindow == this; }
    
    void Redraw(const RECT& Rect);
    void Redraw() { Redraw(GetClientRect()); }
    
    void SetToForeground();
    void SetTopWnd();
    
    void SetCapture() { _Capture = true; }
    void ReleaseCapture() { _Capture = false; }

    static Window* GetFocus() { return _FocusedWindow; }

    /**
     * Locates the child which should get a mouse event.  Prefers the
     * captured child.
     */
    gcc_pure
    Window *EventChildAt(PixelScalar x, PixelScalar y) const;
    
protected:
    std::tstring _szWindowText;
    std::tstring _szWindowName;
    
    bool _bVisible;
    bool _bEnabled;
    
    POINT _Position;
    RECT _ClientRect;
    
    FontReference _Font;
    Window * _Owner;
    std::list<Window*> _lstChild;
    
    bool _bDefined;
    bool _Capture;

    static Window* _FocusedWindow;
    
    //contructor
    Window();

    void StartTimer(unsigned uTime /*millisecond*/);
    void StopTimer();
    
    void AddChild(Window* pWnd);
    void RemoveChild(Window* pWnd);
    
    virtual bool OnUser(unsigned id);

    
public:
    /**
     * Checks if the window wishes to handle a special key, like cursor
     * keys and tab.  This wraps the WIN32 message WM_GETDLGCODE.
     *
     * @return true if the window will handle they key, false if the
     * dialog manager may use it
     */
    gcc_pure
    virtual bool OnKeyCheck(unsigned key_code) const;    
    
    // Event Handling virtual function ( return true for ignore default process ) :
    virtual bool OnCreate(int x, int y, int cx, int cy) { return false; }
    virtual bool OnClose() { return false; }
    virtual bool OnDestroy() { return false; }
    virtual bool OnSize(int cx, int cy) { return false; }

    virtual bool OnPaint(LKSurface& Surface, const RECT& Rect) { return false; }

    virtual bool OnSetFocus() { return false; }
    virtual bool OnKillFocus() { return false; }

    virtual bool OnMouseMove(const POINT& Pos) { return false; }

    virtual bool OnLButtonDown(const POINT& Pos) { return false; }
    virtual bool OnLButtonUp(const POINT& Pos) { return false; }

    virtual bool OnLButtonDblClick(const POINT& Pos) { return false; }

    virtual bool OnKeyDown(unsigned KeyCode) { return false; }
    virtual bool OnKeyUp(unsigned KeyCode) { return false; }

    virtual bool OnKeyDownNotify(Window* pWnd, unsigned KeyCode) { return false; }
    virtual bool OnKeyUpNotify(Window* pWnd, unsigned KeyCode) { return false; }

    virtual bool OnLButtonDownNotify(Window* pWnd, const POINT& Pos) { return false; }
    virtual bool OnLButtonUpNotify(Window* pWnd, const POINT& Pos) { return false; }

    virtual bool OnTimer() { return false; }
};

#endif
