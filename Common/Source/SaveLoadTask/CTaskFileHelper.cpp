/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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
#include "utils/openzip.h"

#include <fstream>
#include "Library/rapidxml/rapidxml.hpp"
#include "Library/rapidxml/rapidxml_print.hpp"


extern void RenameIfVirtual(const unsigned int i);
extern bool FullResetAsked;

namespace {

using xml_node = rapidxml::xml_node<char>;
using xml_attribute = rapidxml::xml_attribute<char>;
using xml_document = rapidxml::xml_document<char>;
using node_type = rapidxml::node_type;

char* AllocFormat(xml_document* doc, const char* fmt, ...) {
    assert(doc);
    if(doc == nullptr) {
        throw std::runtime_error("AllocFormat : 'doc' must be not null");
    }

    int size = 50; /* Guess we need no more than 100 bytes. */
    va_list ap;

    char* p = doc->allocate_string(nullptr, size);

    while (1) {
        /* Try to print in the allocated space. */
        va_start(ap, fmt);
        int n = vsnprintf(p, size, fmt, ap);
        va_end(ap);

        /* If that work, return the string. */
        if (n > -1 && n < size)
            return p;

        /* Else try again with more space. */
        if (n > -1) /* glibc 2.1 */
            size = n + 1; /* precisely what is needed */
        else /* glibc 2.0 */
            size *= 2; /* twice the old size */

        char* np = doc->allocate_string(nullptr, size);
        if (np == nullptr) {
            return nullptr;
        } else {
            p = np;
        }
    }
}

char* ToString(xml_document* doc, unsigned ulong) {
    return AllocFormat(doc, "%u", ulong);
}

char* ToString(xml_document* doc, const char* szString) {
    if(doc == nullptr) {
        throw std::runtime_error("ToString : 'doc' must be not null");
    }
    return doc->allocate_string(szString);
}

#ifdef UNICODE

char* ToString(xml_document* doc, const wchar_t* szString) {
    if(doc == nullptr) {
        throw std::runtime_error("ToString : 'doc' must be not null");
    }
    return doc->allocate_string(to_utf8(szString).c_str());
}

#endif

char* ToString(xml_document* doc, double dVal) {
    return AllocFormat(doc, "%f", dVal);
}

char* ToString(xml_document* doc, int iVal) {
    return AllocFormat(doc, "%d", iVal);
}

char* ToString(xml_document* doc, bool bVal) {
    if(doc == nullptr) {
        throw std::runtime_error("ToString : 'doc' must be not null");
    }
    return doc->allocate_string(bVal ? "true" : "false");
}

template<typename T>
void SetAttribute(xml_node* node, const char* name, const T val) {
    if(node) {
        xml_document* doc = node->document();
        char* string = ToString(doc, val);
        node->append_attribute(doc->allocate_attribute(name, string));
    } else {
        throw std::runtime_error("SetAttribute : node must be not null");
    }
}

xml_node* AddNode(xml_node* parent, const char* name) {
    if (parent) {
        xml_document* doc = parent->document();
        if(doc) {
            xml_node* node = doc->allocate_node(node_type::node_element, name);
            parent->append_node(node);
            return node;
        }
    }
    return nullptr;
}


void FromString(const char* szVal, unsigned& ulong) {
    ulong = szVal ? strtoul(szVal, nullptr, 10) : 0U;
}

template<size_t size>
void FromString(const char* szVal, TCHAR(&szString)[size]) {
    from_utf8(szVal, szString);
}

void FromString(const char* szVal, double& dVal) {
    dVal = szVal ? strtod(szVal, nullptr) : 0.;
}

void FromString(const char* szVal, int& iVal) {
    iVal = szVal ? strtol(szVal, nullptr, 10) : 0;
}

void FromString(const char* szVal, short& iVal) {
    iVal = szVal ? strtol(szVal, nullptr, 10) : 0;
}

void FromString(const char* szVal, bool& bVal) {
    bVal = (szVal && (strcmp(szVal, "true") == 0));
}

const char* GetAttribute(const xml_node* node, const char* name) {
    if(node) {
        xml_attribute* attribute = node->first_attribute(name, 0, false);
        if (attribute) {
            return attribute->value();
        }
    }
    return "";
}

template<typename T>
void GetAttribute(const xml_node* node, const char* name, T& val) {
    FromString(GetAttribute(node, name), val);
}


template<size_t size>
bool getFirstTaskWayPointName(const xml_node* node, TCHAR (&firstWPname)[size]) {
    if(node) {
        xml_node* first = node->first_node("point");
        if(first) {
            GetAttribute(first, "name", firstWPname);
            return true;
        }
    }
    return false;
}

template<size_t size>
bool getLastTaskWayPointName(const xml_node* node, TCHAR (&lastWPname)[size]) {
    if(node) {
        xml_node* first = node->first_node("point");
        if(first) {
            xml_node* last = node->last_node("point");
            if(first != last) {
                GetAttribute(last, "name", lastWPname);
                return true;
            }
        }
    }
    return false;
}

} // namespace


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

    try {

        zzip_file_ptr stream(openzip(taskFileName, "rt"));
        if (stream) {
            zzip_seek(stream, 0, SEEK_END); // seek to end of file
            long size = zzip_tell(stream); // get current file pointer
            zzip_seek(stream, 0, SEEK_SET); // seek back to beginning of file
            auto buff = std::make_unique<char[]>(size + 1);
            zzip_ssize_t nRead = zzip_read(stream, buff.get(), size);
            if (nRead != size) {
                return false;
            }
            buff[nRead] = '\0';

            constexpr int Flags = rapidxml::parse_trim_whitespace | rapidxml::parse_normalize_whitespace;

            xml_document xmldoc;
            xmldoc.parse<Flags>(buff.get());
            const xml_node* rootNode = xmldoc.first_node("lk-task");
            if (rootNode) {
                LoadOptions(rootNode);

                if(ISGAAIRCRAFT) {
                    TCHAR firstWPname[NAME_SIZE+1];
                    TCHAR lastWPname[NAME_SIZE+1];

                    const xml_node* taskNode = rootNode->first_node("taskpoints");

                    bool gotFirstWPname = getFirstTaskWayPointName(taskNode, firstWPname);
                    bool gotLastWPname = getLastTaskWayPointName(taskNode, lastWPname);

                    LoadWayPointList(rootNode->first_node("waypoints"), (gotFirstWPname?firstWPname:nullptr), (gotLastWPname?lastWPname:nullptr));
                } else {
                    LoadWayPointList(rootNode->first_node("waypoints"), nullptr, nullptr);
                }

                if (!LoadTaskPointList(rootNode->first_node("taskpoints"))) {
                    return false;
                }
                if (!LoadStartPointList(rootNode->first_node("startpoints"))) {
                    return false;
                }
            }
        }
    } catch (std::exception& e) {
        StartupStore(_T("LoadTask : %s"), to_tstring(e.what()).c_str());
        return false;
    }

    RefreshTask();

    TaskModified = false;
    TargetModified = false;
    // We are not using the DEMO.lkt forced by FULL RESET. We use the original task filename.
    _tcscpy(LastTaskFileName, szFileName);

    return true;
}

