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
    bool Wait() { return false; }
    void Dispatch() { }
    
    bool IsKeyboardMsg() const { return false; }
    bool IsMouseMsg() const { return false; }
    bool IsInputMsg() const { return false; }
    
    bool IsEscapeKey() const { return false; }
    
    bool IsChildMsg(Window* pWnd) const { return false; }
    
    const Window* Target() const { return nullptr; }
    
};


#endif	/* EVENTLOOP_H */

