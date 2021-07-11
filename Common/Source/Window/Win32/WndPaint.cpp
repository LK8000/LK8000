/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndPaint.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 17 novembre 2014, 23:21
 */

#include "WndPaint.h"
#include "Screen/LKWindowSurface.h"

LRESULT CALLBACK WndPaint::WinMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT:
            if(hWnd) {
                LKPaintSurface Surface(hWnd);
                if(OnPaint(Surface, Surface.GetRect())) return 0;
            }
            break;
        case WM_ERASEBKGND:
            return 1;
        default:
            break;
    }
    return Window::WinMsgHandler(hWnd, uMsg, wParam, lParam);
}
