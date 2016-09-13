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
#include "Util/tstring.hpp"
#include "NavFunctions.h"


typedef std::map<tstring, WAYPOINT> mapCode2Waypoint_t;

class CupObsZoneUpdater {
public:

    CupObsZoneUpdater() : mIdx(), mType(), mR1(), mA1(), mR2(), mA2(), mA12(), mLine() {
    }

    size_t mIdx;
    int mType;
    double mR1;
    double mA1;
    double mR2;
    double mA2;
    double mA12;
    bool mLine;

    void UpdateTask() {
        if (mA1 == 180.0) {
            if (mIdx == 0) {
                StartLine = 0;
                StartRadius = mR1;
            } else if (mIdx == (size_t) getFinalWaypoint()) {
                FinishLine = 0;
                FinishRadius = mR1;
            } else {
                Task[mIdx].AATType = CIRCLE;
                Task[mIdx].AATCircleRadius = mR1;
            }
        } else {

            switch (mType) {
                case 0: // - Fixed value,
                    if (mLine) {
                        StartupStore(_T("..Cup Task : \"Fixed\" LINE Turnpoint is not supported%s"), NEWLINE);
                        UpdateFixedLine();
                    } else {
                        UpdateFixedSector();
                    }
                    break;
                case 1: // - Symmetrical, 
                    if (mLine) {
                        StartupStore(_T("..Cup Task : \"Symmetrical\" LINE Turnpoint is not supported%s"), NEWLINE);
                        UpdateSymLine();
                    } else {
                        UpdateSymSector();
                    }
                    break;
                case 2: // - To next point, 
                    if (mLine) {
                        if (mIdx > 0) {
                            StartupStore(_T("..Cup Task : \"To next point\" LINE Turnpoint is not supported%s"), NEWLINE);
                        }
                        UpdateToNextLine();
                    } else {
                        UpdateToNextSector();
                    }
                    break;
                case 3: // - To previous point, 
                    if (mLine) {
                        if (mIdx < (size_t) getFinalWaypoint()) {
                            StartupStore(_T("..Cup Task : \"To previous point\" LINE Turnpoint is not supported%s"), NEWLINE);
                        }
                        UpdateToPrevLine();
                    } else {
                        UpdateToPrevSector();
                    }
                    break;
                case 4: // - To start point
                    if (mLine) {
                        StartupStore(_T("..Cup Task : \"To start point\" LINE Turnpoint is not supported%s"), NEWLINE);
                        UpdateToStartLine();
                    } else {
                        UpdateToStartSector();
                    }
                    break;
            }
        }
    }

    void UpdateFixedLine() {
        if ((mIdx == 0) || (mIdx == (size_t) getFinalWaypoint())) {
            UpdateSymLine();
        } else {
            StartupStore(_T("..Cup Task : LINE Turnpoint is only supported for Start or Finish%s"), NEWLINE);
            mA1 = 90.0;
            UpdateFixedSector();
        }
    }

    void UpdateFixedSector() {
        if (mIdx == 0) {
            StartLine = 2;
            StartRadius = mR1;
        } else if (mIdx == (size_t) getFinalWaypoint()) {
            FinishLine = 2;
            FinishRadius = mR1;
        } else {
            Task[mIdx].AATType = SECTOR;
            Task[mIdx].AATSectorRadius = mR1;
            Task[mIdx].AATStartRadial = mA12 - 180.0 - (mA1);
            Task[mIdx].AATFinishRadial = mA12 - 180.0 + (mA1);
        }
    }

    void UpdateSymLine() {
        if (mIdx == 0) {
            StartLine = 1;
            StartRadius = mR1;
        } else if (mIdx == (size_t) getFinalWaypoint()) {
            FinishLine = 1;
            FinishRadius = mR1;
        } else {
            StartupStore(_T("..Cup Task : LINE Turnpoint is only supported for Start or Finish%s"), NEWLINE);
            mA1 = 90.0;
            UpdateSymSector();
        }
    }

