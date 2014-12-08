/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndTextEdit.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 18 novembre 2014, 00:08
 */

#ifndef WNDTEXTEDIT_H
#define	WNDTEXTEDIT_H
#include "WndText.h"

class WndTextEdit : public WndText {
public:
    WndTextEdit() : WndText(LKColor(0,0,0), LKColor(0xFF, 0xFF, 0xFF)) {}
    virtual ~WndTextEdit() {}
    
    int GetLineCount() { return 0; }

};

#endif	/* WNDTEXTEDIT_H */

