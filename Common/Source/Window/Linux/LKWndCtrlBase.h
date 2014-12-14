/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndCtrlBase.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 2 décembre 2014
 */

#ifndef WNDCTRLBASE_H
#define	WNDCTRLBASE_H
#include "LKWindow.h"

template<class _Base>
class LKWndCtrlBase : public LKWindow<_Base> {
public:
    LKWndCtrlBase(const TCHAR * szName) {
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
    
protected:
    std::tstring _szWndName;
    std::tstring _szWndText;
        
};

#endif	/* WNDCTRLBASE_H */

