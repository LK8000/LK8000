/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndLabel.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 20 novembre 2014, 21:50
 */

#include "WndTextLabel.h"

WndTextLabel::WndTextLabel() : WndText(LKColor(0,0,0), LKColor(0xFF, 0xFF, 0xFF))  {
    _dwStyles = WS_CHILD|WS_TABSTOP|SS_CENTER|SS_NOTIFY|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_BORDER;
    _szClassName = _T("STATIC");

}

WndTextLabel::~WndTextLabel() {

}
