/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LoadGpxTask.cpp
 * Author: Alberto Realis-Luc
 *
 * Created on 20 December 2013, 10:43
 */

#include "externs.h"
#include "Waypointparser.h"
#include "utils/stringext.h"
#include "LKStyle.h"
#include "utils/zzip_stream.h"
#include "Library/rapidxml/rapidxml.hpp"

using xml_document = rapidxml::xml_document<char>;
using xml_node = rapidxml::xml_node<char>;
using xml_attribute = rapidxml::xml_attribute<char>;

bool LoadGpxTask(std::istream& stream) {

    std::string ss;
    xml_document xmldoc;
    try {
        std::istreambuf_iterator<char> it(stream), end;
        ss.assign(it, end);

        constexpr int Flags = rapidxml::parse_trim_whitespace | rapidxml::parse_normalize_whitespace;
        xmldoc.parse<Flags>(ss.data());
    } catch (rapidxml::parse_error& e) {
        StartupStore(TEXT(".. GPX parse failed : %s"), to_tstring(e.what()).c_str());
        return false;
    }

    xml_node* rootNode = xmldoc.first_node("gpx");
    if(!rootNode) {
        return false;
    }

    //TODO: here we load just the first route may be there are others routes in the GPX file...
    xml_node* routeNode = rootNode->first_node("rte");
    if(!routeNode) { //ERROR no route found in GPX file
        return false;
    }

    ScopeLock lock(CritSec_TaskData);

    ClearTask();

    int idx = 0;
    xml_node* WPnode = routeNode->first_node("rtept");
    if(WPnode) do {
        WAYPOINT newPoint = {};

        xml_attribute* lat = WPnode->first_attribute("lat");
        if(lat && lat->value()) {
            newPoint.Latitude = strtod(lat->value(), nullptr);
        } else {
            ClearTask();
            return false;
        }

        xml_attribute* lon = WPnode->first_attribute("lon");
        if(lon && lon->value()) {
            newPoint.Longitude = strtod(lon->value(), nullptr);
        } else {
            ClearTask();
            return false;
        }

        xml_node* ele = WPnode->first_node("ele");
        if(ele && ele->value()) {
            newPoint.Altitude = strtod(ele->value(), nullptr);
        }

        xml_node* name = WPnode->first_node("name");
        if(name && name->value()) {
            from_utf8(name->value(), newPoint.Name);
        }

        xml_node* comment = WPnode->first_node("cmt");
        if(comment && comment->value()) {
            size_t size = from_utf8(comment->value(), newPoint.Comment, 0) + 1;
            newPoint.Comment = (TCHAR*) malloc(size * sizeof(TCHAR));
            from_utf8(comment->value(), newPoint.Comment, size);
        }

        xml_node* detail = WPnode->first_node("desc");
        if(detail && detail->value()) {
            size_t size = from_utf8(detail->value(), newPoint.Details, 0) + 1;
            newPoint.Details = (TCHAR*) malloc(size * sizeof(TCHAR));
            from_utf8(detail->value(), newPoint.Details, size);
        }

        WPnode = WPnode->next_sibling("rtept");

        newPoint.Format=LKW_GPX;
        newPoint.Style=STYLE_NORMAL;

        if (idx==0) {
            newPoint.Flags = START;
        } else if (!WPnode) {
            newPoint.Flags = FINISH;
        } else {
            newPoint.Flags = TURNPOINT + WAYPOINTFLAG;
        }

        int ix =FindOrAddWaypoint(&newPoint,ISGAAIRCRAFT && (idx==0 || !WPnode)); //if GA check widely if we have already depart and dest airports
        if (ix>=0) {
            Task[idx++].Index=ix;
        }

        if (newPoint.Details) {
            free(newPoint.Details);
        }

        if (newPoint.Comment) {
            free(newPoint.Comment);
        }
    } while(WPnode); //for(each node in rtept)

    if(ISGAAIRCRAFT) { //Set task options for GA aircraft
        StartLine=1; //Line
        StartRadius=1000;
        SectorType=LINE;
        SectorRadius=1000;
        FinishLine=0; //Circle
        FinishRadius=500;
    } else { //otherwise set default task options for other categories
        StartLine=2; //Sector
        StartRadius=1500;
        SectorType=CIRCLE;
        SectorRadius=2000;
        FinishLine=0; //Circle
        FinishRadius=3000;
    }
    AutoAdvance=1; //Auto
    gTaskType=TSK_DEFAULT;
    TskOptimizeRoute=false;
    StartHeightRef=1; //ASL
    RefreshTask();
    TaskModified = false;
    TargetModified = false;
    return true;
}

bool LoadGpxTask(LPCTSTR szFileName) {
    StartupStore(_T(". LoadGpxTask : <%s>"), szFileName);

    zzip_stream file(szFileName, "rt");
    if (file) {
        std::istream stream(&file);
        if (LoadGpxTask(stream)) {
            _tcscpy(LastTaskFileName, szFileName);
            return true;
        }
    }
    return false;
}
