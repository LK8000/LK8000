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
    bool Wait();
    void Dispatch();
    
    bool IsKeyboardMsg() const;
    bool IsMouseMsg() const;
    bool IsInputMsg() const;
    
    bool IsEscapeKey() const;
    
    bool IsChildMsg(Window* pWnd) const;
    
    const Window* Target() const;
    
};


#endif	/* EVENTLOOP_H */

