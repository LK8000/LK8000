/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LocalPath.h"


void SaveDefaultTask(void) {
  LockTaskData();
  TCHAR buffer[MAX_PATH];
  LocalPath(buffer, _T(LKD_TASKS), _T(LKF_DEFAULTASK));
  SaveTask(buffer);
  UnlockTaskData();
}
