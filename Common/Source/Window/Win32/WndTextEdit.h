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

#ifndef _WIN32_WNDTEXTEDIT_H
#define	_WIN32_WNDTEXTEDIT_H
#include "WndText.h"

class WndTextEdit : public WndText {
public:
    WndTextEdit();
    virtual ~WndTextEdit();

    int GetLineCount() {
        return (int)::SendMessage(_hWnd, EM_GETLINECOUNT, 0, 0);
    }

};

#endif	/* _WIN32_WNDTEXTEDIT_H */
