/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *  
 * File:   CTaskFileHelper.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 19 janvier 2013, 12:25
 */

#include <math.h>
#include <string>
#include <stdarg.h>
#include "CTaskFileHelper.h"
#include "Util/tstring.hpp"
#include "utils/fileext.h"
#include "utils/stringext.h"
#include "Waypointparser.h"
#include "Util/UTF8.hpp"
#include "CriticalSection.h"


extern void RenameIfVirtual(const unsigned int i);
extern bool FullResetAsked;


LPTSTR AllocFormat(LPCTSTR fmt, ...) {
    int n;
    int size = 50; /* Guess we need no more than 100 bytes. */
    LPTSTR p;
    LPTSTR np;
    va_list ap;

    if ((p = (LPTSTR) malloc(size * sizeof (TCHAR))) == NULL)
        return NULL;

    while (1) {
        /* Try to print in the allocated space. */
        va_start(ap, fmt);
        n = _vsntprintf(p, size, fmt, ap);
        va_end(ap);

        /* If that worked, return the string. */
        if (n > -1 && n < size)
            return p;

        /* Else try again with more space. */
        if (n > -1) /* glibc 2.1 */
            size = n + 1; /* precisely what is needed */
        else /* glibc 2.0 */
            size *= 2; /* twice the old size */

        if ((np = (LPTSTR) realloc(p, size * sizeof (TCHAR))) == NULL) {
            LKASSERT(np);
            free(p);
            return NULL;
        } else {
            p = np;
        }
    }
}

inline LPTSTR ToString(unsigned ulong) {
    return AllocFormat(_T("%u"), ulong);
}

inline LPTSTR ToString(LPCTSTR szString) {
    return AllocFormat(_T("%s"), szString);
}

inline LPTSTR ToString(double dVal) {
    return AllocFormat(_T("%f"), dVal);
}

inline LPTSTR ToString(int iVal) {
    return AllocFormat(_T("%d"), iVal);
}

inline LPTSTR ToString(bool bVal) {
    return AllocFormat(_T("%s"), bVal ? _T("true") : _T("false"));
}

inline void FromString(LPCTSTR szVal, unsigned& ulong) {
    TCHAR * sz = NULL;
    if (szVal) {
        ulong = _tcstoul(szVal, &sz, 10);
    }
}

inline void FromString(LPCTSTR szVal, LPCTSTR& szString) {
    szString = szVal;
}

inline void FromString(LPCTSTR szVal, double& dVal) {
    TCHAR * sz = NULL;
    if (szVal) {
        dVal = _tcstod(szVal, &sz);
    }
}

inline void FromString(LPCTSTR szVal, int& iVal) {
    TCHAR * sz = NULL;
    if (szVal) {
        iVal = _tcstol(szVal, &sz, 10);
    }
}

inline void FromString(LPCTSTR szVal, short& iVal) {
    TCHAR * sz = NULL;
    if (szVal) {
        iVal = _tcstol(szVal, &sz, 10);
    }
}

inline void FromString(LPCTSTR szVal, bool& bVal) {
    if (szVal) {
        bVal = (_tcscmp(szVal, _T("true")) == 0);
    }
}

#define SetAttribute(node, name, val) if(!node.AddAttribute(ToString(name), ToString(val))) { return false; } 
#define GetAttribute(node, name, val) FromString(node.getAttribute(name, 0), val)