    void UpdateSymSector() {
        if (mIdx == 0) {
            UpdateToNextSector();
        } else if (mIdx == ((size_t) getFinalWaypoint())) {
            UpdateToPrevSector();
        } else {
            const WAYPOINT *CurrPt = TaskWayPoint(mIdx);
            const WAYPOINT *PrevPt = TaskWayPoint(mIdx - 1);
            const WAYPOINT *NextPt = TaskWayPoint(mIdx + 1);
            double InB = 0;
            double OutB = 0;
            // bearing to prev
            DistanceBearing(CurrPt->Latitude, CurrPt->Longitude,
                    PrevPt->Latitude, PrevPt->Longitude, NULL, &InB);
            // bearing to next
            DistanceBearing(CurrPt->Latitude, CurrPt->Longitude,
                    NextPt->Latitude, NextPt->Longitude, NULL, &OutB);
            mA12 = BiSector(InB, OutB);

            UpdateFixedSector();
        }
    }

    void UpdateToNextLine() {
        if ((mIdx == 0) || (mIdx == (size_t) getFinalWaypoint())) {
            UpdateSymLine();
        } else {
            StartupStore(_T("..Cup Task : LINE Turnpoint is only supported for Start or Finish%s"), NEWLINE);
            mA1 = 90.0;
            UpdateToNextSector();
        }
    }

    void UpdateToNextSector() {
        if (ValidTaskPoint(mIdx + 1)) {
            const WAYPOINT *CurrPt = TaskWayPoint(mIdx);
            const WAYPOINT *NextPt = TaskWayPoint(mIdx + 1);
            // bearing to next
            DistanceBearing(CurrPt->Latitude, CurrPt->Longitude,
                    NextPt->Latitude, NextPt->Longitude, NULL, &mA12);

            UpdateFixedSector();
        }
    }

    void UpdateToPrevLine() {
        if ((mIdx == 0) || (mIdx == (size_t) getFinalWaypoint())) {
            UpdateSymLine();
        } else {
            StartupStore(_T("..Cup Task : LINE Turnpoint is only supported for Start or Finish%s"), NEWLINE);
            mA1 = 90.0;
            UpdateToPrevSector();
        }
    }

    void UpdateToPrevSector() {
        if (ValidTaskPoint(mIdx - 1)) {
            const WAYPOINT *CurrPt = TaskWayPoint(mIdx);
            const WAYPOINT *PrevPt = TaskWayPoint(mIdx - 1);
            // bearing to prev
            DistanceBearing(CurrPt->Latitude, CurrPt->Longitude,
                    PrevPt->Latitude, PrevPt->Longitude, NULL, &mA12);

            UpdateFixedSector();
        }
    }

    void UpdateToStartLine() {
        if ((mIdx == 0) || (mIdx == (size_t) getFinalWaypoint())) {
            UpdateSymLine();
        } else {
            StartupStore(_T("..Cup Task : LINE Turnpoint is only supported for Start or Finish%s"), NEWLINE);
            mA1 = 90.0;
            UpdateToStartSector();
        }
    }

    void UpdateToStartSector() {
        if (mIdx > 0) {
            const WAYPOINT *CurrPt = TaskWayPoint(mIdx);
            const WAYPOINT *StartPt = TaskWayPoint(0);
            // bearing to prev
            DistanceBearing(CurrPt->Latitude, CurrPt->Longitude,
                    StartPt->Latitude, StartPt->Longitude, NULL, &mA12);

            UpdateFixedSector();
        }
    }
};

