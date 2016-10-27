/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"


void SaveDefaultTask(void) {
  LockTaskData();
    TCHAR buffer[MAX_PATH];
  LocalPath(buffer,TEXT(LKD_TASKS));
  _tcscat(buffer,TEXT(DIRSEP));
  _tcscat(buffer,_T(LKF_DEFAULTASK)); // 091101
    SaveTask(buffer);
  UnlockTaskData();
}
