/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKWindow.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 9 novembre 2014, 14:49
 */

#ifndef _LINUX_WINDOW_H_
#define _LINUX_WINDOW_H_

#include <assert.h>
#include <tchar.h>
#include <list>
#include "utils/stl_utils.h"
#include "Util/tstring.hpp"
#include "Compiler.h"
#include "Screen/FontReference.h"
#include "Screen/LKFont.h"
#include "ScreenCoordinate.h"
#include "Screen/Point.hpp"
#include "Event/Timer.hpp"

#include "Screen/LKSurface.h"

template<class _Base>
class LKWindow : public _Base,
                 public Timer
{
public:
    LKWindow() = default;
    ~LKWindow() {
        StopTimer();
    }

    virtual void SetWndText(const TCHAR* lpszText) = 0;
    virtual const TCHAR* GetWndText() const = 0;

    void SetVisible(bool Visible) {
        if(Visible) {
            this->Show();
        } else {
            this->Hide();
        }
    }

    void Enable(bool Enable) {
        this->SetEnabled(Enable);
    }

    virtual FontReference GetFont() const {
        return static_cast<FontReference>(&(_Base::GetFont()));
    }

    virtual void SetFont(FontReference Font) {
        assert(Font);
        if(Font && Font->IsDefined()) {
            _Base::SetFont(*Font);
        }
    }

    virtual void Redraw(const RECT& Rect) {
        this->Invalidate();
    }

    virtual void Redraw() {
        this->Invalidate();
    }

    void SetToForeground() {
        if(this->GetParent()) {
            this->BringToTop();
        }
    }
    void SetTopWnd() {
        if(this->GetParent()) {
            this->BringToTop();
        }
    }

    void StartTimer(unsigned uTime /*millisecond*/) {
        Schedule(uTime);
    }

    void StopTimer() {
        Cancel();
    }

    void AddChild(Window* pWnd);
    void RemoveChild(Window* pWnd);

    virtual void OnTimer() {}

    virtual bool OnTimer(WindowTimer &timer) {
        return _Base::OnTimer(timer);
    }

    virtual void OnPaint(Canvas &canvas) {
        LKSurface Surface;
        Surface.Attach(&canvas);
        OnPaint(Surface, this->GetClientRect());
        _Base::OnPaint(canvas);
    }

    virtual void OnPaint(Canvas &canvas, const PixelRect &dirty) {
        LKSurface Surface;
        Surface.Attach(&canvas);
        OnPaint(Surface, dirty);
        _Base::OnPaint(canvas);
    }

    virtual bool OnMouseMove(PixelScalar x, PixelScalar y, unsigned keys) {
        bool bRet = _Base::OnMouseMove(x,y,keys);
        if(!bRet) {
            bRet = OnMouseMove((POINT){x,y});
        }
        return bRet;
    }

    virtual bool OnMouseDown(PixelScalar x, PixelScalar y) {
        bool bRet = _Base::OnMouseDown(x,y);
        if(!bRet) {
            bRet = OnLButtonDown((POINT){x,y});
        }
        return bRet;
    }

    virtual bool OnMouseUp(PixelScalar x, PixelScalar y) {
        bool bRet = _Base::OnMouseUp(x,y);
        if(!bRet) {
            bRet = OnLButtonUp((POINT){x,y});
        }
        return bRet;
    }

    virtual bool OnMouseDouble(PixelScalar x, PixelScalar y) {
        bool bRet = _Base::OnMouseDouble(x,y);
        if(!bRet) {
            bRet = OnLButtonDblClick((POINT){x,y});
        }
        return bRet;
    }
    virtual void OnResize(PixelSize new_size) {
        _Base::OnResize(new_size);
        OnSize(new_size.cx, new_size.cy);
    }

    virtual bool OnMouseMove(const POINT& Pos) { return false; }
    virtual bool OnLButtonDown(const POINT& Pos) { return false; }
    virtual bool OnLButtonUp(const POINT& Pos) { return false; }

    virtual bool OnLButtonDblClick(const POINT& Pos) { return false; }

    virtual bool OnSize(int cx, int cy) { return false; }

    virtual bool OnClose() { return false; }
protected:
    virtual bool OnPaint(LKSurface& Surface, const RECT& Rect) { return false; }

private:
    tstring _szWndName;
};

#endif // _LINUX_WINDOW_H_
