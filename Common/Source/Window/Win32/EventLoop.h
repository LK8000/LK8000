/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   EventLoop.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 4 décembre 2014
 */

#ifndef EVENTLOOP_H
#define	EVENTLOOP_H

#include "Window.h"


class EventLoop {
public:
    EventLoop() {}
    ~EventLoop() {}
    
public:
    inline bool Wait();
    inline void Dispatch();
    
    inline bool IsKeyboardMsg() const;
    inline bool IsMouseMsg() const;
    inline bool IsInputMsg() const;
    
    inline bool IsEscapeKey() const;
    
    inline bool IsChildMsg(Window* pWnd) const;
    
    inline const Window* Target() const;
    
private:
    MSG _msg;
};

inline bool EventLoop::IsInputMsg() const {
    return (IsKeyboardMsg() || IsMouseMsg()); 
}

inline bool EventLoop::IsMouseMsg() const {
    return (_msg.message >= WM_MOUSEFIRST && _msg.message <= WM_MOUSELAST);
}

inline bool EventLoop::IsKeyboardMsg() const {
    return (_msg.message >= WM_KEYFIRST && _msg.message <= WM_KEYLAST);
}

inline bool EventLoop::IsEscapeKey() const {
    return (_msg.message == WM_KEYDOWN && (_msg.wParam & 0xffff) == VK_ESCAPE);
}

inline bool EventLoop::IsChildMsg(Window* pWnd) const {
    return pWnd ? ::IsChild(pWnd->Handle(), _msg.hwnd) : false;
}

inline const Window* EventLoop::Target() const {
    return Window::GetObjectFromWindow(_msg.hwnd);
}

inline bool EventLoop::Wait() {
    return (::GetMessage (&_msg, (HWND)NULL, 0, 0) > 0);
}

inline void EventLoop::Dispatch() {
    ::TranslateMessage(&_msg);
    ::DispatchMessage(&_msg);
}

#endif	/* EVENTLOOP_H */

