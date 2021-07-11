/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

RECT WindowResize(unsigned int x, unsigned int y) {

  RECT w;
  w.left=0;
  w.right=x;
  w.top=0;
  w.bottom=y;

  #if TESTBENCH
  StartupStore(_T("..... WindowResize RECT (rltb) %d %d %d %d%s"),w.right, w.left, w.top, w.bottom,NEWLINE);
  #endif

  return w;

};

