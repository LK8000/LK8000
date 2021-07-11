/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndLabel.h
 * Author: Bruno de Lacheisserie    
 *
 * Created on 20 novembre 2014, 21:50
 */

#ifndef WNDLABEL_H
#define	WNDLABEL_H
#include "Screen/Window.hpp"
#include "WndText.h"

//this this used only as base class for MenuButton cf. Buttons.cpp

class WndTextLabelStyle : public WindowStyle {
public:
    WndTextLabelStyle() {
        text_style |= DT_WORDBREAK|DT_CENTER|DT_VCENTER;
        Disable();
        Hide();
    }
};

class WndTextLabel : public WndText<Window> {
    typedef WndText<Window> __super;
public:
    WndTextLabel() : __super(LKColor(0,0,0), LKColor(0xFF, 0xFF, 0xFF)) {

    }

    virtual void Create(ContainerWindow* pOwner, const RECT& rect) {
        __super::Create(pOwner, rect, WndTextLabelStyle());
    }

};

#endif	/* WNDLABEL_H */

