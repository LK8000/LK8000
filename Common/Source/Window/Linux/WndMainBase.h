/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndMainBase.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 9 novembre 2014, 14:49
 */

#ifndef WndMainBase_H
#define	WndMainBase_H

#include "WndPaint.h"

class WndMainBase : public WndPaint {
public:
    WndMainBase() { }
    virtual ~WndMainBase() { }

    bool Create(const RECT& rect, const TCHAR* szName) {
        return WndPaint::Create(NULL, rect, szName);
    }

    void FullScreen() {};

};

#endif	/* WndMainBase_H */

