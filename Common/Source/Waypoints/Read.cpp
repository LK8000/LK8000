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

    LockTaskData();
    CloseWayPoints(); // BUGFIX 091104 duplicate waypoints entries
    InitVirtualWaypoints();	// 091103

    globalFileNum = 0;
/*
    TCHAR* WaypointFileList[] = {
        szWaypointFile,
        szAdditionalWaypointFile
    };*/

    for( TCHAR* szFile : szWaypointFile) {

        if (_tcslen(szFile) > 0) {
            TCHAR szFilePath[MAX_PATH];
            LocalPath(szFilePath, _T(LKD_WAYPOINTS), szFile);
            ZZIP_FILE* fp = openzip(szFilePath, "rt");
            if (fp) {
                WpFileType[globalFileNum+1] = ReadWayPointFile(fp, szFilePath);
                zzip_fclose(fp);
                fp = nullptr;
            } else {
                StartupStore(TEXT("--- No waypoint file %d"), globalFileNum+1);
                // file not found : reset config
                _tcscpy(szFile, _T(""));
            }
        }

        ++globalFileNum;
    }

    // each time we load WayPoint, we need to init WaypointCalc !!
    InitWayPointCalc();

    UnlockTaskData();
}
