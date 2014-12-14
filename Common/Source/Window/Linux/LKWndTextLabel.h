/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndLabel.h
 * Author: Bruno de Lacheisserie    
 *
 * Created on 20 novembre 2014, 21:50
 */

#ifndef WNDLABEL_H
#define	WNDLABEL_H
#include "LKWndText.h"


template<class _Base>
class LKWndTextLabel : public LKWndText<_Base> {
public:
    LKWndTextLabel() : LKWndText<_Base>(LKColor(0,0,0), LKColor(0xFF, 0xFF, 0xFF)) {
        
    }
    
    virtual bool OnPaint(LKSurface& Surface, const RECT& Rect) {
        Canvas& canvas = Surface;
        
        PixelRect rc(0, 0, canvas.GetWidth() - 1, canvas.GetHeight() - 1);

        canvas.DrawFilledRectangle(rc, this->GetBkColor());
        canvas.DrawOutlineRectangle(rc.left, rc.top, rc.right, rc.bottom, COLOR_BLACK);

        if (this->_Text.empty())
          return true;
/*
        const PixelScalar padding = Layout::GetTextPadding();
        rc.Grow(-padding);
*/
        canvas.SetBackgroundTransparent();
        canvas.SetTextColor(this->_TextColor);

        canvas.DrawFormattedText(&rc, this->_Text.c_str(), this->GetTextStyle());        
        
        return true;
    }
};

#endif	/* WNDLABEL_H */

