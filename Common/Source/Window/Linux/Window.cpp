/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Window.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 7 december 2014
 */

#include <list>

#include "Window.h"

Window* Window::_FocusedWindow = nullptr;

Window::Window() : _bVisible(false), _bEnabled(true), 
        _Position(), _ClientRect(), 
        _Font(), _Owner(), _bDefined(), _Capture()
{

}

bool Window::Create(Window* pOwner, const RECT& rect, const TCHAR* szName) {
    _Position = (POINT){rect.left, rect.top};
    _ClientRect = (RECT){0, 0, rect.right - rect.left, rect.bottom - rect.top};
    _szWindowName = szName;
    
    _Owner = pOwner;
    if(_Owner) {
        _Owner->AddChild(this);
    }
    
    _bDefined = true;
    
    return true;
}

void Window::Move(const RECT& rect, bool bRepaint) {
    _Position = (POINT){rect.left, rect.top};
    _ClientRect = (RECT){0, 0, rect.right - rect.left, rect.bottom - rect.top};
    if(bRepaint) {
        Redraw();
    }
}
    
void Window::Destroy() {
    if(_Owner) {
        _Owner->RemoveChild(this);
    }
    _bDefined = false;
}

void Window::Close() {

}

void Window::Redraw(const RECT& Rect) {

}

void Window::StartTimer(unsigned uTime /*millisecond*/) {

}

void Window::StopTimer() {

}

void Window::SetToForeground() {

}

void Window::SetTopWnd() {

}

void Window::AddChild(Window* pWnd) {
    
    assert(pWnd && pWnd->GetOwner() == this);
    assert(std::find(_lstChild.begin(), _lstChild.end(), pWnd) == _lstChild.end());
    
    _lstChild.push_back(pWnd);
}

void Window::RemoveChild(Window* pWnd) {
    auto It = std::find(_lstChild.begin(), _lstChild.end(), pWnd);
    if(It != _lstChild.end()) {
        _lstChild.erase(It);
    }
}

bool Window::OnUser(unsigned id) {
    assert(false);
    return false;
}

Window* Window::EventChildAt(PixelScalar x, PixelScalar y) const {
    return nullptr;
}

bool Window::OnKeyCheck(unsigned key_code) const {
    return nullptr;
}