bool getFirstTaskWayPointName(XMLNode node, TCHAR *firstWPname) {
    if(node) {
        XMLNode WPnode = node.getChildNode(_T("point"),0);
        if(WPnode) {
            unsigned idx = MAXTASKPOINTS;
            GetAttribute(WPnode, _T("idx"), idx);
            if(idx==0) {
                LPCTSTR first=NULL;
                GetAttribute(WPnode, _T("name"),first);
                if(first) {
                    if(_tcslen(first)<=NAME_SIZE) {
                        _tcscpy(firstWPname,first);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool getLastTaskWayPointName(XMLNode node, TCHAR *lastWPname) {
    if(node) {
        int numOfWPs=node.nChildNode(_T("point")); //count number of WPs in the route
        if(numOfWPs>=2) {
            XMLNode WPnode = node.getChildNode(_T("point"),numOfWPs-1);
            if(WPnode) {
                unsigned idx = MAXTASKPOINTS;
                GetAttribute(WPnode, _T("idx"), idx);
                if(idx==(unsigned long)(numOfWPs-1)) {
                    LPCTSTR last=NULL;
                    GetAttribute(WPnode, _T("name"),last);
                    if(last) {
                        if(_tcslen(last)<=NAME_SIZE) {
                            _tcscpy(lastWPname,last);
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

CTaskFileHelper::CTaskFileHelper() : mFinishIndex() {
}

CTaskFileHelper::~CTaskFileHelper() {
}

bool CTaskFileHelper::Load(const TCHAR* szFileName) {
    CScopeLock LockTask(LockTaskData, UnlockTaskData);
    StartupStore(_T(". LoadTask : <%s>%s"), szFileName, NEWLINE);
    TCHAR taskFileName[MAX_PATH];

    ClearTask();
    if (FullResetAsked) {
        #if TESTBENCH
        StartupStore(_T("... LoadTask detected FullResetAsked, attempt to load DEMO.lkt\n"));
        #endif
        // Clear the flag, forever.
        FullResetAsked=false;

        LocalPath(taskFileName, _T(LKD_TASKS),  _T("DEMO.lkt"));
    } else {
        _tcscpy(taskFileName, szFileName);
    }


    FILE* stream = _tfopen(taskFileName, TEXT("rb"));
    if (stream) {
        fseek(stream, 0, SEEK_END); // seek to end of file
        long size = ftell(stream); // get current file pointer
        fseek(stream, 0, SEEK_SET); // seek back to beginning of file

        char * buff = (char*) calloc(size + 1, sizeof (char));
        long nRead = fread(buff, sizeof (char), size, stream);
        if (nRead != size) {
            fclose(stream);
            free(buff);
            return false;
        }
        fclose(stream);
#ifdef UNICODE
        TCHAR * szXML = (TCHAR*) calloc(size + 1, sizeof (TCHAR));
        from_utf8(buff, szXML, size + 1);
        free(buff);
#else
        TCHAR * szXML = buff;
        if(!ValidateUTF8(szXML)) {
            StartupStore(_T(".. error : Invalid file encoding") NEWLINE);
            free(szXML);
            return false;
        }
#endif
        XMLNode rootNode = XMLNode::parseString(szXML, _T("lk-task"));

        if (rootNode) {
            LoadOptions(rootNode);

            if(ISGAAIRCRAFT) {
                TCHAR firstWPname[NAME_SIZE+1];
                TCHAR lastWPname[NAME_SIZE+1];
                XMLNode taskNode=rootNode.getChildNode(_T("taskpoints"),0);
                bool gotFirstWPname=getFirstTaskWayPointName(taskNode,firstWPname);
                bool gotLastWPname=getLastTaskWayPointName(taskNode,lastWPname);
                LoadWayPointList(rootNode.getChildNode(_T("waypoints"), 0),gotFirstWPname?firstWPname:NULL,gotLastWPname?lastWPname:NULL);
            } else
                LoadWayPointList(rootNode.getChildNode(_T("waypoints"), 0),NULL,NULL);

            if (!LoadTaskPointList(rootNode.getChildNode(_T("taskpoints"), 0))) {
                free(szXML);
                return false;
            }
            if (!LoadStartPointList(rootNode.getChildNode(_T("startpoints"), 0))) {
                free(szXML);
                return false;
            }
        }

        free(szXML);
    }

    RefreshTask();

    TaskModified = false;
    TargetModified = false;
    // We are not using the DEMO.lkt forced by FULL RESET. We use the original task filename.
    _tcscpy(LastTaskFileName, szFileName);

    return true;
}

void CTaskFileHelper::LoadOptions(XMLNode node) {
    if (node) {
        XMLNode nodeOpt = node.getChildNode(_T("options"));
        if (nodeOpt) {
            LPCTSTR szAttr = NULL;
            GetAttribute(nodeOpt, _T("auto-advance"), szAttr);
            if (szAttr) {
                if (_tcscmp(szAttr, _T("Manual")) == 0) {
                    AutoAdvance = 0;
                } else if (_tcscmp(szAttr, _T("Auto")) == 0) {
                    AutoAdvance = 1;
                } else if (_tcscmp(szAttr, _T("Arm")) == 0) {
                    AutoAdvance = 2;
                } else if (_tcscmp(szAttr, _T("ArmStart")) == 0) {
                    AutoAdvance = 3;
                } else if (_tcscmp(szAttr, _T("ArmTPs")) == 0) {
                    AutoAdvance = 4;
                }
            }

            GetAttribute(node, _T("type"), szAttr);
            if (szAttr) {
                if (_tcscmp(szAttr, _T("AAT")) == 0) {
                    gTaskType = TSK_AAT;
                    LoadOptionAAT(nodeOpt);
                } else if (_tcscmp(szAttr, _T("Race")) == 0) {
                    gTaskType = TSK_GP;
                    LoadOptionRace(nodeOpt);
                } else if (_tcscmp(szAttr, _T("Default")) == 0) {
                    gTaskType = TSK_DEFAULT;
                    LoadOptionDefault(nodeOpt);
                }
            }
            LoadRules(nodeOpt.getChildNode(_T("rules"), 0));
        }
    }
}

void CTaskFileHelper::LoadOptionAAT(XMLNode node) {
    if (node) {
        GetAttribute(node, _T("length"), AATTaskLength);
    }
}

void CTaskFileHelper::LoadOptionRace(XMLNode node) {
    if (node) {
        LoadTimeGate(node.getChildNode(_T("time-gate"), 0));
    }
    // SS Turnpoint
    // ESS Turnpoint
}

void CTaskFileHelper::LoadTimeGate(XMLNode node) {
    if (node) {
        GetAttribute(node, _T("number"), PGNumberOfGates);
        LPCTSTR szTime = NULL;
        GetAttribute(node, _T("open-time"), szTime);
        StrToTime(szTime, &PGOpenTimeH, &PGOpenTimeM);
        GetAttribute(node, _T("close-time"), szTime);
        StrToTime(szTime, &PGCloseTimeH, &PGCloseTimeM);
        GetAttribute(node, _T("interval-time"), PGGateIntervalTime);
    } else {
        PGNumberOfGates = 0;
    }
    InitActiveGate();
    
}

void CTaskFileHelper::LoadOptionDefault(XMLNode node) {
    if (node) {
        XMLNode nodeStart = node.getChildNode(_T("start"), 0);
        if (nodeStart) {
            LPCTSTR szType = NULL;
            GetAttribute(nodeStart, _T("type"), szType);
            if (szType) {
                if (_tcscmp(szType, _T("circle")) == 0) {
                    StartLine = 0;
                } else if (_tcscmp(szType, _T("line")) == 0) {
                    StartLine = 1;
                } else if (_tcscmp(szType, _T("sector")) == 0) {
                    StartLine = 2;
                }
            }
            GetAttribute(nodeStart, _T("radius"), StartRadius);
        }

        XMLNode nodeFinish = node.getChildNode(_T("finish"), 0);
        if (nodeFinish) {
            LPCTSTR szType = NULL;
            GetAttribute(nodeFinish, _T("type"), szType);
            if (szType) {
                if (_tcscmp(szType, _T("circle")) == 0) {
                    FinishLine = 0;
                } else if (_tcscmp(szType, _T("line")) == 0) {
                    FinishLine = 1;
                } else if (_tcscmp(szType, _T("sector")) == 0) {
                    FinishLine = 2;
                }
            }
            GetAttribute(nodeFinish, _T("radius"), FinishRadius);
        }

        XMLNode nodeSector = node.getChildNode(_T("sector"), 0);
        if (nodeSector) {
            LPCTSTR szType = NULL;
            GetAttribute(nodeSector, _T("type"), szType);
            if (_tcscmp(szType, _T("circle")) == 0) {
                SectorType = CIRCLE;
            } else if (_tcscmp(szType, _T("sector")) == 0) {
                SectorType = SECTOR;
            } else if (_tcscmp(szType, _T("DAe")) == 0) {
                SectorType = DAe;
            } else if (_tcscmp(szType, _T("line")) == 0) {
                SectorType = LINE;
            }
            GetAttribute(nodeSector, _T("radius"), SectorRadius);
        }
    }
}

void CTaskFileHelper::LoadRules(XMLNode node) {
    if (node) {
        XMLNode nodeFinish = node.getChildNode(_T("finish"), 0);
        if (nodeFinish) {
            GetAttribute(nodeFinish, _T("fai-height"), EnableFAIFinishHeight);
            GetAttribute(nodeFinish, _T("min-height"), FinishMinHeight);
        }
        XMLNode nodeStart = node.getChildNode(_T("start"), 0);
        if (nodeStart) {
            GetAttribute(nodeStart, _T("max-height"), StartMaxHeight);
            GetAttribute(nodeStart, _T("max-height-margin"), StartMaxHeightMargin);
            GetAttribute(nodeStart, _T("max-speed"), StartMaxSpeed);
            GetAttribute(nodeStart, _T("max-speed-margin"), StartMaxSpeedMargin);

            LPCTSTR szAttr = NULL;
            GetAttribute(nodeStart, _T("height-ref"), szAttr);
            if (szAttr) {
                if (_tcscmp(szAttr, _T("AGL")) == 0) {
                    StartHeightRef = 0;
                } else if (_tcscmp(szAttr, _T("ASL")) == 0) {
                    StartHeightRef = 1;
                }
            }
        }
    }
}

bool CTaskFileHelper::LoadTaskPointList(XMLNode node) {
    mFinishIndex = 0;
    if (node) {
        int i = 0;
        XMLNode nodePoint = node.getChildNode(_T("point"), &i);
        while (nodePoint) {
            if (!LoadTaskPoint(nodePoint)) {
                return false;
            }
            nodePoint = node.getChildNode(_T("point"), &i);
        }
    }


    ///////////////////////////////////////////////////////////////
    // TODO : this code is temporary before rewriting task system
    if (UseAATTarget()) {
        if (ValidTaskPoint(mFinishIndex)) {
            switch (Task[mFinishIndex].AATType) {
                case CIRCLE:
                    FinishRadius = Task[mFinishIndex].AATCircleRadius;
                    FinishLine = 0;
                    break;
                case LINE:
                    FinishRadius = Task[mFinishIndex].AATCircleRadius;
                    FinishLine = 1;
                    break;
                case SECTOR:
                    FinishRadius = Task[mFinishIndex].AATSectorRadius;
                    FinishLine = 2;
                    break;
            }
        }
        if (ValidTaskPoint(0)) {
            switch (Task[0].AATType) {
                case CIRCLE:
                    StartRadius = Task[0].AATCircleRadius;
                    StartLine = 0;
                    PGStartOut = !Task[0].OutCircle;
                    break;
                case LINE:
                    StartRadius = Task[0].AATCircleRadius;
                    StartLine = 1;
                    break;
                case SECTOR:
                    StartRadius = Task[0].AATSectorRadius;
                    StartLine = 2;
                    break;
            }
        }
        for(unsigned i = 1; ValidTaskPoint(i+1); ++i) {
            switch (Task[i].AATType) {
                case CIRCLE:
                case SECTOR:
                    break;
                case DAe:
                case LINE:
                    LKASSERT(FALSE);
                    break;
                case CONE:
                    Task[i].AATType = 2;
                    break;
                case ESS_CIRCLE:
                    Task[i].AATType = 3;
                    break;
            }
        }
    }
    ///////////////////////////////////////////////////////////////


    return true;
}

bool CTaskFileHelper::LoadStartPointList(XMLNode node) {
    if (node) {
        int i = 0;
        XMLNode nodePoint = node.getChildNode(_T("point"), &i);
        while (nodePoint) {
            if (!LoadStartPoint(nodePoint)) {
                return false;
            }
            nodePoint = node.getChildNode(_T("point"), &i);
        }
        EnableMultipleStartPoints = ValidStartPoint(0);
    }
    return true;
}

void CTaskFileHelper::LoadWayPointList(XMLNode node, TCHAR *firstWPname, TCHAR *lastWPname) {
    if (node) {
        int i = 0;
        XMLNode nodePoint = node.getChildNode(_T("point"), &i);
        while (nodePoint) {
            LoadWayPoint(nodePoint,firstWPname,lastWPname);
            nodePoint = node.getChildNode(_T("point"), &i);
        }
    }
}

bool CTaskFileHelper::LoadTaskPoint(XMLNode node) {
    if (node) {
        unsigned idx = MAXTASKPOINTS;
        GetAttribute(node, _T("idx"), idx);
        LPCTSTR szName = NULL;
        GetAttribute(node, _T("name"), szName);
        if (idx >= MAXTASKPOINTS || szName == NULL) {
            return false; // invalide TaskPoint index
        }
        std::map<tstring, size_t>::const_iterator it = mWayPointLoaded.find(szName);
        if (it == mWayPointLoaded.end()) {
            return false; // non existing Waypoint
        }
        // cannot happen
        if (!ValidWayPointFast(it->second)) {
            StartupStore(_T("... LoadTaskPoint invalid, ignored\n"));
            return false;
        }

        Task[idx].Index = it->second;

        mFinishIndex = std::max(mFinishIndex, idx);

        LPCTSTR szType = NULL;
        GetAttribute(node, _T("type"), szType);
        if (szType) {
            if (_tcscmp(szType, _T("circle")) == 0) {
                Task[idx].AATType = CIRCLE;
                GetAttribute(node, _T("radius"), Task[idx].AATCircleRadius);
                GetAttribute(node, _T("Exit"), Task[idx].OutCircle);
            } else if (_tcscmp(szType, _T("sector")) == 0) {
                Task[idx].AATType = SECTOR;
                GetAttribute(node, _T("radius"), Task[idx].AATSectorRadius);
                GetAttribute(node, _T("start-radial"), Task[idx].AATStartRadial);
                GetAttribute(node, _T("end-radial"), Task[idx].AATFinishRadial);
            } else if (_tcscmp(szType, _T("line")) == 0) {
                Task[idx].AATType = LINE;
                GetAttribute(node, _T("radius"), Task[idx].AATCircleRadius);
            } else if (_tcscmp(szType, _T("DAe")) == 0) {
                Task[idx].AATType = DAe; // not Used in AAT and PGTask
            } else if (_tcscmp(szType, _T("cone")) == 0) {
                Task[idx].AATType = CONE; // Only Used in PGTask
                GetAttribute(node, _T("base"), Task[idx].PGConeBase);
                GetAttribute(node, _T("radius"), Task[idx].PGConeBaseRadius);
                GetAttribute(node, _T("slope"), Task[idx].PGConeSlope);
                Task[idx].OutCircle = false;
            } else if (_tcscmp(szType, _T("ess_circle")) == 0) {
                Task[idx].AATType = ESS_CIRCLE;
                GetAttribute(node, _T("radius"), Task[idx].AATCircleRadius);
                GetAttribute(node, _T("Exit"), Task[idx].OutCircle);
            }
        }
        GetAttribute(node, _T("lock"), Task[idx].AATTargetLocked);
        GetAttribute(node, _T("target-lat"), Task[idx].AATTargetLat);
        GetAttribute(node, _T("target-lon"), Task[idx].AATTargetLon);
        GetAttribute(node, _T("offset-radius"), Task[idx].AATTargetOffsetRadius);
        GetAttribute(node, _T("offset-radial"), Task[idx].AATTargetOffsetRadial);
    }
    return true;
}

bool CTaskFileHelper::LoadStartPoint(XMLNode node) {
    if (node) {
        unsigned idx = MAXSTARTPOINTS;
        GetAttribute(node, _T("idx"), idx);
        LPCTSTR szName = NULL;
        GetAttribute(node, _T("name"), szName);

        if (idx >= MAXSTARTPOINTS || szName == NULL) {
            return false; // invalide TaskPoint index
        }
        std::map<tstring, size_t>::const_iterator it = mWayPointLoaded.find(szName);
        if (it == mWayPointLoaded.end()) {
            return false; // non existing Waypoint
        }
        // cannot happen, but if it happens..
        if (!ValidWayPointFast(it->second)) {
            StartupStore(_T("... LoadStartPoint invalid, ignored\n"));
            return false;
        }
        
        StartPoints[idx].Index = it->second;
        StartPoints[idx].Active = true;
    }
    return true;
}

void CTaskFileHelper::LoadWayPoint(XMLNode node, TCHAR *firstWPname, TCHAR *lastWPname) {
    LPCTSTR szAttr = NULL;
    WAYPOINT newPoint;
    memset(&newPoint, 0, sizeof (newPoint));

    GetAttribute(node, _T("code"), szAttr);
    if (szAttr) {
        _tcscpy(newPoint.Code, szAttr);
    }
    GetAttribute(node, _T("name"), szAttr);
    if (szAttr) {
        _tcscpy(newPoint.Name, szAttr);
    }
    bool lookupAirfield=false;
    if(ISGAAIRCRAFT) {
        if(firstWPname) {
            if(_tcscmp(newPoint.Name,firstWPname)==0) lookupAirfield=true;
        }
        if(lastWPname && !lookupAirfield) {
            if(_tcscmp(newPoint.Name,lastWPname)==0) lookupAirfield=true;
        }
    }
    GetAttribute(node, _T("latitude"), newPoint.Latitude);
    GetAttribute(node, _T("longitude"), newPoint.Longitude);
    GetAttribute(node, _T("altitude"), newPoint.Altitude);
    GetAttribute(node, _T("flags"), newPoint.Flags);
    GetAttribute(node, _T("comment"), szAttr);
    SetWaypointComment(newPoint, szAttr);
#if TASK_DETAILS
    GetAttribute(node, _T("details"), szAttr);
    SetWaypointDetails(newPoint, szAttr);
#endif
    GetAttribute(node, _T("format"), newPoint.Format);
    GetAttribute(node, _T("freq"), szAttr);
    if (szAttr) {
        _tcscpy(newPoint.Freq, szAttr);
    }
    GetAttribute(node, _T("runwayLen"), newPoint.RunwayLen);
    GetAttribute(node, _T("runwayDir"), newPoint.RunwayDir);
    GetAttribute(node, _T("country"), szAttr);
    if (szAttr) {
        _tcscpy(newPoint.Country, szAttr);
    }
    GetAttribute(node, _T("style"), newPoint.Style);

    // NOTICE: we must consider that FindOrAdd can return -1
    int ix = FindOrAddWaypoint(&newPoint,lookupAirfield);
    if (ix < 0)  { // -1 actually
        StartupStore(_T("... Failed task LoadWaypoint <%s>, not loaded.\n"),newPoint.Name);
        return;
    }
    mWayPointLoaded[newPoint.Name] = ix;
#if TASK_DETAILS
    if (newPoint.Details) {
        free(newPoint.Details);
    }
#endif
    if (newPoint.Comment) {
        free(newPoint.Comment);
    }
}

bool CTaskFileHelper::Save(const TCHAR* szFileName) {
    if (WayPointList.empty()) return false; // this should never happen, but just to be safe...

    CScopeLock LockTask(LockTaskData, UnlockTaskData);
    StartupStore(_T(". SaveTask : saving <%s>%s"), szFileName, NEWLINE);
    
    XMLNode topNode = XMLNode::createXMLTopNode();
    XMLNode rootNode = topNode.AddChild(ToString(_T("lk-task")), false);

    if (!SaveOption(rootNode)) {
        return false;
    }

    if (!SaveTaskPointList(rootNode.AddChild(ToString(_T("taskpoints")), false))) {
        return false;
    }
    if (EnableMultipleStartPoints && ValidStartPoint(0)) {
        if (!SaveStartPointList(rootNode.AddChild(ToString(_T("startpoints")), false))) {
            return false;
        }
    }
    if (!SaveWayPointList(rootNode.AddChild(ToString(_T("waypoints")), false))) {
        return false;
    }

    bool bSuccess = false;
    int ContentSize = 0;
    LPTSTR szContent = topNode.createXMLString(1, &ContentSize);
    Utf8File file;
    if (file.Open(szFileName, Utf8File::io_create)) {
        file.WriteLn(_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"));
        file.WriteLn(szContent);
        file.Close();

        bSuccess = true;
    }
    free(szContent);

    return bSuccess;
}

bool CTaskFileHelper::SaveOption(XMLNode node) {
    if (!node) {
        return false;
    }

    XMLNode OptNode = node.AddChild(ToString(_T("options")), false);
    if (!OptNode) {
        return false;
    }

    switch (AutoAdvance) {
        case 0:
            SetAttribute(OptNode, _T("auto-advance"), _T("Manual"));
            break;
        case 1:
            SetAttribute(OptNode, _T("auto-advance"), _T("Auto"));
            break;
        case 2:
            SetAttribute(OptNode, _T("auto-advance"), _T("Arm"));
            break;
        case 3:
            SetAttribute(OptNode, _T("auto-advance"), _T("ArmStart"));
            break;
        case 4:
            SetAttribute(OptNode, _T("auto-advance"), _T("ArmTPs"));
            break;
    }

    switch (gTaskType) {
        case TSK_AAT:
            // AAT Task
        SetAttribute(node, _T("type"), _T("AAT"));
        if (!SaveOptionAAT(OptNode)) {
            return false;
        }
            break;
        case TSK_GP: // Paraglider optimized Task
        SetAttribute(node, _T("type"), _T("Race"));
        if (!SaveOptionRace(OptNode)) {
            return false;
        }
            break;
        default: // default Task
        SetAttribute(node, _T("type"), _T("Default"));
        if (!SaveOptionDefault(OptNode)) {
            return false;
        }
            break;
    }

    if (!SaveTaskRule(OptNode.AddChild(ToString(_T("rules")), false))) {
        return false;
    }

    return true;
}

bool CTaskFileHelper::SaveOptionAAT(XMLNode node) {
    if (!node) {
        return false;
    }
    SetAttribute(node, _T("length"), AATTaskLength);

    return true;
}

bool CTaskFileHelper::SaveOptionRace(XMLNode node) {
    if (!node) {
        return false;
    }
    if (PGNumberOfGates > 0) {
        if (!SaveTimeGate(node.AddChild(ToString(_T("time-gate")), false))) {
            return false;
        }
    }

    return true;
}

bool CTaskFileHelper::SaveOptionDefault(XMLNode node) {
    if (!node) {
        return false;
    }

    XMLNode nodeStart = node.AddChild(ToString(_T("start")), false);
    if (nodeStart) {
        switch (StartLine) {
            case 0: //circle
                SetAttribute(nodeStart, _T("type"), _T("circle"));
                break;
            case 1: //line
                SetAttribute(nodeStart, _T("type"), _T("line"));
                break;
            case 2: //sector
                SetAttribute(nodeStart, _T("type"), _T("sector"));
                break;
        }
        SetAttribute(nodeStart, _T("radius"), StartRadius);
    } else {
        return false;
    }

    XMLNode nodeFinish = node.AddChild(ToString(_T("finish")), false);
    if (nodeFinish) {
        switch (FinishLine) {
            case 0: //circle
                SetAttribute(nodeFinish, _T("type"), _T("circle"));
                break;
            case 1: //line
                SetAttribute(nodeFinish, _T("type"), _T("line"));
                break;
            case 2: //sector
                SetAttribute(nodeFinish, _T("type"), _T("sector"));
                break;
        }
        SetAttribute(nodeFinish, _T("Radius"), FinishRadius);
    } else {
        return false;
    }

    XMLNode nodeSector = node.AddChild(ToString(_T("sector")), false);
    if (nodeSector) {
        switch (SectorType) {
            case CIRCLE: //circle
                SetAttribute(nodeSector, _T("type"), _T("circle"));
                SetAttribute(nodeSector, _T("Radius"), SectorRadius);
                break;
            case SECTOR: //sector
                SetAttribute(nodeSector, _T("type"), _T("sector"));
                SetAttribute(nodeSector, _T("Radius"), SectorRadius);
                break;
            case DAe: //DAe
                SetAttribute(nodeSector, _T("type"), _T("DAe"));
                break;
            case LINE: //line
                SetAttribute(nodeSector, _T("type"), _T("line"));
                SetAttribute(nodeSector, _T("Radius"), SectorRadius);
                break;
                
        }
    } else {
        return false;
    }

    return true;
}

bool CTaskFileHelper::SaveTimeGate(XMLNode node) {
    if (!node) {
        return false;
    }

    SetAttribute(node, _T("number"), PGNumberOfGates);

    if(!node.AddAttribute(ToString(_T("open-time")), AllocFormat(_T("%02d:%02d"), PGOpenTimeH, PGOpenTimeM))) { 
        return false; 
    }

	if(!node.AddAttribute(ToString(_T("close-time")), AllocFormat(_T("%02d:%02d"), PGCloseTimeH, PGCloseTimeM))) { 
        return false; 
    }
    
    SetAttribute(node, _T("interval-time"), PGGateIntervalTime);

    return true;
}

bool CTaskFileHelper::SaveTaskRule(XMLNode node) {
    if (!node) {
        return false;
    }

    XMLNode FinishNode = node.AddChild(ToString(_T("finish")), false);
    if (!FinishNode) {
        return false;
    }
    SetAttribute(FinishNode, _T("fai-height"), EnableFAIFinishHeight);
    SetAttribute(FinishNode, _T("min-height"), FinishMinHeight);

    XMLNode StartNode = node.AddChild(ToString(_T("start")), false);
    if (!StartNode) {
        return false;
    }
    SetAttribute(StartNode, _T("max-height"), StartMaxHeight);
    SetAttribute(StartNode, _T("max-height-margin"), StartMaxHeightMargin);
    SetAttribute(StartNode, _T("max-speed"), StartMaxSpeed);
    SetAttribute(StartNode, _T("max-speed-margin"), StartMaxSpeedMargin);
    switch (StartHeightRef) {
        case 0:
            SetAttribute(StartNode, _T("height-ref"), _T("AGL"));
            break;
        case 1:
            SetAttribute(StartNode, _T("height-ref"), _T("ASL"));
            break;
    }
    return true;
}

bool CTaskFileHelper::SaveTaskPointList(XMLNode node) {
    if (!node) {
        return false;
    }

    for (unsigned i = 0; ValidTaskPoint(i); ++i) {
        XMLNode PointNode = node.AddChild(ToString(_T("point")), false);
        if (!PointNode) {
            return false;
        }

        SetAttribute(PointNode, _T("idx"), i);

        RenameIfVirtual(i); // TODO: check code is unique ?

        if (!SaveTaskPoint(PointNode, i, Task[i])) {
            return false;
        }
    }
    return true;
}

bool CTaskFileHelper::SaveStartPointList(XMLNode node) {
    if (!node) {
        return false;
    }
    for (unsigned i = 0; ValidStartPoint(i); ++i) {
        XMLNode PointNode = node.AddChild(ToString(_T("point")), false);
        if (!PointNode) {
            return false;
        }

        SetAttribute(PointNode, _T("idx"), i);

        RenameIfVirtual(i); // TODO: check code is unique ?

        if (!SaveStartPoint(PointNode, StartPoints[i])) {
            return false;
        }
    }
    return true;
}

bool CTaskFileHelper::SaveWayPointList(XMLNode node) {
    for (std::set<size_t>::const_iterator it = mWayPointToSave.begin(); it != mWayPointToSave.end(); ++it) {
        if (!SaveWayPoint(node.AddChild(ToString(_T("point")), false), WayPointList[*it])) {
            return false;
        }
    }
    return true;
}

bool CTaskFileHelper::SaveTaskPoint(XMLNode node, const unsigned long idx, const TASK_POINT& TaskPt) {
    LKASSERT(ValidWayPoint(TaskPt.Index));
    SetAttribute(node, _T("name"), WayPointList[TaskPt.Index].Name);

    if (UseAATTarget()) {
        int Type; double Radius;
        GetTaskSectorParameter(idx, &Type, &Radius);
        switch (Type) {
            case CIRCLE:
                SetAttribute(node, _T("type"), _T("circle"));
                SetAttribute(node, _T("radius"), Radius);
                if (DoOptimizeRoute()) {
                    if(idx==0) {
                        SetAttribute(node, _T("Exit"), PGStartOut?_T("false"):_T("true"));
                    } else {
                        SetAttribute(node, _T("Exit"), TaskPt.OutCircle?_T("true"):_T("false"));
                    }
                }
                break;
            case SECTOR:
                SetAttribute(node, _T("type"), _T("sector"));
                SetAttribute(node, _T("radius"), Radius);
                SetAttribute(node, _T("start-radial"), TaskPt.AATStartRadial);
                SetAttribute(node, _T("end-radial"), TaskPt.AATFinishRadial);
                break;
            case LINE:
                SetAttribute(node, _T("type"), _T("line"));
                SetAttribute(node, _T("radius"), Radius);
                break;
            case DAe: // not Used in AAT and PGTask
                LKASSERT(false);
                break;
            case CONE:
                SetAttribute(node, _T("type"), _T("cone"));
                SetAttribute(node, _T("base"), TaskPt.PGConeBase);
                SetAttribute(node, _T("radius"), TaskPt.PGConeBaseRadius);
                SetAttribute(node, _T("slope"), TaskPt.PGConeSlope);
                break;
            case ESS_CIRCLE:
                SetAttribute(node, _T("type"), _T("ess_circle"));
                SetAttribute(node, _T("radius"), Radius);
                if (DoOptimizeRoute()) {
                    if(idx==0) {
                        SetAttribute(node, _T("Exit"), PGStartOut?_T("false"):_T("true"));
                    } else {
                        SetAttribute(node, _T("Exit"), TaskPt.OutCircle?_T("true"):_T("false"));
                    }
                }    
                break;
            default:
                LKASSERT(false);
                break;
        }

        if (gTaskType == TSK_AAT) {
            SetAttribute(node, _T("lock"), TaskPt.AATTargetLocked);
            if (TaskPt.AATTargetLocked) {
                SetAttribute(node, _T("target-lat"), TaskPt.AATTargetLat);
                SetAttribute(node, _T("target-lon"), TaskPt.AATTargetLon);
            }
            SetAttribute(node, _T("offset-radius"), TaskPt.AATTargetOffsetRadius);
            SetAttribute(node, _T("offset-radial"), TaskPt.AATTargetOffsetRadial);
        }
    }
    mWayPointToSave.insert(TaskPt.Index);
    return true;
}

bool CTaskFileHelper::SaveStartPoint(XMLNode node, const START_POINT& StartPt) {
    LKASSERT(ValidWayPoint(StartPt.Index));
    SetAttribute(node, _T("name"), (LPCTSTR)(WayPointList[StartPt.Index].Name));

    mWayPointToSave.insert(StartPt.Index);
    return true;
}

bool CTaskFileHelper::SaveWayPoint(XMLNode node, const WAYPOINT& WayPoint) {
    SetAttribute(node, _T("name"), WayPoint.Name);
    SetAttribute(node, _T("latitude"), WayPoint.Latitude);
    SetAttribute(node, _T("longitude"), WayPoint.Longitude);
    SetAttribute(node, _T("altitude"), WayPoint.Altitude);
    SetAttribute(node, _T("flags"), WayPoint.Flags);
    if (_tcslen(WayPoint.Code) > 0) {
        SetAttribute(node, _T("code"), (LPCTSTR)(WayPoint.Code));
    }
    if (WayPoint.Comment && _tcslen(WayPoint.Comment) > 0) {
        SetAttribute(node, _T("comment"), WayPoint.Comment);
    }
#if TASK_DETAILS
    if (WayPoint.Details && _tcslen(WayPoint.Details) > 0) {
        SetAttribute(node, _T("details"), WayPoint.Details);
    }
#endif
    SetAttribute(node, _T("format"), WayPoint.Format);
    if (_tcslen(WayPoint.Freq) > 0) {
        SetAttribute(node, _T("freq"), WayPoint.Freq);
    }
    if (WayPoint.RunwayLen > 0) {
        SetAttribute(node, _T("runwayLen"), WayPoint.RunwayLen);
    }
    if (WayPoint.RunwayLen > 0) {
        SetAttribute(node, _T("runwayDir"), WayPoint.RunwayDir);
    }
    if (_tcslen(WayPoint.Country) > 0) {
        SetAttribute(node, _T("country"), WayPoint.Country);
    }
    if (WayPoint.Style > 0) {
        SetAttribute(node, _T("style"), WayPoint.Style);
    }
    return true;
}
