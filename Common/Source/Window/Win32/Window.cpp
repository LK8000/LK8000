/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Window.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 9 novembre 2014, 14:49
 */

/*
 * References :
 *  Steve Hanov, A Lightweight Windows Wrapper, C/C++ Users Journal, Aug 2000 pg.26
*/

#include "Window.h"
#include "Screen/LKWindowSurface.h"

extern HINSTANCE _hInstance; // Set by WinMain

BOOL Window::RegisterWindow()
{
    WNDCLASS wcx;

    // Fill in the window class structure with default parameters

    wcx.style = CS_HREDRAW | CS_VREDRAW;						// redraw if size changes
    wcx.lpfnWndProc = Window::stWinMsgHandler;				// points to window procedure
    wcx.cbClsExtra = 0;											// no extra class memory
    wcx.cbWndExtra = 0;											// no extra window memory
    wcx.hInstance = _hInstance;									// handle to instance
    wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);				// predefined app. icon
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);					// predefined arrow
    wcx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	// white background brush
    wcx.lpszMenuName = NULL;									// name of menu resource
    wcx.lpszClassName = _T("BaseWindow");						// name of window class

    // Register the window class.
    return RegisterWindow(&wcx);

}

BOOL Window::RegisterWindow(UINT style, HICON hIcon, HCURSOR hCursor, HBRUSH hbrBackground,
									LPCTSTR lpszMenuName, LPCTSTR lpszClassName)
{
    WNDCLASS wcx;

    // Fill in the window class structure with default parameters

    wcx.style = style;								// redraw if size changes
    wcx.lpfnWndProc = Window::stWinMsgHandler;	// points to window procedure
    wcx.cbClsExtra = 0;								// no extra class memory
    wcx.cbWndExtra = 0;								// no extra window memory
    wcx.hInstance = _hInstance;						// handle to instance
    wcx.hIcon = hIcon;								// predefined app. icon
    wcx.hCursor = hCursor;							// predefined arrow
    wcx.hbrBackground = hbrBackground;				// white background brush
    wcx.lpszMenuName = lpszMenuName;				// name of menu resource
    wcx.lpszClassName = lpszClassName;				// name of window class

    // Register the window class.
    return RegisterWindow(&wcx);
}

BOOL Window::RegisterWindow(const WNDCLASS* wcx)
{
	// Register the window class.
	_szClassName  = wcx->lpszClassName;

	if (RegisterClass(wcx) == 0)
		return FALSE;
	else
		return TRUE;
}

void Window::SubClassWindow() {
  SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)this);
  _OriginalWndProc = (WNDPROC)SetWindowLongPtr(_hWnd, GWLP_WNDPROC, (LONG_PTR) Window::stWinMsgHandler);
}

/*
	You can not initialize the window class with a class method as the window
	procedure unless it is a static method, so the class also needs a static
	message handler that determines which instance of the class is the recipient
	of the message and calls that instance's window procedure.

	See "http://www.gamedev.net/reference/articles/article1810.asp" for more info.
*/
LRESULT CALLBACK Window::stWinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Window* pWnd = NULL;
/*
 * A difference between a desktop version of Windows and the CE version is in the window creation process.
 * On the desktop WM_NCCREATE is the first message so the this pointer should be captured during the its processing.
 * In Windows CE this message doesn't exist. The first message is WM_CREATE so it must be used.
 */
#ifdef UNDER_CE
	if (uMsg == WM_CREATE)
#else
	if (uMsg == WM_NCCREATE)
