/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "dlgTools.h"
#include "CTaskFileHelper.h"

void RenameIfVirtual(const unsigned int i);

void SaveTask(const TCHAR *szFileName) {
    CTaskFileHelper helper;
    if (helper.Save(szFileName)) {
        TaskModified = false; // task successfully saved
        TargetModified = false;
        _tcscpy(LastTaskFileName, szFileName);
#if TESTBENCH
        StartupStore(_T(".... SaveTask: Ok%s"), NEWLINE);
#endif
    } else {

        MessageBoxX(
                // LKTOKEN  _@M263_ = "Error in saving task!" 
                gettext(TEXT("_@M263_")), gettext(TEXT("Save task")), mbOk);
        
        StartupStore(_T("++++++ SaveTask: ERROR saving task!%s"), NEWLINE);
    }
}


//
// If it is a virtual waypoint, rename it before saving to task
//
void RenameIfVirtual(const unsigned int i) {

  if (i>RESWP_END) return;
  LKASSERT(i<=NumberOfWayPoints);

  if ( _tcslen(WayPointList[i].Name)> (NAME_SIZE-5)) return;

  TCHAR tmp[NAME_SIZE+10];
  _stprintf(tmp,_T("TSK_%s"),WayPointList[i].Name);
  LK_tcsncpy(WayPointList[i].Name,tmp,NAME_SIZE);
  if (WayPointList[i].Latitude==RESWP_INVALIDNUMBER && WayPointList[i].Longitude==RESWP_INVALIDNUMBER) {
	if (WayPointList[RESWP_TAKEOFF].Latitude!=RESWP_INVALIDNUMBER) {
		WayPointList[i].Latitude=WayPointList[RESWP_TAKEOFF].Latitude;
		WayPointList[i].Longitude=WayPointList[RESWP_TAKEOFF].Longitude;
		WayPointList[i].Altitude=WayPointList[RESWP_TAKEOFF].Altitude;
	}
  }
 
} 




