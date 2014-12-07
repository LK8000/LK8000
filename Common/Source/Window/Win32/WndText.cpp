/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndText.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 20 novembre 2014, 21:57
 */

#include "WndText.h"

WndText::WndText(const LKColor& TextColor, const LKColor& BkColor) : Window(), _TextColor(TextColor), _BkColor(BkColor)  {

}

WndText::~WndText() {
}

HBRUSH WndText::OnCtlColor(HDC hdc) {
    assert(hdc);
    ::SetBkColor(hdc, _BkColor);
    ::SetTextColor(hdc, _TextColor);
    return _BkBrush;
}

bool WndText::OnCreate(int x, int y, int cx, int cy) {
    _BkBrush.Create(_BkColor);
    return OnCreate(x, y, cx, cy);
}

