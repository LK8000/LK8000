/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
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

template<class _Base>
class WndText : public WndPaint<_Base> {
public:
    WndText(const LKColor& TextColor, const LKColor& BkColor) : _TextColor(TextColor), _BkColor(BkColor), _BkBrush(BkColor) {
        
    }

    void SetTextColor(const LKColor& color) {
        _TextColor = color;
    }

    void SetBkColor(const LKColor& color) {
        _BkColor = color;
        _BkBrush.Create(_BkColor);
    }
    
    const LKColor& GetBkColor() const {
        return _BkColor;
    }
    
    virtual void SetWndText(const TCHAR* lpszText) {
        _Text = lpszText?lpszText:_T("");
    }

    virtual const TCHAR* GetWndText() const {
        return _Text.c_str();
    }

    virtual bool OnPaint(LKSurface& Surface, const RECT& Rect) {
        Canvas& canvas = Surface;
        
        PixelRect rc(0, 0, canvas.GetWidth() - 1, canvas.GetHeight() - 1);

        canvas.DrawFilledRectangle(rc, this->GetBkColor());
        canvas.DrawOutlineRectangle(rc.left, rc.top, rc.right, rc.bottom, COLOR_BLACK);

        if (this->_Text.empty())
          return true;

//        const PixelScalar padding = Layout::GetTextPadding();
//        rc.Grow(-1);

        canvas.SetBackgroundTransparent();
        canvas.SetTextColor(this->_TextColor);

        canvas.DrawFormattedText(&rc, this->_Text.c_str(), this->GetTextStyle());        
        
        return true;
    }    

protected:
    LKColor _TextColor;
    LKColor _BkColor;
    LKBrush _BkBrush;

    std::tstring _Text;
};

#endif	/* WNDTEXT_H */
