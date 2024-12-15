/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndTextEdit.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 18 novembre 2014, 00:08
 */

#include "RGB.h"
#include "WndTextEdit.h"

WndTextEdit::WndTextEdit() : WndText(RGB_BLACK, RGB_WHITE) {
    _dwStyles = WS_CHILD|ES_MULTILINE|ES_CENTER|WS_BORDER|ES_READONLY|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
    _szClassName = _T("EDIT");

}

WndTextEdit::~WndTextEdit() {

}
