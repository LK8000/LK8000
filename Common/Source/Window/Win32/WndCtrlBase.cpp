/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndCtrlBase.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 2 décembre 2014
 */

#include "WndCtrlBase.h"

WndCtrlBase::WndCtrlBase(const TCHAR* szName) : _szWindowName(szName?szName:_T("")) {

    static BOOL bRegister = FALSE;
    if(!bRegister) {
        bRegister = RegisterWindow(CS_SAVEBITS|CS_VREDRAW|CS_HREDRAW|CS_DBLCLKS, NULL, NULL, NULL, NULL, _T("LKWndCtrl"));
    } else {
        _szClassName = _T("LKWndCtrl");
    }
    _dwStyles = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
}

WndCtrlBase::~WndCtrlBase() {
}
