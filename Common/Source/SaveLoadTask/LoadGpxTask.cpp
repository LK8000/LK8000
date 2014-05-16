/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LoadGpxTask.cpp
 * Author: Alberto Realis-Luc
 *
 * Created on 20 December 2013, 10:43
 */

#include "externs.h"
#include "Waypointparser.h"
#include "xmlParser.h"
#include "utils/stringext.h"
#include <string>

bool LoadGpxTask(LPCTSTR szFileName) {
    LockTaskData();
    StartupStore(_T(". LoadTask : <%s>%s"), szFileName, NEWLINE);
    ClearTask();
    FILE* stream = _tfopen(szFileName, TEXT("rb"));
    if(stream) {
        fseek(stream, 0, SEEK_END); // seek to end of file
        long size = ftell(stream); // get current file pointer
        fseek(stream, 0, SEEK_SET); // seek back to beginning of file
        char * buff = (char*) calloc(size + 1, sizeof (char));
        long nRead = fread(buff, sizeof (char), size, stream);
        if(nRead != size) {
            fclose(stream);
            free(buff);
            UnlockTaskData();
            return false;
        }
        fclose(stream);
        TCHAR * szXML = (TCHAR*) calloc(size + 1, sizeof (TCHAR));
        utf2TCHAR(buff, szXML, size + 1);
        free(buff);
        XMLNode rootNode = XMLNode::parseString(szXML, _T("gpx"));
        if(rootNode) {
            if(rootNode.isEmpty()) {
                free(szXML);
                UnlockTaskData();
                return false;
            }
            //TODO: here we load just the first route may be there are others routes in the GPX file...
            XMLNode routeNode=rootNode.getChildNode(TEXT("rte"));
            if(routeNode.isEmpty()) { //ERROR no route found in GPX file
                free(szXML);
                UnlockTaskData();
                return false;
            }
            int numWPnodes=routeNode.nChildNode(); //count number of XML nodes inside <rte> </rte>
            int numOfWPs=routeNode.nChildNode(TEXT("rtept")); //count number of WPs in the route
            if(numOfWPs<1 || numOfWPs>MAXTASKPOINTS) { //ERROR: no WPs at all or too many WPs found in route in GPX file
                free(szXML);
                UnlockTaskData();
                return false;
            }
            LPCTSTR dataStr=NULL;
            double lat;
            XMLNode WPnode,detailNode;
            WAYPOINT newPoint;
            for(int i=0,idx=0;i<numWPnodes;i++) {
                memset(&newPoint, 0, sizeof (newPoint));
                WPnode=routeNode.getChildNode(i);
                if(_tcscmp(WPnode.getName(),TEXT("rtept"))==0) {
                    dataStr=WPnode.getAttribute(TEXT("lat"));
                    if(!dataStr) { //ERROR: WP without latitude
                        free(szXML);
                        ClearTask();
                        UnlockTaskData();
                        return false;
                    }
                    lat=_tcstod(dataStr,NULL);
                    dataStr=WPnode.getAttribute(TEXT("lon"));
                    if(!dataStr) { //ERROR: WP without longitude
                        free(szXML);
                        ClearTask();
                        UnlockTaskData();
                        return false;
                    }
                    memset(&newPoint, 0, sizeof (newPoint));
                    newPoint.Latitude=lat;
                    newPoint.Longitude=_tcstod(dataStr,NULL);
                    detailNode=WPnode.getChildNode(TEXT("ele"),0);
                    if(detailNode) {
                        dataStr=detailNode.getText(0);
                        if(dataStr) newPoint.Altitude=_tcstod(dataStr,NULL);
                    }
                    detailNode=WPnode.getChildNode(TEXT("name"),0);
                    if(detailNode) {
                        dataStr=detailNode.getText(0);
                        if(dataStr) _tcscpy(newPoint.Name, dataStr);
                    }
                    detailNode=WPnode.getChildNode(TEXT("cmt"),0);
                    if(detailNode) {
                        dataStr=detailNode.getText(0);
                        if(dataStr) {
                            newPoint.Comment = (TCHAR*) malloc((_tcslen(dataStr)+1)*sizeof(TCHAR));
                            if(newPoint.Comment) LK_tcsncpy(newPoint.Comment, dataStr,COMMENT_SIZE);
                        }
                    }
#ifdef TASK_DETAILS
                    detailNode=WPnode.getChildNode(TEXT("desc"),0);
                    if(detailNode) {
                        dataStr=detailNode.getText(0);
                        if(dataStr) {
                            newPoint.Details = (TCHAR*) malloc((_tcslen(dataStr)+1)*sizeof(TCHAR));
                            if(newPoint.Details) _tcscpy(newPoint.Details, dataStr);
                        }
                    }
#else
                    newPoint.Details=NULL;
#endif
                    /*TODO: other possible data to get somehow from the GPX file:
                    newPoint.Code
                    newPoint.Flags
                    newPoint.Format
                    newPoint.Freq
                    newPoint.RunwayLen
                    newPoint.RunwayDir
                    newPoint.Country
                    newPoint.Style */
                    if(ISGAAIRCRAFT && (idx==0 || idx==numOfWPs-1)) {
                        int ix =FindOrAddWaypoint(&newPoint,true); 
                        //if GA check widely if we have already depart and dest airports
                        if (ix>=0) Task[idx++].Index=ix;
                    } else {
                        int ix =FindOrAddWaypoint(&newPoint,false); 
                        if (ix>=0) Task[idx++].Index=ix; //else add WP normally
                    }
#ifdef TASK_DETAILS
                    if (newPoint.Details) {
                        free(newPoint.Details);
                    }
#endif
                    if (newPoint.Comment) {
                        free(newPoint.Comment);
                    }
                } //if(rtept)
            } //for(each node in rtept)
        } //if(rootNode)
        free(szXML);
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
    AATEnabled=false;
    PGOptimizeRoute=false;
    StartHeightRef=1; //ASL
    RefreshTask();
    TaskModified = false;
    TargetModified = false;
    _tcscpy(LastTaskFileName, szFileName);
    UnlockTaskData();
    return true;
}
