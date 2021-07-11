/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndText.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 20 novembre 2014, 21:57
 */

#ifndef WNDTEXT_H
#define	WNDTEXT_H

#include "WndPaint.h"
#include "Screen/LKColor.h"
#include "Screen/LKBrush.h"
#include "Screen/BufferCanvas.hpp"

template<class _Base>
class WndText : public WndPaint<_Base> {
public:
    WndText(const LKColor& TextColor, const LKColor& BkColor)
        : _TextColor(TextColor)
        , _BkColor(BkColor)
        , _BkBrush(BkColor)
#ifdef ENABLE_OPENGL
        , _Dirty(true)
#endif
    {

    }

#ifdef ENABLE_OPENGL
    virtual void OnCreate() {
        WndPaint<_Base>::OnCreate();
        _canvas.Create(this->GetSize());
    }

    virtual void OnDestroy() {
        WndPaint<_Base>::OnDestroy();
        _canvas.Destroy();
    }

    virtual void OnResize(PixelSize new_size) {
        WndPaint<_Base>::OnResize(new_size);
        _Dirty= true;
    }

    virtual void Invalidate() {
        _Dirty = true;
        WndPaint<_Base>::Invalidate();
    }

#endif


    void SetTextColor(const LKColor& color) {
        _TextColor = color;
        this->Invalidate();
    }

    void SetBkColor(const LKColor& color) {
        _BkColor = color;
        _BkBrush.Create(_BkColor);
        this->Invalidate();
    }

    const LKColor& GetBkColor() const {
        return _BkColor;
    }

    virtual void SetWndText(const TCHAR* lpszText) {
        _Text = lpszText?lpszText:_T("");
        this->Invalidate();
    }

    virtual const TCHAR* GetWndText() const {
        return _Text.c_str();
    }

    virtual bool OnPaint(LKSurface& Surface, const RECT& Rect) {

#ifdef ENABLE_OPENGL
        if (_Dirty || !_canvas.IsDefined()) {
            _canvas.Begin(Surface);
            this->Setup(_canvas);
#else
            Canvas& _canvas = Surface;
#endif

            PixelRect rc(0, 0, _canvas.GetWidth(), _canvas.GetHeight());

            _canvas.DrawFilledRectangle(rc, this->GetBkColor());
            _canvas.DrawOutlineRectangle(rc.left, rc.top, rc.right, rc.bottom, COLOR_BLACK);

            if (!this->_Text.empty()) {

        //        const PixelScalar padding = Layout::GetTextPadding();
        //        rc.Grow(-1);

                _canvas.SetBackgroundTransparent();
                _canvas.SetTextColor(this->_TextColor);

                _canvas.DrawFormattedText(&rc, this->_Text.c_str(), this->GetTextStyle());
            }

#ifdef ENABLE_OPENGL
            _canvas.Commit(Surface);
            _Dirty = false;
        } else {
            _canvas.CopyTo(Surface);
        }
#endif

        return true;
    }

protected:
    LKColor _TextColor;
    LKColor _BkColor;
    LKBrush _BkBrush;

    tstring _Text;

#ifdef ENABLE_OPENGL
    BufferCanvas _canvas;
    bool _Dirty;
#endif
};

#endif	/* WNDTEXT_H */
