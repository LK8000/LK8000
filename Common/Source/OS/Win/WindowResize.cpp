/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

RECT WindowResize(unsigned int x, unsigned int y) {

  RECT w = { 0, 0, (LONG)x, (LONG)y };

#if defined(UNDER_CE) || defined(USE_FULLSCREEN)
  //
  // For Windows CE we disable frames, so no borders to calculate.
  //
#else
  //
  // For Windows PC we need to calculate borders
  //
  HWND hwnd = main_window->Handle();
  // if main_window not already exist, use default style.
  DWORD dwStyle = WS_SYSMENU|WS_SIZEBOX|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_VISIBLE|WS_CAPTION;
  DWORD dwExStyle = 0U;
  HMENU menu = NULL;
  if(hwnd) {
    dwStyle = GetWindowLongPtr( hwnd, GWL_STYLE ) ;
    dwExStyle = GetWindowLongPtr( hwnd, GWL_EXSTYLE ) ;
    menu = GetMenu( hwnd ) ;
  }
    
  if(!AdjustWindowRectEx( &w, dwStyle, menu ? TRUE : FALSE, dwExStyle )) {
      StartupStore(_T("AdjustWindowRectEx failed: error <%d>" NEWLINE), (int)GetLastError());
  }
  
  // Center in Screen
  ::OffsetRect(&w, (GetSystemMetrics(SM_CXSCREEN) - (w.right - w.left)) /2,
                    (GetSystemMetrics(SM_CYSCREEN) - (w.bottom - w.top)) /2);
#endif

  #if TESTBENCH
  StartupStore(_T("..... WindowResize RECT (rltb) %d %d %d %d%s"),w.right, w.left, w.top, w.bottom,NEWLINE);
  #endif

  return(w);

}