bool LoadCupTask(LPCTSTR szFileName) {
    LockTaskData();

    mapCode2Waypoint_t mapWaypoint;

    ClearTask();
    size_t idxTP = 0;
    bool bTakeOff = true;
    bool bLoadComplet = true;
    bool bLastInvalid=true;

    TCHAR szString[READLINE_LENGTH + 1];
    TCHAR TpCode[NAME_SIZE + 1];

    szString[READLINE_LENGTH] = _T('\0');
    TpCode[NAME_SIZE] = _T('\0');

    memset(szString, 0, sizeof (szString)); // clear Temp Buffer
    WAYPOINT newPoint = {0};
    WAYPOINT* WPtoAdd=NULL;

    enum {
        none, Waypoint, TaskTp, Option
    } FileSection = none;
    FILE * stream = _tfopen(szFileName, _T("rt"));
    if (stream) {
        charset cs = charset::unknown;
        while (ReadStringX(stream, READLINE_LENGTH, szString, cs)) {

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
                    memset(&newPoint, 0, sizeof(newPoint));
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
                    WPtoAdd=NULL;
                    while (bLoadComplet && (pToken = strsep_r(NULL, TEXT(","), &pWClast)) != NULL) {
                        if (idxTP < MAXTASKPOINTS) {
                            _tcsncpy(TpCode, pToken, NAME_SIZE);
                            CleanCupCode(TpCode);
                            mapCode2Waypoint_t::iterator It = mapWaypoint.find(TpCode);
                            if(!ISGAAIRCRAFT) {
                                if (It != mapWaypoint.end()) {
                                    if (bTakeOff) {
                                        // skip TakeOff Set At Home Waypoint
                                        int ix = FindOrAddWaypoint(&(It->second),false);
                                        if (ix>=0) {
                                            #if 0 // REMOVE
                                            // We must not change HomeWaypoint without user knowing!
                                            // The takeoff and homewaypoint are independent from task.
                                            // In addition, this is a bug because on next run the index is invalid
                                            // and we have no more HowWaypoint!
                                            HomeWaypoint = ix;
                                            #endif
                                            bTakeOff = false;
                                        }
                                        #if BUGSTOP
                                        else LKASSERT(0); // .. else is unmanaged, TODO
                                        #endif
                                    } else {
                                        int ix =  FindOrAddWaypoint(&(It->second),false);
                                        if (ix>=0) Task[idxTP++].Index = ix;
                                        #if BUGSTOP
                                        else LKASSERT(0); // .. else is unmanaged, TODO
                                        #endif
                                    }
                                    bLastInvalid=false;
                                } else {
                                    // An invalid takeoff, probably a "???" , which we ignore
                                    #if TESTBENCH
                                    if (bTakeOff) StartupStore(_T("....... CUP Takeoff not found: <%s>\n"),TpCode);
                                    #endif
                                    // in any case bTakeOff now is false
                                    bTakeOff=false;
                                    bLastInvalid=true;
                                }
                            } else { //ISGAIRRCRAFT
                                if(It != mapWaypoint.end()) {
                                    if(WPtoAdd!=NULL) {
                                        //add what we found in previous cycle: it was not the last one
                                        int ix = FindOrAddWaypoint(WPtoAdd,false); 
                                        if (ix>=0) Task[idxTP++].Index = ix;
                                        #if BUGSTOP
                                        else LKASSERT(0); // .. else is unmanaged, TODO
                                        #endif
                                    }
                                    if (bTakeOff) { //it's the first: may be we have a corresponding airfield
                                        //look for departure airfield and add it
                                        int ix = FindOrAddWaypoint(&(It->second),true); 
                                        if (ix>=0) {
                                            Task[idxTP++].Index = ix;
                                            bTakeOff = false;
                                        } 
                                        #if BUGSTOP
                                        else LKASSERT(0); // .. else is unmanaged, TODO
                                        #endif
                                    } else WPtoAdd=&(It->second); //store it for next cycle (may be it is the last one)
                                }
                            }
                        } else {
                            bLoadComplet = false;
                        }
                    }
                    if(ISGAAIRCRAFT) { //For GA: check if we have an airport corresponding to the last WP
                        if(WPtoAdd!=NULL) { //if we have the last one (probably an airfield) still to add...
                            if(idxTP<MAXTASKPOINTS) {
                                int ix=FindOrAddWaypoint(WPtoAdd,true); //look for arrival airport and add it
                                if (ix>=0) {
                                    Task[idxTP++].Index= ix;
                                }
                                #if BUGSTOP
                                else LKASSERT(0); // .. else is unmanaged, TODO
                                #endif
                            }
                            else bLoadComplet=false;
                        }
                    }
                    FileSection = Option;
                break;
                case Option:
                    if ((pToken = strsep_r(szString, TEXT(","), &pWClast)) != NULL) {
                        if (_tcscmp(pToken, _T("Options")) == 0) {
                            while ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) != NULL) {
                                if (_tcsstr(pToken, _T("NoStart=")) == pToken) {
                                    // Opening of start line
                                    PGNumberOfGates = 1;
                                    StrToTime(pToken + 8, &PGOpenTimeH, &PGOpenTimeM);
                                } else if (_tcsstr(pToken, _T("TaskTime=")) == pToken) {
                                    // Designated Time for the task
                                    // TODO :
                                } else if (_tcsstr(pToken, _T("WpDis=")) == pToken) {
                                    // Task distance calculation. False = use fixes, True = use waypoints
                                    // TODO :
                                } else if (_tcsstr(pToken, _T("NearDis=")) == pToken) {
                                    // Distance tolerance
                                    // TODO :
                                } else if (_tcsstr(pToken, _T("NearAlt=")) == pToken) {
                                    // Altitude tolerance
                                    // TODO :
                                } else if (_tcsstr(pToken, _T("MinDis=")) == pToken) {
                                    // Uncompleted leg. 
                                    // False = calculate maximum distance from last observation zone.
                                    // TODO :
                                } else if (_tcsstr(pToken, _T("RandomOrder=")) == pToken) {
                                    // if true, then Random order of waypoints is checked
                                    // TODO :
                                } else if (_tcsstr(pToken, _T("MaxPts=")) == pToken) {
                                    // Maximum number of points
                                    // TODO :
                                } else if (_tcsstr(pToken, _T("BeforePts=")) == pToken) {
                                    // Number of mandatory waypoints at the beginning. 1 means start line only, two means
                                    //      start line plus first point in task sequence (Task line).
                                    // TODO :
                                } else if (_tcsstr(pToken, _T("AfterPts=")) == pToken) {
                                    // Number of mandatory waypoints at the end. 1 means finish line only, two means finish line
                                    //      and one point before finish in task sequence (Task line).
                                    // TODO :
                                } else if (_tcsstr(pToken, _T("Bonus=")) == pToken) {
                                    // Bonus for crossing the finish line         
                                    // TODO :
                                }
                            }
                        } else if (_tcsstr(pToken, _T("ObsZone=")) == pToken) {
                            TCHAR *sz = NULL;
                            CupObsZoneUpdater TmpZone;
                            TmpZone.mIdx = _tcstol(pToken + 8, &sz, 10);
                            if (TmpZone.mIdx < MAXSTARTPOINTS) {
                                while ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) != NULL) {
                                    if (_tcsstr(pToken, _T("Style=")) == pToken) {
                                        // Direction. 0 - Fixed value, 1 - Symmetrical, 2 - To next point, 3 - To previous point, 4 - To start point
                                        TmpZone.mType = _tcstol(pToken + 6, &sz, 10);
                                    } else if (_tcsstr(pToken, _T("R1=")) == pToken) {
                                        // Radius 1
                                        TmpZone.mR1 = ReadLength(pToken + 3);
                                    } else if (_tcsstr(pToken, _T("A1=")) == pToken) {
                                        // Angle 1 in degrees
                                        TmpZone.mA1 = _tcstod(pToken + 3, &sz);
                                    } else if (_tcsstr(pToken, _T("R2=")) == pToken) {
                                        // Radius 2
                                        TmpZone.mR2 = ReadLength(pToken + 3);
                                    } else if (_tcsstr(pToken, _T("A2=")) == pToken) {
                                        // Angle 2 in degrees
                                        TmpZone.mA2 = _tcstod(pToken + 3, &sz);
                                    } else if (_tcsstr(pToken, _T("A12=")) == pToken) {
                                        // Angle 12
                                        TmpZone.mA12 = _tcstod(pToken + 4, &sz);
                                    } else if (_tcsstr(pToken, _T("Line=")) == pToken) {
                                        // true For Line Turmpoint type 
                                        // Exist only for start an Goalin LK
                                        TmpZone.mLine = (_tcstol(pToken + 5, &sz, 10) == 1);
                                    }
                                }
                                TmpZone.UpdateTask();
                            }
                        }
                    }
                    break;
                case none:
                default:
                    break;
            }
            memset(szString, 0, sizeof (szString)); // clear Temp Buffer
        }
        fclose(stream);
    }
    if(!ISGAAIRCRAFT) {
        // Landing don't exist in LK Task Systems, so Remove It;
        if ( bLoadComplet && !bLastInvalid ) {
            RemoveTaskPoint(getFinalWaypoint());
        }
    }
    UnlockTaskData();
    for (mapCode2Waypoint_t::iterator It = mapWaypoint.begin(); It != mapWaypoint.end(); ++It) {
        if (It->second.Comment) {
            free(It->second.Comment);
        }
        if (It->second.Details) {
            free(It->second.Details);
        }
    }
    mapWaypoint.clear();

    return ValidTaskPoint(0);
}

