/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

RECT WindowResize(unsigned int x, unsigned int y) {

  RECT w;

#if defined(UNDER_CE) || defined(USE_FULLSCREEN)
  //
  // For Windows CE we disable frames, so no borders to calculate.
  //
  w.left = 0;
  w.top = 0;
  w.right = x;
  w.bottom = y;
#else
  // 
  // For Windows PC we need to calculate borders
  //
  w.right = x + 2*GetSystemMetrics( SM_CXFIXEDFRAME);
  w.left = (GetSystemMetrics(SM_CXSCREEN) - w.right) / 2;
  w.right = w.right + w.left;
  w.bottom = y + 2*GetSystemMetrics( SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION);
  w.top = (GetSystemMetrics(SM_CYSCREEN) - w.bottom) / 2;
  w.bottom = w.bottom + w.top;
#endif

  #if TESTBENCH
  StartupStore(_T("..... WindowResize RECT (rltb) %d %d %d %d%s"),w.right, w.left, w.top, w.bottom,NEWLINE);
  #endif

  return(w);

}


