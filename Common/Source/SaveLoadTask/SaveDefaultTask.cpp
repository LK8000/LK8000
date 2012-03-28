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
//#if (!defined(WINDOWSPC) || (WINDOWSPC <=0) )
#if 1
  LocalPath(buffer,TEXT(LKD_TASKS));
  _tcscat(buffer,TEXT("\\"));
  _tcscat(buffer,_T(LKF_DEFAULTASK)); // 091101
#else // REMOVE 
  SHGetSpecialFolderPath(hWndMainWindow, buffer, CSIDL_PERSONAL, false); // REMOVABLE
  _tcscat(buffer,TEXT("\\"));
  _tcscat(buffer,TEXT(LKDATADIR));
  _tcscat(buffer,_T("\\"));
  _tcscat(buffer,TEXT(LKD_TASKS)); // 091101
  _tcscat(buffer,_T("\\"));
  _tcscat(buffer,_T(LKF_DEFAULTASK)); // 091101
#endif
    SaveTask(buffer);
  UnlockTaskData();
}
