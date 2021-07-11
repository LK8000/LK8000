/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndMainBase.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 9 novembre 2014, 14:49
 */

#include "WndMainBase.h"
#include "resource.h"
#include "Event/Event.h"
#include "MessageLog.h"

extern HINSTANCE _hInstance; // Set by WinMain

WndMainBase::WndMainBase() : WndPaint(NULL), _hWndFocus()  {

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

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = Window::stWinMsgHandler;
    wc.cbClsExtra = 0;
    wc.hInstance = _hInstance;
    wc.hIcon = LoadIcon(_hInstance, MAKEINTRESOURCE(IDI_LK8000SWIFT));
    wc.hCursor = 0;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = _T("LK8000_Main");

/*
 * http://msdn.microsoft.com/fr-fr/library/windows/apps/xaml/ms908209.aspx
 *
 * Applications create custom dialog box classes by filling a WNDCLASS structure with appropriate information and
 * registering the class with the RegisterClass function. Some applications fill the structure by using the GetClassInfo
 * function, specifying the name of the predefined dialog box. In such cases, the applications modify at least the
 * lpszClassName member before registering. In all cases, the cbWndExtra member of WNDCLASS for a custom
 * dialog box class must be set to at least DLGWINDOWEXTRA.
 */

#ifdef UNDER_CE
    WNDCLASS dc;
    GetClassInfo(_hInstance, TEXT("DIALOG"), &dc);
    wc.cbWndExtra = dc.cbWndExtra;
#else
    wc.cbWndExtra = DLGWINDOWEXTRA;
#endif


    // Register the window class.
    _szClassName = wc.lpszClassName;

    if(!RegisterClass(&wc)) {
        StartupStore(_T("RegisterClass(%s) failed = <0x%x>\n"), wc.lpszClassName, GetLastError());
    }


    _szWindowText = _T("LK8000");
    _dwStyles = WS_SYSMENU|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
    
#ifndef UNDER_CE
    _dwStyles |= WS_SIZEBOX;
#endif

    return WndPaint::Create(NULL, rect);
}

void WndMainBase::OnCreate() {

#ifdef HAVE_ACTIVATE_INFO
    SHSetAppKeyWndAssoc(VK_APP1, _hWnd);
    SHSetAppKeyWndAssoc(VK_APP2, _hWnd);
    SHSetAppKeyWndAssoc(VK_APP3, _hWnd);
    SHSetAppKeyWndAssoc(VK_APP4, _hWnd);
    SHSetAppKeyWndAssoc(VK_APP5, _hWnd);
    SHSetAppKeyWndAssoc(VK_APP6, _hWnd);
#endif

    WndPaint::OnCreate();
}

void WndMainBase::OnDestroy() {
    WndPaint::OnDestroy();
    PostQuitMessage(0);
}

void WndMainBase::Fullscreen() {
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

bool WndMainBase::FilterEvent(const Event &event, Window *allowed) const {
    assert(allowed != nullptr);

    if (event.IsUserInput()) {
        if (allowed->IdentifyDescendant(event.msg.hwnd))
            /* events to the current modal dialog are allowed */
            return true;

        return false;
    } else
        return true;
}

void WndMainBase::RunModalLoop() {
    assert(event_queue);
    EventLoop loop(*event_queue);
    Event event;
    while (IsDefined() && loop.Get(event)) {
        loop.Dispatch(event);
    }
}
