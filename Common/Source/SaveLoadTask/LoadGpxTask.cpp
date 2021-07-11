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
#include "utils/openzip.h"
#include "Util/ScopeExit.hxx"
#include "Library/rapidxml/rapidxml.hpp"
// #define TASK_DETAILS

using xml_document = rapidxml::xml_document<char>;
using xml_node = rapidxml::xml_node<char>;
using xml_attribute = rapidxml::xml_attribute<char>;

bool LoadGpxTask(LPCTSTR szFileName) {
    LockTaskData();
    AtScopeExit() {
        UnlockTaskData();
    };

    StartupStore(_T(". LoadGpxTask : <%s>"), szFileName);
    ClearTask();

    zzip_file_ptr stream(openzip(szFileName, "rt"));
    if(stream) {

        zzip_off_t size = zzip_seek(stream, 0, SEEK_END); // seek to end of file and get file size
        if(size <= 0) {
            return false;
        }
        zzip_seek(stream, 0, SEEK_SET); // seek back to beginning of file

        std::unique_ptr<char[]> buff(new (std::nothrow) char[size+1]);
        if(!buff) {
            return false;
        }

        // Read the file
        zzip_ssize_t nRead = zzip_read(stream, buff.get(), size);
        // fread can return -1...
        if (nRead < 0) {
            StartupStore(_T(". LoadGpxTask, fread failure!"));
            return false;
        }
        if(nRead != size) {
            StartupStore(TEXT(".. Not able to buffer."));
            return false;
        }
        buff[nRead]= '\0';

        xml_document xmldoc;
        try {
            constexpr int Flags = rapidxml::parse_trim_whitespace | rapidxml::parse_normalize_whitespace;
            xmldoc.parse<Flags>(buff.get());
        } catch (rapidxml::parse_error& e) {
            StartupStore(TEXT(".. GPX parse failed : %s"), to_tstring(e.what()).c_str());
            return false;
        }

        xml_node* rootNode = xmldoc.first_node("gpx");
        if(rootNode) {
            //TODO: here we load just the first route may be there are others routes in the GPX file...
            xml_node* routeNode = rootNode->first_node("rte");
            if(!routeNode) { //ERROR no route found in GPX file
                return false;
            }

            WAYPOINT newPoint;
            int idx = 0;
            xml_node* WPnode = routeNode->first_node("rtept");
            if(WPnode) do {
                memset(&newPoint, 0, sizeof(newPoint));

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
                    size_t len = strlen(comment->value())+1;
                    newPoint.Comment = (TCHAR*) malloc(len * sizeof(TCHAR));
                    from_utf8(comment->value(), newPoint.Comment, len);
                }

#ifdef TASK_DETAILS
                xml_node* detail = WPnode->first_node("desc");
                if(detail && detail->value()) {
                    size_t len = strlen(detail->value())+1;
                    newPoint.Details = (TCHAR*) malloc(len * sizeof(TCHAR));
                    from_utf8(detail->value(), newPoint.Details, len);
                }
#else
                newPoint.Details = nullptr;
#endif

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

#ifdef TASK_DETAILS
                if (newPoint.Details) {
                    free(newPoint.Details);
                }
#endif

                if (newPoint.Comment) {
                    free(newPoint.Comment);
                }
            } while(WPnode); //for(each node in rtept)
        } //if(rootNode)
    } //if(stream)
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
    _tcscpy(LastTaskFileName, szFileName);
    return true;
}
