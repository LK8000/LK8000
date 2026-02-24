/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "options.h"
#include "MessageLog.h"
#include "math.h"
#include "Defines.h"
#include "WndMainBase.h"

LRESULT CALLBACK WndMainBase::WinMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {

        case WM_ERASEBKGND:
            return 1; // JMW trying to reduce screen flicker
            break;
        case WM_CREATE:
            break;

        case WM_ACTIVATE:
            if(LOWORD(wParam) == WA_INACTIVE) {
                HWND hWndFocus = ::GetFocus();
                if(hWndFocus && ::IsChild(_hWnd, hWndFocus)) {
                   //  Save Focus if Main Window Loses activation.
                    _hWndFocus = hWndFocus;
                }
            } else {
                SetWindowPos(_hWnd, HWND_TOP,
                        0, 0, 0, 0,
                        SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
#if defined(USE_FULLSCREEN)
                DEVMODE fullscreenSettings;
                EnumDisplaySettings(NULL, 0, &fullscreenSettings);
                fullscreenSettings.dmPelsWidth = GetSystemMetrics(SM_CXSCREEN);
                fullscreenSettings.dmPelsHeight = GetSystemMetrics(SM_CYSCREEN);

                SetWindowLongPtr(_hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
                SetWindowLongPtr(_hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
                SetWindowPos(_hWnd, HWND_TOPMOST, 0, 0, fullscreenSettings.dmPelsWidth, fullscreenSettings.dmPelsHeight, SWP_SHOWWINDOW);
                ChangeDisplaySettings(&fullscreenSettings, CDS_FULLSCREEN);
                ShowWindow(_hWnd, SW_MAXIMIZE);
#endif

            }
            return 0;

        case WM_SETTINGCHANGE:
            return 0;

        case WM_SETFOCUS:
		if(::IsWindow(_hWndFocus) && ::IsChild(_hWnd, _hWndFocus)) {
			// Set the Focus to the last know focus window
			::SetFocus(_hWndFocus);
			return 0;
		}
		_hWndFocus = NULL;
		break; // process default.

        default:
            break;
    }
    return WndPaint::WinMsgHandler(hWnd, uMsg, wParam, lParam);
}
