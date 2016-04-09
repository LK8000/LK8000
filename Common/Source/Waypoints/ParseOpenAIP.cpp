/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   ParseOpenAIP.cpp
   Created on: Apr 7, 2016
   Author: Alberto Realis-Luc
*/

#include "externs.h"
#include "Waypointparser.h"
#include "utils/stringext.h"
#include "xmlParser.h"

extern int globalFileNum;

bool ParseAirports(ZZIP_FILE *fp, XMLNode &airportsNode);
bool ParseNavAids(ZZIP_FILE *fp, XMLNode &navAidsNode);
bool ParseHotSpots(ZZIP_FILE *fp, XMLNode &hotSpotsNode);

bool ParseOpenAIP(ZZIP_FILE *fp)
{
    // Allocate buffer to read the file
    zzip_seek(fp, 0, SEEK_END); // seek to end of file
    long size = zzip_tell(fp); // get current file pointer
    zzip_seek(fp, 0, SEEK_SET); // seek back to beginning of file
    char* buff = (char*) calloc(size + 1, sizeof(char));
    if(buff==nullptr) {
        StartupStore(TEXT(".. Failed to allocate buffer to read OpenAIP waypoints file.%s"), NEWLINE);
        return false;
    }

    // Read the file
    long nRead = zzip_fread(buff, sizeof (char), size, fp);
    if(nRead != size) {
        StartupStore(TEXT(".. Not able to buffer all airspace file.%s"), NEWLINE);
        free(buff);
        return false;
    }

    // Convert from UTF8
    TCHAR* szXML = (TCHAR*) calloc(size + 1, sizeof (TCHAR));
    if(szXML==nullptr) {
        StartupStore(TEXT(".. Not able to allocate memory to convert from UTF8.%s"), NEWLINE);
        free(buff);
        return false;
    }
    utf2TCHAR(buff, szXML, size + 1);
    free(buff);

    // Get 'root' node OPENAIP
    XMLNode rootNode = XMLNode::parseString(szXML, _T("OPENAIP"));
    free(szXML);
    if(rootNode.isEmpty()) {
        StartupStore(TEXT(".. OPENAIP tag not found.%s"), NEWLINE);
        return false;
    }

    // Check version of OpenAIP format
    LPCTSTR dataStr=rootNode.getAttribute(TEXT("DATAFORMAT"));
    if(dataStr==nullptr || _tcstod(dataStr,nullptr) != 1.1) {
        StartupStore(TEXT(".. DATAFORMAT attribute missing or not at the expected version: 1.1.%s"), NEWLINE);
        return false;
    }

    bool wptFound=false;
    // Look for the 'root' of airports: WAYPOINTS tag
    XMLNode node=rootNode.getChildNode(TEXT("WAYPOINTS"));
    if(!node.isEmpty()) {
        StartupStore(TEXT(".. Found airports waypoints.%s"), NEWLINE);
        wptFound=ParseAirports(fp,node);
    }

    // Look for the 'root' of navigation aids: NAVAIDS tag
    node=rootNode.getChildNode(TEXT("NAVAIDS"));
    if(!node.isEmpty()) {
        StartupStore(TEXT(".. Found navigation aids waypoints.%s"), NEWLINE);
        wptFound=wptFound || ParseNavAids(fp,node);
    }

    // Look for the 'root' of hot spots: HOTSPOTS tag
    node=rootNode.getChildNode(TEXT("HOTSPOTS"));
    if(!node.isEmpty()) {
        StartupStore(TEXT(".. Found hot spots waypoints.%s"), NEWLINE);
        wptFound=wptFound || ParseHotSpots(fp,node);
    }

    if(!wptFound) StartupStore(TEXT(".. Waypoints of any kind not found in this OpenAIP file.%s"), NEWLINE);
    return wptFound;
}

