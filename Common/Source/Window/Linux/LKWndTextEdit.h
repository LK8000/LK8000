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
#include "LKWndText.h"

template<class _Base>
class LKWndTextEdit : public LKWndText<_Base> {
public:
    LKWndTextEdit() : LKWndText<_Base>(LKColor(0,0,0), LKColor(0xFF, 0xFF, 0xFF)) {
        
    }
    
    int GetLineCount() {
        return 1;
    }

    virtual bool OnPaint(LKSurface& Surface, const RECT& Rect) {
        return true;
    }
};

#endif	/* WNDTEXTEDIT_H */

