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

#include "LKWndPaint.h"
#include "Screen/LKColor.h"
#include "Screen/LKBrush.h"

template<class _Base>
class LKWndText : public LKWndPaint<_Base> {
public:
    LKWndText(const LKColor& TextColor, const LKColor& BkColor) : _TextColor(TextColor), _BkColor(BkColor), _BkBrush(BkColor) {
        
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

protected:
    LKColor _TextColor;
    LKColor _BkColor;
    LKBrush _BkBrush;

    std::tstring _Text;
};

#endif	/* WNDTEXT_H */
