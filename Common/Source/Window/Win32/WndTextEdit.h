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
#include "Window.h"
#include "Screen/LKColor.h"
#include "Screen/LKBrush.h"

class WndTextEdit : public Window {
public:
    WndTextEdit();
    virtual ~WndTextEdit();
    
    int GetLineCount() {
        return (int)::SendMessage(_hWnd, EM_GETLINECOUNT, 0, 0);
    }

    void SetTextColor(const LKColor& color) {
        _TextColor = color;
    }

    void SetBkColor(const LKColor& color) {
        _BkColor = color;
        _BkBrush.Create(_BkColor);
    }

protected:
    virtual HBRUSH OnCtlColor(HDC hdc);

private:
    LKColor _TextColor;
    LKColor _BkColor;
    LKBrush _BkBrush;

};

#endif	/* WNDTEXTEDIT_H */

