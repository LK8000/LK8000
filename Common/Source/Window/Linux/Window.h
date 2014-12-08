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
#include "utils/stl_utils.h"
#include "utils/tstring.h"
#include "Screen/FontReference.h"

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

    Window* GetOwner() const;
    
    const TCHAR* GetWndName() const {
        return _szWindowName.c_str();
    }

    void Move(const RECT& rc, bool bRepaint);
    void SetVisible(bool Visible);
    bool IsVisible();
    void Enable(bool Enable);
    bool IsEnabled();
    void SetToForeground();
    RECT GetClientRect() const;
    void Close();
    void Destroy();
    void SetFont(FontReference Font);
    void SetFocus();
    bool HasFocus();
    void Redraw(const RECT& Rect);
    void Redraw();
    void SetTopWnd();
    void SetCapture();
    void ReleaseCapture();

    static Window* GetFocus();
protected:

    void StartTimer(unsigned uTime /*millisecond*/) { }
    void StopTimer() { }

protected:
    std::tstring _szWindowText;
    std::tstring _szWindowName;

    
    //contructor
    Window() {

    }

protected:
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