void CTaskFileHelper::LoadOptions(const xml_node* node) {
    if (node) {
        const xml_node* nodeOpt = node->first_node("options");
        if (nodeOpt) {
            const char* szAttr = GetAttribute(nodeOpt, "auto-advance");
            if (szAttr) {
                if (strcmp(szAttr, "Manual") == 0) {
                    AutoAdvance = 0;
                } else if (strcmp(szAttr, "Auto") == 0) {
                    AutoAdvance = 1;
                } else if (strcmp(szAttr, "Arm") == 0) {
                    AutoAdvance = 2;
                } else if (strcmp(szAttr, "ArmStart") == 0) {
                    AutoAdvance = 3;
                } else if (strcmp(szAttr, "ArmTPs") == 0) {
                    AutoAdvance = 4;
                }
            }

            const char* szType = GetAttribute(node, "type");
            if (szType) {
                if (strcmp(szType, "AAT") == 0) {
                    gTaskType = TSK_AAT;
                    LoadOptionAAT(nodeOpt);
                } else if (strcmp(szType, "Race") == 0) {
                    gTaskType = TSK_GP;
                    LoadOptionRace(nodeOpt);
                } else if (strcmp(szType, "Default") == 0) {
                    gTaskType = TSK_DEFAULT;
                    LoadOptionDefault(nodeOpt);
                }
            }
            LoadRules(nodeOpt->first_node("rules"));
        }
    }
}

