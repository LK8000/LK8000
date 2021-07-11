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

#ifndef _LINUX_WNDCTRLBASE_H
#define	_LINUX_WNDCTRLBASE_H
#include "LKWindow.h"
#include "Screen/ContainerWindow.hpp"

class WndCtrlBase : public LKWindow<ContainerWindow> {
public:
    WndCtrlBase(const TCHAR * szName) {
        _szWndName = szName?szName:_T("");
    }

    virtual void SetWndText(const TCHAR* lpszText) {
        _szWndText = lpszText?lpszText:_T("");
    }

    virtual const TCHAR* GetWndText() const {
        return _szWndText.c_str();
    }

    const TCHAR* GetWndName() const {
        return _szWndName.c_str();
    }

    void SetWndName(const TCHAR * szName) {
        _szWndName = szName?szName:_T("");
    }

    virtual bool OnLButtonDown(const POINT& Pos) { return true; }
    virtual bool OnLButtonUp(const POINT& Pos) { return true; }

protected:
    tstring _szWndName;
    tstring _szWndText;

};

#endif	/* _LINUX_WNDCTRLBASE_H */
