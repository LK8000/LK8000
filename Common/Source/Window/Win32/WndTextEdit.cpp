/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndTextEdit.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 18 novembre 2014, 00:08
 */

#include "WndTextEdit.h"

WndTextEdit::WndTextEdit() : Window(), _TextColor(0,0,0), _BkColor(0xFF, 0xFF, 0xFF)  {
    _dwStyles = WS_CHILD|ES_MULTILINE|ES_CENTER|WS_BORDER|ES_READONLY|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
    _szClassName = _T("EDIT");
    
    _BkBrush.Create(_BkColor);
}

WndTextEdit::~WndTextEdit() {
}

HBRUSH WndTextEdit::OnCtlColor(HDC hdc) {
    assert(hdc);
    ::SetBkColor(hdc, _BkColor);
    ::SetTextColor(hdc, _TextColor);
    return _BkBrush;
}
