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
#include "utils/stl_utils.h"
#include "utils/tstring.h"
#include "Screen/LKFont.h"
#include "../WindowAbstract.h"

class LKSurface;

class Window : public WindowAbstrat {
public:
    // many different ways to register
    virtual BOOL RegisterWindow();
    virtual BOOL RegisterWindow(UINT style, HICON hIcon, HCURSOR hCursor, HBRUSH hbrBackground, LPCTSTR lpszMenuName, LPCTSTR lpszClassName);
    virtual BOOL RegisterWindow(const WNDCLASS* wcx);

    // static message handler to put in WNDCLASSEX structure
    static LRESULT CALLBACK stWinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // just so you can change the window caption...

    void SetWindowTitle(LPCTSTR lpszTitle) {
        _szWindowTitle = lpszTitle;
    };

    virtual bool Create(DWORD dwStyles, const RECT& rect);

    HWND Handle() {
        return _hWnd;
    }

    void Move(const RECT& rc, bool bRepaint) {
        ::MoveWindow(_hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, bRepaint);
    }

    void Visible(bool Visible) {
        ::ShowWindow(_hWnd, Visible ? SW_SHOW : SW_HIDE);
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


protected:
    HWND _hWnd;
    std::tstring _szClassName;
    std::tstring _szWindowTitle;

    //contructor
    Window(CONST WNDCLASS* wcx = NULL) : _hWnd() {
        if (wcx != NULL) {
            RegisterWindow(wcx);
        }
    }

    // the real message handler
    virtual LRESULT CALLBACK WinMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // returns a pointer the window (stored as the WindowLong)

    inline static Window *GetObjectFromWindow(HWND hWnd) {
        return (Window *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
    }

};

#endif