/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "utils/zzip_stream.h"

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
    static_assert(std::size(szWaypointFile) == std::size(WpFileType), "invalid array size");

    for( TCHAR* szFile : szWaypointFile) {

        if (_tcslen(szFile) > 0) {
            TCHAR szFilePath[MAX_PATH];
            LocalPath(szFilePath, _T(LKD_WAYPOINTS), szFile);
            int fileformat=GetWaypointFileFormatType(szFilePath);
            bool not_found = true;
            if(fileformat == LKW_OPENAIP) {
                zzip_file_ptr file(openzip(szFilePath, "rt"));
                if(file) {
                  StartupStore(_T(". Waypoint file %d format: OpenAIP"),globalFileNum);
                  if(ParseOpenAIP(file)) {
                    WpFileType[globalFileNum] = LKW_OPENAIP;
                    not_found = false;
                  }
                }
            } else {
              zzip_stream stream(szFilePath, "rt");
              if (stream) {
                WpFileType[globalFileNum] = ReadWayPointFile(stream, fileformat);
                not_found = false;
              }
            }
            
            if (not_found) {
                StartupStore(TEXT("--- No waypoint file %d"), globalFileNum);
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
