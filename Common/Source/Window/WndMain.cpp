/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndMain.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 9 novembre 2014, 16:51
 */

#include "externs.h"
#include "WndMain.h"
#include "LKLanguage.h"
#include "MapWindow.h"
#include "Dialogs.h"

WndMain::WndMain() : WndMainBase(), _MouseButtonDown() {
}

WndMain::~WndMain() {
}

extern void Shutdown();

bool WndMain::OnCreate(int x, int y, int cx, int cy) {
    MapWindow::_OnCreate(*this, cx, cy);
    bool bRet = WndMainBase::OnCreate(x, y, cx, cy);
    StartTimer(500);
    return bRet;
}

bool WndMain::OnClose() {
    if (MessageBoxX(gettext(TEXT("_@M198_")), // LKTOKEN  _@M198_ = "Confirm Exit?"
                    TEXT("LK8000"),
                    mbYesNo) == IdYes) {

        Shutdown();
    }
    return true;
}

bool WndMain::OnDestroy() {
    StopTimer();
    MapWindow::_OnDestroy();
    return WndMainBase::OnDestroy();
}

bool WndMain::OnSize(int cx, int cy) {
    MapWindow::_OnSize(cx, cy);
    return true;
}

extern StartupState_t ProgramStarted;
bool WndMain::OnPaint(LKSurface& Surface, const RECT& Rect) {
    if(ProgramStarted >= psFirstDrawDone) {
        Surface.Copy(Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top, ScreenSurface, Rect.left, Rect.top);
    } else {
        
    }
    return true;
}

bool WndMain::OnKillFocus() { 
    _MouseButtonDown = false;
    return  WndMainBase::OnKillFocus();
}


bool WndMain::OnMouseMove(const POINT& Pos) {
    if(_MouseButtonDown) {
        MapWindow::_OnDragMove(Pos);
    }
    return true;
}

bool WndMain::OnLButtonDown(const POINT& Pos) {
    _MouseButtonDown = true;    
    MapWindow::_OnLButtonDown(Pos);
    return true;
}

bool WndMain::OnLButtonUp(const POINT& Pos) {
    _MouseButtonDown = false;
    MapWindow::_OnLButtonUp(Pos);
    return true;
}

bool WndMain::OnLButtonDblClick(const POINT& Pos) {
    MapWindow::_OnLButtonDblClick(Pos);
    return true;
}

bool WndMain::OnKeyDown(unsigned KeyCode) {
    MapWindow::_OnKeyDown(KeyCode);
    return true;
}

extern void AfterStartup();
extern void StartupLogFreeRamAndStorage();
extern void SIMProcessTimer(void);
extern void ProcessTimer(void);

bool WndMain::OnTimer() {
    // WM_TIMER is run at about 2hz.
    LKHearthBeats++; // 100213
    if (ProgramStarted > psInitInProgress) {
        if (SIMMODE) {
            SIMProcessTimer();
        } else {
            ProcessTimer();
        }
        if (ProgramStarted == psFirstDrawDone) {
            AfterStartup();
            ProgramStarted = psNormalOp;
            StartupStore(_T(". ProgramStarted=NormalOp %s%s"), WhatTimeIsIt(), NEWLINE);
            StartupLogFreeRamAndStorage();
        }
    }
    return true; // we have processes this message;
}

void WndMain::RunModalLoop() {
    while(_EventLoop.Wait()) {
        _EventLoop.Dispatch();
    }
}