#endif
	{
		// get the pointer to the window from lpCreateParams which was set in CreateWindow
        pWnd = reinterpret_cast<Window *>(((LPCREATESTRUCT)lParam)->lpCreateParams);
        ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));

        pWnd->SetHandle(hwnd); // this is need, otherwise _hWnd not already set inside OnCreate() ..
	} else {
	// get the pointer to the window
        pWnd = GetObjectFromWindow(hwnd);
    }

	// if we have the pointer, go to the message handler of the window
	// else, use DefWindowProc
	if (pWnd)
		return pWnd->WinMsgHandler(hwnd, uMsg, wParam, lParam);
	else
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool Window::Create(Window* pOwner, const RECT& rect) {
#ifndef NDEBUG
    if(pOwner) {
        assert(::GetCurrentThreadId() == ::GetWindowThreadProcessId(pOwner->Handle(), NULL));
    }
#endif

    // Create the window
    // send the this pointer as the window creation parameter
    HWND hwnd = CreateWindow(_szClassName.c_str(), _szWindowText.c_str(), _dwStyles, rect.left, rect.top,
            rect.right - rect.left, rect.bottom - rect.top, pOwner?pOwner->Handle():NULL, NULL, _hInstance,
            (void *)this);

    assert(hwnd);

    if(hwnd && !_hWnd) {
        // if CreateWindow return valid HWND, this it's not custom window class, so, we need to set WNDPROC
        _hWnd = hwnd;
        SubClassWindow();
    }

    if(_hWnd) {
        OnCreate();
    } else {
        StartupStore(_T("Window::Create error <0x%x>\n"), GetLastError());
    }

    return (_hWnd != NULL);
}

LRESULT CALLBACK Window::WinMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:
            if (OnClose()) return 0;
            break;
        case WM_DESTROY:
            OnDestroy();
            break;
        case WM_SIZE:
            if(OnSize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))) return 0;
            break;
        case WM_SYSKEYDOWN:
            /*
             * http://msdn.microsoft.com/en-us/library/windows/desktop/ms646286(v=vs.85).aspx
             * Bit 29 of lParam :
             * The context code. The value is 1 if the ALT key is down while the key is pressed; it is 0 if the
             * WM_SYSKEYDOWN message is posted to the active window because no window has the keyboard focus.
             */
            if(!((lParam>>29)&0x01)) {
                if(OnKeyDown(wParam)) return 0;
            }
            break;
        case WM_KEYDOWN:
            if(OnKeyDown(wParam)) return 0;
            break;
        case WM_KEYUP:
            if(OnKeyUp(wParam)) return 0;
            break;
        case WM_LBUTTONDBLCLK:
            if(OnLButtonDblClick((POINT){GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)})) return 0;
            break;
        case WM_MOUSEMOVE:
            if(OnMouseMove((POINT){GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)})) return 0;
            break;
        case WM_LBUTTONDOWN:
            if(OnLButtonDown((POINT){GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)})) return 0;
            break;
        case WM_LBUTTONUP:
            if(OnLButtonUp((POINT){GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)})) return 0;
            break;
        case WM_SETFOCUS:
            OnSetFocus();
            break;
        case WM_KILLFOCUS:
            OnKillFocus();
            break;
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT:
            if(lParam) {
                Window * pChildWnd = Window::GetObjectFromWindow((HWND)lParam);
                if(pChildWnd) {
                    HBRUSH hBrush = pChildWnd->OnCtlColor((HDC)wParam);
                    if(hBrush) {
                        return (LRESULT)hBrush;
                    }
                }
            }
            break;
        case WM_TIMER:
            OnTimer();
            break;
        default:
            break;
    }


    if (uMsg >= WM_USER && uMsg <= 0x7FFF && OnUser(uMsg - WM_USER)) {
        return 0;
    }

    Window * pOwner = GetParent();
    if(pOwner && pOwner->Notify(this, uMsg, wParam, lParam)) {
        return 0;
    }

    if(_OriginalWndProc) {
        return CallWindowProc(_OriginalWndProc, hWnd, uMsg, wParam, lParam);
    }
    return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool Window::Notify(Window* pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_KEYDOWN:
            if(OnKeyDownNotify(pWnd, wParam)) return true;
            break;
        case WM_KEYUP:
            if(OnKeyUpNotify(pWnd, wParam)) return true;
            break;
        case WM_LBUTTONDOWN:
            if(OnLButtonDownNotify(pWnd, (POINT){GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)})) return true;
            break;
        case WM_LBUTTONUP:
            if(OnLButtonUpNotify(pWnd, (POINT){GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)})) return true;
            break;
        default:
            break;
    }
    Window * pOwner = GetParent();
    if(pOwner && pOwner->Notify(pWnd, uMsg, wParam, lParam)) {
        return true;
    }
    return false;
}
