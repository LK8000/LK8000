/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "utils/openzip.h"

int globalFileNum = 0;

void InitVirtualWaypoints();
void InitWayPointCalc(void);

void ReadWayPoints(void)
{
  #if TESTBENCH
  StartupStore(TEXT(". ReadWayPoints%s"),NEWLINE);
  #endif

  TCHAR szFile1[MAX_PATH] = TEXT("\0");
  TCHAR szFile2[MAX_PATH] = TEXT("\0");

  ZZIP_FILE *fp=NULL;

    LockTaskData();
    CloseWayPoints(); // BUGFIX 091104 duplicate waypoints entries
    InitVirtualWaypoints();	// 091103

    _tcscpy(szFile1,szWaypointFile);

    _tcscpy(szWaypointFile,_T(""));

    if (_tcslen(szFile1)>0) {
      ExpandLocalPath(szFile1);
      fp = openzip(szFile1, "rt");
    } else {
    }

    if(fp != NULL)
      {
        globalFileNum = 0;
        WpFileType[1]=ReadWayPointFile(fp, szFile1);
        zzip_fclose(fp);
        fp = 0;
        // read OK, so set the registry to the actual file name
        ContractLocalPath(szFile1);
	_tcscpy(szWaypointFile,szFile1);
      } else {
      StartupStore(TEXT("--- No waypoint file 1%s"),NEWLINE);
    }

  // read additional waypoint file

    // reset to empty until we verified it is existing
    _tcscpy(szFile2,szAdditionalWaypointFile);
    _tcscpy(szAdditionalWaypointFile,_T(""));

    if (_tcslen(szFile2)>0){
      ExpandLocalPath(szFile2);
      fp = openzip(szFile2, "rt");
      if(fp != NULL){
        globalFileNum = 1;
        WpFileType[2]=ReadWayPointFile(fp, szFile2);
        zzip_fclose(fp);
        fp = NULL;
        // read OK, so set the registry to the actual file name
        ContractLocalPath(szFile2);
	_tcscpy(szAdditionalWaypointFile,szFile2);
      } else {
        StartupStore(TEXT("--- No waypoint file 2%s"),NEWLINE);
      }
    }

    // each time we load WayPoint, we need to init WaypointCalc !!
    InitWayPointCalc();

  UnlockTaskData();

}
