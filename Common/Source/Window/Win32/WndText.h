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

#include "Window.h"
#include "Screen/LKColor.h"
#include "Screen/LKBrush.h"

class WndText : public Window {
public:
    WndText(const LKColor& TextColor, const LKColor& BkColor);
    virtual ~WndText();

    void SetTextColor(const LKColor& color) {
        _TextColor = color;
    }

    void SetBkColor(const LKColor& color) {
        _BkColor = color;
        _BkBrush.Create(_BkColor);
    }

protected:
    virtual HBRUSH OnCtlColor(HDC hdc);
    virtual void OnCreate();
		virtual void OnDestroy();


private:
    LKColor _TextColor;
    LKColor _BkColor;
    LKBrush _BkBrush;

};

#endif	/* WNDTEXT_H */

