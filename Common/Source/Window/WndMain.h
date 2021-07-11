/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndMain.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 9 novembre 2014, 16:51
 */

#ifndef WNDMAIN_H
#define	WNDMAIN_H

#include "WndMainBase.h"
#include "MapWindow.h"

class WndMain : public WndMainBase, public MapWindow {
public:
    WndMain();
    virtual ~WndMain();

protected:
    virtual void OnCreate();
    virtual bool OnClose();
    virtual void OnDestroy();

    virtual bool OnSize(int cx, int cy);

    virtual bool OnPaint(LKSurface& Surface, const RECT& Rect);

    virtual void OnKillFocus();

    virtual bool OnMouseMove(const POINT& Pos);

    virtual bool OnLButtonDown(const POINT& Pos);
    virtual bool OnLButtonUp(const POINT& Pos);

    virtual bool OnLButtonDblClick(const POINT& Pos);

    virtual bool OnKeyDown(unsigned KeyCode);

    virtual void OnTimer();

private:
    bool _MouseButtonDown;
    bool _isRunning;
};

#endif	/* WNDMAIN_H */
