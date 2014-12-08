/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndLabel.h
 * Author: Bruno de Lacheisserie    
 *
 * Created on 20 novembre 2014, 21:50
 */

#ifndef WNDLABEL_H
#define	WNDLABEL_H
#include "WndText.h"


class WndTextLabel : public WndText {
public:
    WndTextLabel(): WndText(LKColor(0,0,0), LKColor(0xFF, 0xFF, 0xFF)) {};
    virtual ~WndTextLabel() {};

protected:

};

#endif	/* WNDLABEL_H */

