/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndCtrlBase.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 2 décembre 2014
 */

#ifndef _WIN32_WNDCTRLBASE_H
#define	_WIN32_WNDCTRLBASE_H
#include "WndPaint.h"

class WndCtrlBase :public WndPaint {
public:
    WndCtrlBase(const TCHAR* szName);
    ~WndCtrlBase();

    const TCHAR* GetWndName() const {
        return _szWindowName.c_str();
    }

protected:
    tstring _szWindowName;
};

#endif	/* _WIN32_WNDCTRLBASE_H */
