/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "utils/zzip_file_stream.h"
#include "LocalPath.h"
#include "cupx_reader.h"

int globalFileNum = 0;

void InitVirtualWaypoints();
void InitWayPointCalc(void);

void ReadWayPoints(void)
{
    TestLog(TEXT(". ReadWayPoints"));

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
            if (fileformat == LKW_CUPX) {
              try {
                cupx_reader cupx(szFilePath);
                zzip_disk_file_stream stream = cupx.read_points_cup();
                std::istream in(&stream);
                ReadWayPointFile(in, LKW_CUP);
                WpFileType[globalFileNum] = LKW_CUPX;
                not_found = false;
              }
              catch (const std::exception& e) {
                StartupStore(TEXT(R"(---- cupx exception : %d : "%s")"), globalFileNum, e.what());
              }
            }
            else {
              zzip_file_stream stream(szFilePath, "rt");
              if (stream) {
                if (fileformat == LKW_OPENAIP) {
                  if (ParseOpenAIP(stream)) {
                    WpFileType[globalFileNum] = LKW_OPENAIP;
                    not_found = false;
                  }
                }
                else {
                  std::istream in(&stream);
                  WpFileType[globalFileNum] = ReadWayPointFile(in, fileformat);
                  not_found = false;
                }
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