void CTaskFileHelper::LoadOptionAAT(const xml_node* node) {
    if (node) {
        GetAttribute(node, "length", AATTaskLength);
    }
}

void CTaskFileHelper::LoadOptionRace(const xml_node* node) {
    if (node) {
        LoadTimeGate(node->first_node("time-gate"));
    }
    // SS Turnpoint
    // ESS Turnpoint
}

void CTaskFileHelper::LoadTimeGate(const xml_node* node) {
    if (node) {
        GetAttribute(node, "number", PGNumberOfGates);
        TCHAR szTime[50];
        GetAttribute(node, "open-time", szTime);
        StrToTime(szTime, &PGOpenTimeH, &PGOpenTimeM);
        GetAttribute(node, "close-time", szTime);
        StrToTime(szTime, &PGCloseTimeH, &PGCloseTimeM);
        GetAttribute(node, "interval-time", PGGateIntervalTime);
    } else {
        PGNumberOfGates = 0;
    }
    InitActiveGate();    
}

void CTaskFileHelper::LoadOptionDefault(const xml_node* node) {
    if (node) {
        const xml_node* nodeStart = node->first_node("start");
        if (nodeStart) {
            const char* szType = GetAttribute(nodeStart, "type");
            if (szType) {
                if (strcmp(szType, "circle") == 0) {
                    StartLine = 0;
                } else if (strcmp(szType, "line") == 0) {
                    StartLine = 1;
                } else if (strcmp(szType, "sector") == 0) {
                    StartLine = 2;
                }
            }
            GetAttribute(nodeStart, "radius", StartRadius);
        }

        const xml_node* nodeFinish = node->first_node("finish");
        if (nodeFinish) {
            const char* szType = GetAttribute(nodeStart, "type");
            if (szType) {
                if (strcmp(szType, "circle") == 0) {
                    FinishLine = 0;
                } else if (strcmp(szType, "line") == 0) {
                    FinishLine = 1;
                } else if (strcmp(szType, "sector") == 0) {
                    FinishLine = 2;
                }
            }
            GetAttribute(nodeFinish, "radius", FinishRadius);
        }

        const xml_node* nodeSector = node->first_node("sector");
        if (nodeSector) {
            const char* szType = GetAttribute(nodeSector, "type");
            if (strcmp(szType, "circle") == 0) {
                SectorType = CIRCLE;
            } else if (strcmp(szType, "sector") == 0) {
                SectorType = SECTOR;
            } else if (strcmp(szType, "DAe") == 0) {
                SectorType = DAe;
            } else if (strcmp(szType, "line") == 0) {
                SectorType = LINE;
            }
            GetAttribute(nodeSector, "radius", SectorRadius);
        }
    }
}

