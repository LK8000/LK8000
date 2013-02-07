/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *  
 * File:   CTaskFileHelper.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 29 janvier 2013, 23:49
 */

#include "externs.h"
#include "Waypointparser.h"
#include <string>


typedef std::map<std::wstring, WAYPOINT> mapCode2Waypoint_t;

bool LoadCupTask(LPCTSTR szFileName) {
    LockTaskData();

    mapCode2Waypoint_t mapWaypoint;

    ClearTask();
    size_t idxTP = 0;

    TCHAR szString[READLINE_LENGTH + 1];
    TCHAR TpCode[NAME_SIZE + 1];

    szString[READLINE_LENGTH] = _T('\0');
    TpCode[NAME_SIZE] = _T('\0');

    memset(szString, 0, sizeof (szString)); // clear Temp Buffer
    WAYPOINT newPoint = {0};

    enum {
        none, Waypoint, TaskTp, Option
    } FileSection = none;
    FILE * stream = _tfopen(szFileName, _T("rt"));
    if (stream) {
        while (ReadStringX(stream, READLINE_LENGTH, szString)) {

            if ((FileSection == none) && ((_tcsncmp(_T("name,code,country"), szString, 17) == 0) ||
                                          (_tcsncmp(_T("Title,Code,Country"), szString, 18) == 0))) {
                FileSection = Waypoint;
                continue;
            } else if ((FileSection == Waypoint) && (_tcscmp(szString, _T("-----Related Tasks-----")) == 0)) {
                FileSection = TaskTp;
                continue;
            }

            TCHAR *pToken = NULL;
            TCHAR *pWClast = NULL;

            switch (FileSection) {
                case Waypoint:
                    if (ParseCUPWayPointString(szString, &newPoint)) {
                        mapWaypoint[newPoint.Name] = newPoint;
                    }
                    break;
                case TaskTp:
                    // 1. Description
                    //       First column is the description of the task. If filled it should be double quoted.
                    //       If left empty, then SeeYou will determine the task type on runtime.
                    if ((pToken = strsep_r(szString, TEXT(","), &pWClast)) == NULL) {
                        UnlockTaskData();
                        return false;
                    }

                    // 2. and all successive columns, separated by commas
                    //       Each column represents one waypoint name double quoted. The waypoint name must be exactly the
                    //       same as the Long name of a waypoint listed above the Related tasks.
                    while ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) != NULL) {
                        _tcsncpy(TpCode, pToken, NAME_SIZE);
                        CleanCupCode(TpCode);
                        mapCode2Waypoint_t::iterator It = mapWaypoint.find(TpCode);
                        if (It != mapWaypoint.end()) {
                            Task[idxTP++].Index = FindOrAddWaypoint(&(It->second));
                        }
                    }
                    FileSection = Option;
                    break;
                case Option:
#if 0 // Unused
                    if ((pToken = strsep_r(szString, TEXT(","), &pWClast)) != NULL) {
                        if (_tcscmp(pToken, _T("Options")) == 0) {
                            while ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) != NULL) {
                                if (_tcscmp(pToken, _T("NoStart=")) == 0) {
                                    // Opening of start line
                                    // TODO :
                                } else if (_tcscmp(pToken, _T("TaskTime=")) == 0) {
                                    // Designated Time for the task
                                    // TODO :
                                } else if (_tcscmp(pToken, _T("WpDis=")) == 0) {
                                    // Task distance calculation. False = use fixes, True = use waypoints
                                    // TODO :
                                } else if (_tcscmp(pToken, _T("NearDis=")) == 0) {
                                    // Distance tolerance
                                    // TODO :
                                } else if (_tcscmp(pToken, _T("NearAlt=")) == 0) {
                                    // Altitude tolerance
                                    // TODO :
                                } else if (_tcscmp(pToken, _T("MinDis=")) == 0) {
                                    // Uncompleted leg. 
                                    // False = calculate maximum distance from last observation zone.
                                    // TODO :
                                } else if (_tcscmp(pToken, _T("RandomOrder=")) == 0) {
                                    // if true, then Random order of waypoints is checked
                                    // TODO :
                                } else if (_tcscmp(pToken, _T("MaxPts=")) == 0) {
                                    // Maximum number of points
                                    // TODO :
                                } else if (_tcscmp(pToken, _T("BeforePts=")) == 0) {
                                    // Number of mandatory waypoints at the beginning. 1 means start line only, two means
                                    //      start line plus first point in task sequence (Task line).
                                    // TODO :
                                } else if (_tcscmp(pToken, _T("AfterPts=")) == 0) {
                                    // Number of mandatory waypoints at the end. 1 means finish line only, two means finish line
                                    //      and one point before finish in task sequence (Task line).
                                    // TODO :
                                } else if (_tcscmp(pToken, _T("Bonus=")) == 0) {
                                    // Bonus for crossing the finish line         
                                    // TODO :
                                }
                            }
                        } else if (_tcscmp(pToken, _T("ObsZone=")) == 0) {
                            TCHAR *sz = NULL;
                            size_t idx = _tcstol(pToken + 8, &sz, 10);

                            while ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) != NULL) {
                                if (_tcscmp(pToken, _T("Style=")) == 0) {
                                    // Direction. 0 - Fixed value, 1 - Symmetrical, 2 - To next point, 3 - To previous point, 4 - To start point
                                    // TODO :
                                } else if (_tcscmp(pToken, _T("R1=")) == 0) {
                                    // Radius 1
                                    // TODO :
                                } else if (_tcscmp(pToken, _T("A1=")) == 0) {
                                    // Angle 1 in degrees
                                    // TODO :
                                } else if (_tcscmp(pToken, _T("R2=")) == 0) {
                                    // Radius 2
                                    // TODO :
                                } else if (_tcscmp(pToken, _T("A2=")) == 0) {
                                    // Angle 2 in degrees
                                    // TODO :
                                } else if (_tcscmp(pToken, _T("A12=")) == 0) {
                                    // Angle 12
                                    // TODO :
                                } else if (_tcscmp(pToken, _T("Line=")) == 0) {
                                    // ???
                                    // TODO :
                                }
                            }
                        }

                    }
                    break;
#endif
                case none:
                default:
                    break;
            }
            memset(szString, 0, sizeof (szString)); // clear Temp Buffer
        }
        fclose(stream);
    }
    UnlockTaskData();
    return ValidTaskPoint(0);
}

