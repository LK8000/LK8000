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

#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include "utils/stl_utils.h"
#include "utils/tstring.h"
#include "Screen/LKFont.h"

class LKSurface;

class Window {
public:
    // many different ways to register
    virtual BOOL RegisterWindow();
    virtual BOOL RegisterWindow(UINT style, HICON hIcon, HCURSOR hCursor, HBRUSH hbrBackground, LPCTSTR lpszMenuName, LPCTSTR lpszClassName);
    virtual BOOL RegisterWindow(const WNDCLASS* wcx);

    // static message handler to put in WNDCLASSEX structure
    static LRESULT CALLBACK stWinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // just so you can change the window caption...

    virtual void SetText(const TCHAR* lpszText) {
        assert(lpszText);
        _szWindowText = lpszText;
        if(_hWnd) {
            // already exist
            ::SetWindowText(_hWnd, _szWindowText.c_str());
        }
        // else, Text is set by CreateWindow().
    }

    virtual bool Create(Window* pOwner, const RECT& rect);

    HWND Handle() {
        return _hWnd;
    }

    void Move(const RECT& rc, bool bRepaint) {
        ::MoveWindow(_hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, bRepaint);
    }

    void Visible(bool Visible) {
        ::ShowWindow(_hWnd, Visible ? SW_SHOW : SW_HIDE);
    }

    bool IsVisible() {
        return ::IsWindowVisible(_hWnd);
    }

    void Enable(bool Enable) {
        ::EnableWindow(_hWnd, Enable);
    }

    bool IsEnabled() {
        return ::IsWindowEnabled(_hWnd);
    }

    void SetToForeground() {
        ::ShowWindow(_hWnd, SW_SHOWNORMAL);
        ::BringWindowToTop(_hWnd);
        ::SetActiveWindow(_hWnd);
    }

    RECT GetClientRect() const {
        RECT rc;
        ::GetClientRect(_hWnd, &rc);
        return rc;
    }

    void Close() {
        ::SendMessage(_hWnd, WM_CLOSE, 0, 0);
    }

    void Destroy() {
        ::DestroyWindow(_hWnd);
        _hWnd = NULL;
    }

    void SetFont(const LKFont& Font) {
        ::SendMessage(_hWnd, WM_SETFONT, (WPARAM)(HFONT)Font, MAKELPARAM(TRUE,0));
    }

    void SetFocus() {
        ::SetFocus(_hWnd);
    }

    void Redraw(const RECT& Rect) {
        ::InvalidateRect(_hWnd, &Rect, FALSE);
    }

    void SetTopWnd() {
        ::SetWindowPos(_hWnd, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
    }

protected:
    HWND _hWnd;
    DWORD _dwStyles;
    std::tstring _szClassName;
    std::tstring _szWindowText;
    WNDPROC _OriginalWndProc;
    
    //contructor
    Window(CONST WNDCLASS* wcx = NULL) : _hWnd(), _dwStyles(), _OriginalWndProc() {
        if (wcx != NULL) {
            RegisterWindow(wcx);
        }
    }
    
    void SetHandle(HWND hwnd) {
        _hWnd = hwnd;
    }    

    void SubClassWindow();

    // the real message handler
    virtual LRESULT CALLBACK WinMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // returns a pointer the window (stored as the WindowLong)

    inline static Window *GetObjectFromWindow(HWND hWnd) {
        return (Window *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
    }

    virtual HBRUSH OnCtlColor(HDC hdc) { return NULL; }

protected:
    // Event Handling virtual function ( return true for ignore default process ) :
    virtual bool OnCreate(int x, int y, int cx, int cy) { return false; }
    virtual bool OnClose() { return false; }
    virtual bool OnDestroy() { return false; }
    virtual bool OnSize(int cx, int cy) { return false; }

    virtual bool OnPaint(LKSurface& Surface, const RECT& Rect) { return false; }

    virtual bool OnKillFocus() { return false; }

	virtual bool OnMouseMove(const POINT& Pos) { return false; }

	virtual bool OnLButtonDown(const POINT& Pos) { return false; }
    virtual bool OnLButtonUp(const POINT& Pos) { return false; }

	virtual bool OnLButtonDblClick(const POINT& Pos) { return false; }

    virtual bool OnKeyDown(unsigned KeyCode) { return false; }
    
};

#endif