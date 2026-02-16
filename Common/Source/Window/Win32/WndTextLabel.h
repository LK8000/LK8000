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

#ifndef _WIN32_WNDTEXTLABEL_H
#define	_WIN32_WNDTEXTLABEL_H
#include "WndText.h"

//this this used only as base class for MenuButton cf. Buttons.cpp

class WndTextLabel : public WndText {
public:
    WndTextLabel();
    virtual ~WndTextLabel();

    void SetDrawAsButton(bool /*v*/) { /* no-op on Win32 */ }

protected:

};

#endif	/* _WIN32_WNDTEXTLABEL_H */

