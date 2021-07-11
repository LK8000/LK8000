/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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
#include "Util/tstring.hpp"
#include "Screen/FontReference.h"
#include "Screen/LKFont.h"

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif


class LKSurface;

class Window {
    friend class EventLoop;
public:
    // many different ways to register
    virtual BOOL RegisterWindow();
    virtual BOOL RegisterWindow(UINT style, HICON hIcon, HCURSOR hCursor, HBRUSH hbrBackground, LPCTSTR lpszMenuName, LPCTSTR lpszClassName);
    virtual BOOL RegisterWindow(const WNDCLASS* wcx);

    // static message handler to put in WNDCLASSEX structure
    static LRESULT CALLBACK stWinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // just so you can change the window caption...

    virtual void SetWndText(const TCHAR* lpszText) {
        _szWindowText = lpszText?lpszText:_T("");
        if(_hWnd) {
            // already exist
            ::SetWindowText(_hWnd, _szWindowText.c_str());
        }
        // else, Text is set by CreateWindow().
    }

    const TCHAR* GetWndText() const { return _szWindowText.c_str(); }

    virtual void Create(Window& Owner, const RECT& rect) {
        Create(&Owner, rect);
    }
    
    virtual bool Create(Window* pOwner, const RECT& rect);

    HWND Handle() const {
        return _hWnd;
    }
    
    bool IsDefined() const {
        return _hWnd != NULL;
    }

    bool IdentifyDescendant(HWND h) const {
        assert(IsDefined());
        return h == _hWnd || ::IsChild(_hWnd, h);
    }

    Window* GetParent() const {
        return GetObjectFromWindow(::GetParent(_hWnd));
    }
    
    void Move(const RECT& rc) {
        ::SetWindowPos(_hWnd, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    }

    void FastMove(const RECT& rc) { 
        Move(rc); 
    }

    void Move(LONG left, LONG top) {
        ::SetWindowPos(_hWnd, NULL, left, top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    }

    void Resize(LONG width, LONG height) {
        ::SetWindowPos(_hWnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    }

    void SetVisible(bool Visible) {
        ::ShowWindow(_hWnd, Visible ? SW_SHOW : SW_HIDE);
    }

    bool IsVisible() {
        // IsWindowVisible also test parent's, we don't want that
        // return ::IsWindowVisible(_hWnd);
        return (GetWindowLongPtr(_hWnd, GWL_STYLE) & WS_VISIBLE);
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
    
    POINT GetPosition() const {
        const RECT& rc = GetClientRect();
        POINT pos = { rc.left, rc.top }; 
        Window* Parent = GetParent();
        if(Parent) {
            ::ClientToScreen(Handle(), &pos);
            ::ScreenToClient(Parent->Handle(), &pos);
        }
        return pos;
    }
    
    unsigned GetWidth() const {
        const RECT& rc = GetClientRect();
        return rc.right - rc.left;
    }

    unsigned GetHeight() const {
        const RECT& rc = GetClientRect();
        return rc.bottom - rc.top;
    }

    int GetTop() const {
        return GetPosition().y;
    }

    int GetLeft() const {
        return GetPosition().x;
    } 
    
    int GetRight() const {
      return GetLeft() + GetWidth();
    }

    int GetBottom() const {
      return GetTop() + GetHeight();
    }    
 
    virtual void Close() {
        ::SendMessage(_hWnd, WM_CLOSE, 0, 0);
    }

    virtual void Destroy() {
        ::DestroyWindow(_hWnd);
        _hWnd = NULL;
    }

    virtual FontReference GetFont() const {
        return _Font;
    }
    
    virtual void SetFont(FontReference Font) {
      _Font = Font;
        ::SendMessage(_hWnd, WM_SETFONT, (WPARAM)(HFONT)_Font, MAKELPARAM(TRUE,0));
    }

    void SetFocus() {
        ::SetFocus(_hWnd);
    }

    bool HasFocus() {
        return (::GetFocus() == _hWnd);
    }

    void Redraw(const RECT& Rect) {
        ::InvalidateRect(_hWnd, &Rect, FALSE);
        if(::GetCurrentThreadId() == ::GetWindowThreadProcessId(_hWnd, NULL)) {
            // ::UpdateWindow() can'be called by another thread, otherwise, we can have deadlock with MsgPump
            ::UpdateWindow(_hWnd);
        }
    }

    void Redraw() {
        ::InvalidateRect(_hWnd, NULL, FALSE);
        if(::GetCurrentThreadId() == ::GetWindowThreadProcessId(_hWnd, NULL)) {
            // ::UpdateWindow() can'be called by another thread, otherwise, we can have deadlock with MsgPump
            ::UpdateWindow(_hWnd);
        }
    }

    void SetTopWnd() {
        ::SetWindowPos(_hWnd, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
    }

    void SetCapture() {
        ::SetCapture(_hWnd);
    }

    void ReleaseCapture() {
        ::ReleaseCapture();
    }

    static Window* GetFocusedWindow() {
        return Window::GetObjectFromWindow(::GetFocus());
    }
    
    virtual void Show() {
        SetVisible(true);
    }

    void ToScreen(POINT& pos) const {
        ::ClientToScreen(_hWnd, &pos);
    }

    void ToClient(POINT& pos) const {
        ::ScreenToClient(_hWnd, &pos);
    }

protected:

    void StartTimer(unsigned uTime /*millisecond*/) { ::SetTimer(_hWnd, 1, uTime, NULL); }
    void StopTimer() { ::KillTimer(_hWnd, 1); }

protected:
    HWND _hWnd;
    DWORD _dwStyles;
    tstring _szClassName;
    tstring _szWindowText;
    tstring _szWindowName;

    FontReference _Font;
            
    WNDPROC _OriginalWndProc;
    
    //contructor
    Window(CONST WNDCLASS* wcx = NULL) : _hWnd(), _dwStyles(), _Font(), _OriginalWndProc() {
        if (wcx != NULL) {
            RegisterWindow(wcx);
        }
    }
    
    virtual ~Window() { }
    
    void SetHandle(HWND hwnd) {
        _hWnd = hwnd;
    }    

    void SubClassWindow();

    // the real message handler
    virtual LRESULT CALLBACK WinMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    // process Message forwarded by child window.
    virtual bool Notify(Window* pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // returns a pointer the window (stored as the WindowLong)
    inline static Window *GetObjectFromWindow(HWND hWnd) {
        return (Window *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
    }

    virtual HBRUSH OnCtlColor(HDC hdc) { return NULL; }

protected:
    // Event Handling virtual function ( return true for ignore default process ) :
    virtual void OnCreate() { }
    virtual bool OnClose() { return false; }
    virtual void OnDestroy() { }
    virtual bool OnSize(int cx, int cy) { return false; }

    virtual bool OnPaint(LKSurface& Surface, const RECT& Rect) { return false; }

    virtual void OnSetFocus() { }
    virtual void OnKillFocus() { }

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

    virtual void OnTimer() { }
    virtual bool OnUser(unsigned id) { return false; }
};

typedef Window ContainerWindow;
#endif
