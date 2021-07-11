/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "CalcTask.h"


void RefreshTaskStatistics(void) {
  LockTaskData();
  TaskStatistics(&GPS_INFO, &CALCULATED_INFO, MACCREADY);
  AATStats(&GPS_INFO, &CALCULATED_INFO);
  TaskSpeed(&GPS_INFO, &CALCULATED_INFO, MACCREADY);
  UnlockTaskData();
}