void CTaskFileHelper::LoadRules(const xml_node* node) {
    if (node) {
        const xml_node* nodeFinish = node->first_node("finish");
        if (nodeFinish) {
            GetAttribute(nodeFinish, "fai-height", EnableFAIFinishHeight);
            GetAttribute(nodeFinish, "min-height", FinishMinHeight);
        }
        const xml_node* nodeStart = node->first_node("start");
        if (nodeStart) {
            GetAttribute(nodeStart, "max-height", StartMaxHeight);
            GetAttribute(nodeStart, "max-height-margin", StartMaxHeightMargin);
            GetAttribute(nodeStart, "max-speed", StartMaxSpeed);
            GetAttribute(nodeStart, "max-speed-margin", StartMaxSpeedMargin);

            const char* szAttr = GetAttribute(nodeStart, "height-ref");
            if (szAttr) {
                if (strcmp(szAttr, "AGL") == 0) {
                    StartHeightRef = 0;
                } else if (strcmp(szAttr, "ASL") == 0) {
                    StartHeightRef = 1;
                }
            }
        }
    }
}

bool CTaskFileHelper::LoadTaskPointList(const xml_node* node) {
    mFinishIndex = 0;
    if (node) {
        const xml_node* nodePoint = node->first_node("point");
        while (nodePoint) {
            if (!LoadTaskPoint(nodePoint)) {
                return false;
            }
            nodePoint = nodePoint->next_sibling("point");
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

bool CTaskFileHelper::LoadStartPointList(const xml_node* node) {
    if (node) {
        const xml_node* nodePoint = node->first_node("point");
        while (nodePoint) {
            if (!LoadStartPoint(nodePoint)) {
                return false;
            }
            nodePoint = nodePoint->next_sibling("point");
        }
        EnableMultipleStartPoints = ValidStartPoint(0);
    }
    return true;
}

void CTaskFileHelper::LoadWayPointList(const xml_node* node, const TCHAR *firstWPname, const TCHAR *lastWPname) {
    if (node) {
        const xml_node* nodePoint = node->first_node("point");
        while (nodePoint) {
            LoadWayPoint(nodePoint, firstWPname, lastWPname);
            nodePoint = nodePoint->next_sibling("point");
        }
    }
}

bool CTaskFileHelper::LoadTaskPoint(const xml_node* node) {
    if (node) {
        unsigned idx = MAXTASKPOINTS;
        GetAttribute(node, "idx", idx);
        TCHAR szName[NAME_SIZE+1];
        GetAttribute(node, "name", szName);
        if (idx >= MAXTASKPOINTS) {
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

        const char* szType = GetAttribute(node, "type");
        if (szType) {
            if (strcmp(szType, "circle") == 0) {
                Task[idx].AATType = CIRCLE;
                GetAttribute(node, "radius", Task[idx].AATCircleRadius);
                GetAttribute(node, "Exit", Task[idx].OutCircle);
            } else if (strcmp(szType, "sector") == 0) {
                Task[idx].AATType = SECTOR;
                GetAttribute(node, "radius", Task[idx].AATSectorRadius);
                GetAttribute(node, "start-radial", Task[idx].AATStartRadial);
                GetAttribute(node, "end-radial", Task[idx].AATFinishRadial);
            } else if (strcmp(szType, "line") == 0) {
                Task[idx].AATType = LINE;
                GetAttribute(node, "radius", Task[idx].AATCircleRadius);
            } else if (strcmp(szType, "DAe") == 0) {
                Task[idx].AATType = DAe; // not Used in AAT and PGTask
            } else if (strcmp(szType, "cone") == 0) {
                Task[idx].AATType = CONE; // Only Used in PGTask
                GetAttribute(node, "base", Task[idx].PGConeBase);
                GetAttribute(node, "radius", Task[idx].PGConeBaseRadius);
                GetAttribute(node, "slope", Task[idx].PGConeSlope);
                Task[idx].OutCircle = false;
            } else if (strcmp(szType, "ess_circle") == 0) {
                Task[idx].AATType = ESS_CIRCLE;
                GetAttribute(node, "radius", Task[idx].AATCircleRadius);
                GetAttribute(node, "Exit", Task[idx].OutCircle);
            }
        }
        GetAttribute(node, "lock", Task[idx].AATTargetLocked);
        GetAttribute(node, "target-lat", Task[idx].AATTargetLat);
        GetAttribute(node, "target-lon", Task[idx].AATTargetLon);
        GetAttribute(node, "offset-radius", Task[idx].AATTargetOffsetRadius);
        GetAttribute(node, "offset-radial", Task[idx].AATTargetOffsetRadial);
    }
    return true;
}

bool CTaskFileHelper::LoadStartPoint(const xml_node* node) {
    if (node) {
        unsigned idx = MAXSTARTPOINTS;
        GetAttribute(node, "idx", idx);
        TCHAR szName[NAME_SIZE+1];
        GetAttribute(node, "name", szName);

        if (idx >= MAXSTARTPOINTS) {
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

void CTaskFileHelper::LoadWayPoint(const xml_node* node, const TCHAR *firstWPname, const TCHAR *lastWPname) {
    WAYPOINT newPoint;
    memset(&newPoint, 0, sizeof (newPoint));

    GetAttribute(node, "code", newPoint.Code);
    GetAttribute(node, "name", newPoint.Name);

    bool lookupAirfield=false;
    if(ISGAAIRCRAFT) {
        if(firstWPname) {
            if(_tcscmp(newPoint.Name,firstWPname)==0) lookupAirfield=true;
        }
        if(lastWPname && !lookupAirfield) {
            if(_tcscmp(newPoint.Name,lastWPname)==0) lookupAirfield=true;
        }
    }
    GetAttribute(node, "latitude", newPoint.Latitude);
    GetAttribute(node, "longitude", newPoint.Longitude);
    GetAttribute(node, "altitude", newPoint.Altitude);
    GetAttribute(node, "flags", newPoint.Flags);

    const char* comment = GetAttribute(node, "comment");
    SetWaypointComment(newPoint, utf8_to_tstring(comment).c_str());

    const char* details = GetAttribute(node, "details");
    SetWaypointDetails(newPoint, utf8_to_tstring(details).c_str());

    GetAttribute(node, "format", newPoint.Format);
    GetAttribute(node, "freq", newPoint.Freq);
    GetAttribute(node, "runwayLen", newPoint.RunwayLen);
    GetAttribute(node, "runwayDir", newPoint.RunwayDir);
    GetAttribute(node, "country", newPoint.Country);
    GetAttribute(node, "style", newPoint.Style);

    // NOTICE: we must consider that FindOrAdd can return -1
    int ix = FindOrAddWaypoint(&newPoint,lookupAirfield);
    if (ix < 0)  { // -1 actually
        StartupStore(_T("... Failed task LoadWaypoint <%s>, not loaded.\n"),newPoint.Name);
        return;
    }
    mWayPointLoaded[newPoint.Name] = ix;

    if (newPoint.Details) {
        free(newPoint.Details);
        newPoint.Details = nullptr;
    }

    if (newPoint.Comment) {
        free(newPoint.Comment);
        newPoint.Comment = nullptr;
    }
}

bool CTaskFileHelper::Save(const TCHAR* szFileName) {
    if (WayPointList.empty()) return false; // this should never happen, but just to be safe...

    CScopeLock LockTask(LockTaskData, UnlockTaskData);
    StartupStore(_T(". SaveTask : saving <%s>%s"), szFileName, NEWLINE);
    
    try {
        xml_document doc;
        xml_node* decl = doc.allocate_node(node_type::node_declaration);
        decl->append_attribute(doc.allocate_attribute("version", "1.0"));
        decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
        doc.append_node(decl);

        xml_node* rootNode = doc.allocate_node(node_type::node_element, "lk-task");
        doc.append_node(rootNode);

        if (!SaveOption(rootNode)) {
            return false;
        }



        if (!SaveTaskPointList(AddNode(rootNode, "taskpoints"))) {
            return false;
        }
        if (EnableMultipleStartPoints && ValidStartPoint(0)) {
            if (!SaveStartPointList(AddNode(rootNode, "startpoints"))) {
                return false;
            }
        }

        if (!SaveWayPointList(AddNode(rootNode, "waypoints"))) {
            return false;
        }

        std::ofstream file_out(szFileName);
        rapidxml::print(std::ostream_iterator<char>(file_out), doc);
        file_out.close();
    } catch (std::exception& e) {
        StartupStore(_T("SaveTask : %s"), to_tstring(e.what()).c_str());
        return false;
    }
    return true;
}

bool CTaskFileHelper::SaveOption(xml_node* node) {
    if (!node) {
        return false;
    }

    xml_node* OptNode = AddNode(node, "options");
    switch (AutoAdvance) {
        case 0:
            SetAttribute(OptNode, "auto-advance", "Manual");
            break;
        case 1:
            SetAttribute(OptNode, "auto-advance", "Auto");
            break;
        case 2:
            SetAttribute(OptNode, "auto-advance", "Arm");
            break;
        case 3:
            SetAttribute(OptNode, "auto-advance", "ArmStart");
            break;
        case 4:
            SetAttribute(OptNode, "auto-advance", "ArmTPs");
            break;
    }

    switch (gTaskType) {
        case TSK_AAT:
            // AAT Task
        SetAttribute(node, "type", "AAT");
        if (!SaveOptionAAT(OptNode)) {
            return false;
        }
            break;
        case TSK_GP: // Paraglider optimized Task
        SetAttribute(node, "type", "Race");
        if (!SaveOptionRace(OptNode)) {
            return false;
        }
            break;
        default: // default Task
        SetAttribute(node, "type", "Default");
        if (!SaveOptionDefault(OptNode)) {
            return false;
        }
            break;
    }

    if (!SaveTaskRule(AddNode(node, "rules"))) {
        return false;
    }

    return true;
}

bool CTaskFileHelper::SaveOptionAAT(xml_node* node) {
    if (!node) {
        return false;
    }
    SetAttribute(node, "length", AATTaskLength);

    return true;
}

bool CTaskFileHelper::SaveOptionRace(xml_node* node) {
    if (!node) {
        return false;
    }
    if (PGNumberOfGates > 0) {
        if (!SaveTimeGate(AddNode(node, "time-gate"))) {
            return false;
        }
    }

    return true;
}

bool CTaskFileHelper::SaveOptionDefault(xml_node* node) {
    if (!node) {
        return false;
    }

    xml_node* nodeStart = AddNode(node, "start");
    if (nodeStart) {
        switch (StartLine) {
            case 0: //circle
                SetAttribute(nodeStart, "type", "circle");
                break;
            case 1: //line
                SetAttribute(nodeStart, "type", "line");
                break;
            case 2: //sector
                SetAttribute(nodeStart, "type", "sector");
                break;
        }
        SetAttribute(nodeStart, "radius", StartRadius);
    } else {
        return false;
    }

    xml_node* nodeFinish = AddNode(node, "finish");
    if (nodeFinish) {
        switch (FinishLine) {
            case 0: //circle
                SetAttribute(nodeFinish, "type", "circle");
                break;
            case 1: //line
                SetAttribute(nodeFinish, "type", "line");
                break;
            case 2: //sector
                SetAttribute(nodeFinish, "type", "sector");
                break;
        }
        SetAttribute(nodeFinish, "radius", FinishRadius);
    } else {
        return false;
    }

    xml_node* nodeSector = AddNode(node, "sector");
    if (nodeSector) {
        switch (SectorType) {
            case CIRCLE: //circle
                SetAttribute(nodeSector, "type", "circle");
                SetAttribute(nodeSector, "radius", SectorRadius);
                break;
            case SECTOR: //sector
                SetAttribute(nodeSector, "type", "sector");
                SetAttribute(nodeSector, "radius", SectorRadius);
                break;
            case DAe: //DAe
                SetAttribute(nodeSector, "type", "DAe");
                break;
            case LINE: //line
                SetAttribute(nodeSector, "type", "line");
                SetAttribute(nodeSector, "radius", SectorRadius);
                break;
                
        }
    } else {
        return false;
    }

    return true;
}

bool CTaskFileHelper::SaveTimeGate(xml_node* node) {
    if (!node) {
        return false;
    }

    SetAttribute(node, "number", PGNumberOfGates);

    xml_document* doc = node->document();

    char* open = AllocFormat(doc, "%02d:%02d", PGOpenTimeH, PGOpenTimeM);
    SetAttribute(node, "open-time", open);

    char* close = AllocFormat(doc, "%02d:%02d", PGCloseTimeH, PGCloseTimeM);
    SetAttribute(node, "close-time", close);
    
    SetAttribute(node, "interval-time", PGGateIntervalTime);

    return true;
}

bool CTaskFileHelper::SaveTaskRule(xml_node* node) {
    if (!node) {
        return false;
    }

    xml_node* FinishNode = AddNode(node, "finish");
    SetAttribute(FinishNode, "fai-height", EnableFAIFinishHeight);
    SetAttribute(FinishNode, "min-height", FinishMinHeight);

    xml_node* StartNode = AddNode(node, "start");
    SetAttribute(StartNode, "max-height", StartMaxHeight);
    SetAttribute(StartNode, "max-height-margin", StartMaxHeightMargin);
    SetAttribute(StartNode, "max-speed", StartMaxSpeed);
    SetAttribute(StartNode, "max-speed-margin", StartMaxSpeedMargin);
    switch (StartHeightRef) {
        case 0:
            SetAttribute(StartNode, "height-ref", "AGL");
            break;
        case 1:
            SetAttribute(StartNode, "height-ref", "ASL");
            break;
    }
    return true;
}

bool CTaskFileHelper::SaveTaskPointList(xml_node* node) {
    if (!node) {
        return false;
    }

    for (unsigned i = 0; ValidTaskPoint(i); ++i) {
        xml_node* PointNode = AddNode(node, "point");
        SetAttribute(PointNode, "idx", i);

        RenameIfVirtual(i); // TODO: check code is unique ?

        if (!SaveTaskPoint(PointNode, i, Task[i])) {
            return false;
        }
    }
    return true;
}

bool CTaskFileHelper::SaveStartPointList(xml_node* node) {
    if (!node) {
        return false;
    }
    for (unsigned i = 0; ValidStartPoint(i); ++i) {
        xml_node* PointNode = AddNode(node, "point");
        SetAttribute(PointNode, "idx", i);

        RenameIfVirtual(i); // TODO: check code is unique ?

        if (!SaveStartPoint(PointNode, StartPoints[i])) {
            return false;
        }
    }
    return true;
}

bool CTaskFileHelper::SaveWayPointList(xml_node* node) {
    for (std::set<size_t>::const_iterator it = mWayPointToSave.begin(); it != mWayPointToSave.end(); ++it) {
        if (!SaveWayPoint(AddNode(node, "point"), WayPointList[*it])) {
            return false;
        }
    }
    return true;
}

bool CTaskFileHelper::SaveTaskPoint(xml_node* node, const unsigned long idx, const TASK_POINT& TaskPt) {
    LKASSERT(ValidWayPoint(TaskPt.Index));
    SetAttribute(node, "name", WayPointList[TaskPt.Index].Name);

    if (UseAATTarget()) {
        int Type; double Radius;
        GetTaskSectorParameter(idx, &Type, &Radius);
        switch (Type) {
            case CIRCLE:
                SetAttribute(node, "type", "circle");
                SetAttribute(node, "radius", Radius);
                if (DoOptimizeRoute()) {
                    if(idx==0) {
                        SetAttribute(node, "Exit", PGStartOut?_T("false"):_T("true"));
                    } else {
                        SetAttribute(node, "Exit", TaskPt.OutCircle?_T("true"):_T("false"));
                    }
                }
                break;
            case SECTOR:
                SetAttribute(node, "type", "sector");
                SetAttribute(node, "radius", Radius);
                SetAttribute(node, "start-radial", TaskPt.AATStartRadial);
                SetAttribute(node, "end-radial", TaskPt.AATFinishRadial);
                break;
            case LINE:
                SetAttribute(node, "type", "line");
                SetAttribute(node, "radius", Radius);
                break;
            case DAe: // not Used in AAT and PGTask
                LKASSERT(false);
                break;
            case CONE:
                SetAttribute(node, "type", "cone");
                SetAttribute(node, "base", TaskPt.PGConeBase);
                SetAttribute(node, "radius", TaskPt.PGConeBaseRadius);
                SetAttribute(node, "slope", TaskPt.PGConeSlope);
                break;
            case ESS_CIRCLE:
                SetAttribute(node, "type", "ess_circle");
                SetAttribute(node, "radius", Radius);
                if (DoOptimizeRoute()) {
                    if(idx==0) {
                        SetAttribute(node, "Exit", PGStartOut?_T("false"):_T("true"));
                    } else {
                        SetAttribute(node, "Exit", TaskPt.OutCircle?_T("true"):_T("false"));
                    }
                }    
                break;
            default:
                LKASSERT(false);
                break;
        }

        if (gTaskType == TSK_AAT) {
            SetAttribute(node, "lock", TaskPt.AATTargetLocked);
            if (TaskPt.AATTargetLocked) {
                SetAttribute(node, "target-lat", TaskPt.AATTargetLat);
                SetAttribute(node, "target-lon", TaskPt.AATTargetLon);
            }
            SetAttribute(node, "offset-radius", TaskPt.AATTargetOffsetRadius);
            SetAttribute(node, "offset-radial", TaskPt.AATTargetOffsetRadial);
        }
    }
    mWayPointToSave.insert(TaskPt.Index);
    return true;
}

bool CTaskFileHelper::SaveStartPoint(xml_node* node, const START_POINT& StartPt) {
    LKASSERT(ValidWayPoint(StartPt.Index));
    SetAttribute(node, "name", (LPCTSTR)(WayPointList[StartPt.Index].Name));

    mWayPointToSave.insert(StartPt.Index);
    return true;
}

bool CTaskFileHelper::SaveWayPoint(xml_node* node, const WAYPOINT& WayPoint) {
    SetAttribute(node, "name", WayPoint.Name);
    SetAttribute(node, "latitude", WayPoint.Latitude);
    SetAttribute(node, "longitude", WayPoint.Longitude);
    SetAttribute(node, "altitude", WayPoint.Altitude);
    SetAttribute(node, "flags", WayPoint.Flags);
    if (_tcslen(WayPoint.Code) > 0) {
        SetAttribute(node, "code", (LPCTSTR)(WayPoint.Code));
    }
    if (WayPoint.Comment && _tcslen(WayPoint.Comment) > 0) {
        SetAttribute(node, "comment", WayPoint.Comment);
    }
    if (WayPoint.Details && _tcslen(WayPoint.Details) > 0) {
        SetAttribute(node, "details", WayPoint.Details);
    }
    SetAttribute(node, "format", WayPoint.Format);
    if (_tcslen(WayPoint.Freq) > 0) {
        SetAttribute(node, "freq", WayPoint.Freq);
    }
    if (WayPoint.RunwayLen > 0) {
        SetAttribute(node, "runwayLen", WayPoint.RunwayLen);
    }
    if (WayPoint.RunwayLen > 0) {
        SetAttribute(node, "runwayDir", WayPoint.RunwayDir);
    }
    if (_tcslen(WayPoint.Country) > 0) {
        SetAttribute(node, "country", WayPoint.Country);
    }
    if (WayPoint.Style > 0) {
        SetAttribute(node, "style", WayPoint.Style);
    }
    return true;
}
