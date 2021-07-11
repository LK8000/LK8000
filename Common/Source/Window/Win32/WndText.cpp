/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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

void WndText::OnCreate() {
    _BkBrush.Create(_BkColor);
    Window::OnCreate();
}

void WndText::OnDestroy() {
    _BkBrush.Release();
    Window::OnDestroy();
}
