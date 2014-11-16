/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndMainBase.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 9 novembre 2014, 14:49
 */

#include "WndMainBase.h"
#include "resource.h"

extern HINSTANCE _hInstance; // Set by WinMain

WndMainBase::WndMainBase() : Window(NULL), iTimerID()  {

#ifdef HAVE_ACTIVATE_INFO
    if(GetProcAddress(GetModuleHandle(TEXT("AYGSHELL")), TEXT("SHHandleWMActivate"))) {
        api_has_SHHandleWMActivate = true;
    }
    if(GetProcAddress(GetModuleHandle(TEXT("AYGSHELL")), TEXT("SHHandleWMSettingChange"))) {
        api_has_SHHandleWMSettingChange = true;
    }
#endif
}

WndMainBase::~WndMainBase() {
    
}

bool WndMainBase::Create(const RECT& rect) {

    WNDCLASS wc;
    WNDCLASS dc;
    GetClassInfo(_hInstance, TEXT("DIALOG"), &dc);

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = Window::stWinMsgHandler;
    wc.cbClsExtra = 0;
#if (WINDOWSPC>0)
    wc.cbWndExtra = 0;
#else
    wc.cbWndExtra = dc.cbWndExtra;
#endif
    wc.hInstance = _hInstance;
    wc.hIcon = LoadIcon(_hInstance, MAKEINTRESOURCE(IDI_LK8000SWIFT));
    wc.hCursor = 0;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = _T("LK8000_Main");

    // Register the window class.
    _szClassName = wc.lpszClassName;

    RegisterClass(&wc);

    _szWindowTitle = _T("LK8000");

    return Window::Create(WS_SYSMENU|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, rect);
}

bool WndMainBase::OnCreate(int x, int y, int cx, int cy) {
    
#ifdef HAVE_ACTIVATE_INFO
    SHSetAppKeyWndAssoc(VK_APP1, _hWnd);
    SHSetAppKeyWndAssoc(VK_APP2, _hWnd);
    SHSetAppKeyWndAssoc(VK_APP3, _hWnd);
    SHSetAppKeyWndAssoc(VK_APP4, _hWnd);
    SHSetAppKeyWndAssoc(VK_APP5, _hWnd);
    SHSetAppKeyWndAssoc(VK_APP6, _hWnd);
#endif
    
    return Window::OnCreate(x, y, cx, cy);
}

void WndMainBase::FullScreen() {
    SetForegroundWindow(_hWnd);
#ifndef UNDER_CE
    SetWindowPos(_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
#else
#ifndef CECORE
    SHFullScreen(_hWnd, SHFS_HIDETASKBAR | SHFS_HIDESIPBUTTON | SHFS_HIDESTARTICON);
#endif
    SetWindowPos(_hWnd, HWND_TOP,
                 0, 0,
                 GetSystemMetrics(SM_CXSCREEN),
                 GetSystemMetrics(SM_CYSCREEN),
                 SWP_SHOWWINDOW);
#endif
}
