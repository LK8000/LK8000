/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndTextEdit.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 18 novembre 2014, 00:08
 */

#ifndef _LINUX_WNDTEXTEDIT_H
#define	_LINUX_WNDTEXTEDIT_H
#include "Screen/Window.hpp"
#include "WndText.h"

//this this used only as base class for WndMessage cf. Message.cpp

class WndTextEditStyle : public WindowStyle {
public:
    WndTextEditStyle() {
        text_style |= DT_WORDBREAK|DT_CENTER|DT_VCENTER;
        Hide();
    }
};

class WndTextEdit : public WndText<Window> {
    typedef WndText<Window> __super;
public:
    WndTextEdit() : WndText<Window>(LKColor(0,0,0), LKColor(0xFF, 0xFF, 0xFF)) {

    }

    virtual void Create(ContainerWindow* pOwner, const RECT& rect) {
        __super::Create(pOwner, rect, WndTextEditStyle());
    }

    int GetLineCount() {
        int line = 1;
        const TCHAR* p = GetWndText();
        if (p) {
            while (*(p++)) {
                if (*p == _T('\r')) {
                    if (*(p + 1) == _T('\n')) {
                        ++p;
                    }
                    ++line;
                } else if (*p == _T('\n')) {
                    ++line;
                }
            }
        }
        return line;
    }

};

#endif	/* _LINUX_WNDTEXTEDIT_H */