bool ParseAirports(ZZIP_FILE *fp, XMLNode &airportsNode)
{
    int numOfAirports=airportsNode.nChildNode(TEXT("AIRPORT")); //count number of airports in the file
    if(numOfAirports<1) {
        StartupStore(TEXT(".. Expected to find at least one AIRPORT tag inside WAYPOINTS tag.%s"), NEWLINE);
        return false;
    }
    if(numOfAirports!=airportsNode.nChildNode()) {
        StartupStore(TEXT(".. Expected to find only AIRPORT tags inside WAYPOINTS tag.%s"), NEWLINE);
        return false;
    } else StartupStore(TEXT(".. OpenAIP waypoints file contains: %u airports.%s"), (unsigned)numOfAirports, NEWLINE);

    XMLNode AirportNode;
    for(int i=0;i<numOfAirports;i++) {
        AirportNode=airportsNode.getChildNode(i);
        if(AirportNode.isEmpty()) {
            StartupStore(TEXT(".. Skipping empty AIRPORT tag.%s"), NEWLINE);
            continue;
        }

        // Airport type
        LPCTSTR dataStr=AirportNode.getAttribute(TEXT("TYPE"));
        if(dataStr==nullptr) {
            StartupStore(TEXT(".. Skipping AIRPORT with no TYPE attribute.%s"), NEWLINE);
            continue;
        }

        size_t len=_tcslen(dataStr);
        int Type=-1;
        if(len==0) {
            StartupStore(TEXT(".. Skipping AIRPORT with empty TYPE attribute.%s"), NEWLINE);
            continue;
        }

        // Prepare the new waypoint
        WAYPOINT new_waypoint;
        new_waypoint.Details = nullptr;
        new_waypoint.Comment = nullptr;
        new_waypoint.Visible = true; // default all waypoints visible at start
        new_waypoint.FarVisible = true;
        new_waypoint.Format = LKW_CUP;
        new_waypoint.Number = WayPointList.size();
        new_waypoint.FileNum = globalFileNum;
        new_waypoint.Style = -1; // -1: style unknown
        bool isClosed=false;
        bool isMilitary=false;
        bool isHeliport=false;
        bool isWaterField=false;


        /*
        AD_CLOSED Aerodrome Closed
        AD_MIL Military Aerodrome
        AF_CIVIL Airfield Civil
        AF_MIL_CIVIL Airfield (civil/military)
        AF_WATER Water Airfield
        APT Airport resp. Airfield IFR
        GLIDING Glider Site
        HELI_CIVIL Heliport Civil
        HELI_MIL Heliport Military
        INTL_APT International Airport
        LIGHT_AIRCRAFT Ultra Light Flying Site
        */
        /*
        1 - Normal
        2 - AirfieldGrass
        3 - Outlanding
        4 - GliderSite
        5 - AirfieldSolid
        6 - MtPass
        7 - MtTop
        8 - Sender
        9 - Vor
        10 - Ndb
        11 - CoolTower
        12 - Dam
        13 - Tunnel
        14 - Bridge
        15 - PowerPlant
        16 - Castle
        17 - Intersection*/

        //TODO: here
        switch(dataStr[0]) {
        case 'A':
            if (_tcsicmp(dataStr,_T("AF_CIVIL"))==0 || _tcsicmp(dataStr,_T("AF_MIL_CIVIL"))==0 || _tcsicmp(dataStr,_T("APT"))==0) new_waypoint.Style=5; //Airfield solid
            else if(_tcsicmp(dataStr,_T("AD_CLOSED"))==0) { new_waypoint.Style=5; isClosed=true; }
            else if(_tcsicmp(dataStr,_T("AD_MIL"))==0) { new_waypoint.Style=5; isMilitary=true; }
            else if(_tcsicmp(dataStr,_T("AF_WATER"))==0) { new_waypoint.Style=2; isWaterField=true; }
            break;
        case 'G':
            if(_tcsicmp(dataStr,_T("GLIDING"))==0) new_waypoint.Style=4; //Glider site
            break;
        case 'H':
            if (_tcsicmp(dataStr,_T("HELI_CIVIL"))==0) { new_waypoint.Style=5; isHeliport=true; }
            else if(_tcsicmp(dataStr,_T("HELI_MIL"))==0) { new_waypoint.Style=5; isHeliport=true; isMilitary=true; }
            break;
        case 'I':
            if(_tcsicmp(dataStr,_T("INTL_APT"))==0) new_waypoint.Style=5;
            break;
        case 'L':
            if(_tcsicmp(dataStr,_T("LIGHT_AIRCRAFT"))==0) new_waypoint.Style=2; //Airfield Grass
            break;
        }
        if(new_waypoint.Style==-1) continue;

        XMLNode node=AirportNode.getChildNode(TEXT("COUNTRY"));
        if(!node.isEmpty() && (dataStr=node.getText(0))!=nullptr && dataStr[0]!='\0') {
            LK_tcsncpy(new_waypoint.Country, dataStr, CUPSIZE_COUNTRY);
            if (_tcslen(dataStr)>3) new_waypoint.Country[3]= _T('\0');
        }

        node=AirportNode.getChildNode(TEXT("NAME"));
        if(!node.isEmpty() && (dataStr=node.getText(0))!=nullptr && dataStr[0]!='\0') {
            LK_tcsncpy(new_waypoint.Name, dataStr, NAME_SIZE);
            if (_tcslen(dataStr)>NAME_SIZE) new_waypoint.Name[NAME_SIZE]= _T('\0');
        } else continue;

        node=AirportNode.getChildNode(TEXT("ICAO"));
        if(!node.isEmpty() && (dataStr=node.getText(0))!=nullptr && dataStr[0]!='\0') {
            LK_tcsncpy(new_waypoint.Code, dataStr, CUPSIZE_CODE);
            if(_tcslen(dataStr)>CUPSIZE_CODE) new_waypoint.Code[CUPSIZE_CODE]=_T('\0');
        }

        node=AirportNode.getChildNode(TEXT("GEOLOCATION"));
        if(!node.isEmpty()) {
            XMLNode subNode=node.getChildNode(TEXT("LAT"));
            if(!subNode.isEmpty() && (dataStr=subNode.getText(0))!=nullptr && dataStr[0]!='\0') {
                new_waypoint.Latitude=_tcstod(dataStr,nullptr);
                if (new_waypoint.Latitude<-90 || new_waypoint.Latitude>90) continue;
            } else continue;
            subNode=node.getChildNode(TEXT("LON"));
            if(!subNode.isEmpty() && (dataStr=subNode.getText(0))!=nullptr && dataStr[0]!='\0') {
                new_waypoint.Longitude=_tcstod(dataStr,nullptr);
                if (new_waypoint.Longitude<-180 || new_waypoint.Longitude>180) continue;
            } else continue;
            subNode=node.getChildNode(TEXT("ELEV"));
            if(!subNode.isEmpty() && (dataStr=subNode.getAttribute(TEXT("UNIT")))!=nullptr && _tcslen(dataStr)==1 && dataStr[0]=='M' && (dataStr=subNode.getText(0))!=nullptr && dataStr[0]!='\0')
                new_waypoint.Altitude=_tcstod(dataStr,nullptr);
            else continue;
        } else continue;

        //TODO: can be more radio tags
        node=AirportNode.getChildNode(TEXT("RADIO"));
        if(!node.isEmpty() && (dataStr=node.getAttribute(TEXT("CATEGORY")))!=nullptr) {
            switch(dataStr[0]) {
            case 'C': //COMMUNICATION Frequency used for communication
                break;
            case 'I': //INFORMATION Frequency to automated information service
                break;
            case 'N': //NAVIGATION Frequency used for navigation
                break;
            case 'O': //OHER Other frequency purpose
                break;
            default:
                continue;
            }

        }




//TODO: parse all the rest!!!!!


        // Add the new waypoint
        if (WaypointInTerrainRange(&new_waypoint)) {
            if(!AddWaypoint(new_waypoint)) {
                return false; // failed to allocate
            }
        }


    }
    return true;
}

bool ParseNavAids(ZZIP_FILE *fp, XMLNode &navAidsNode)
{
    //TODO:....
    return false;
}

bool ParseHotSpots(ZZIP_FILE *fp, XMLNode &hotSpotsNode)
{
    //TODO:...
    return false;
}

