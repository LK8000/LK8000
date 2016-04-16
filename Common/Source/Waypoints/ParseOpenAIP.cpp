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
#include <sstream>
#include "LKStyle.h"

extern int globalFileNum;

bool ParseAirports(XMLNode &airportsNode);
bool ParseNavAids(XMLNode &navAidsNode);
bool ParseHotSpots(XMLNode &hotSpotsNode);
bool GetGeolocation(XMLNode &parentNode, double &lat, double &lon, double &alt);
bool GetContent(XMLNode &parentNode, LPCTSTR tagName, LPCTSTR &outputString);
bool GetAttribute(XMLNode &node, LPCTSTR attributeName, LPCTSTR &outputString);
bool GetValue(XMLNode &parentNode, LPCTSTR tagName, double &value);
bool GetMeasurement(XMLNode &parentNode, LPCTSTR tagName, char expectedUnit, double &value);

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
        wptFound=ParseAirports(node);
    }

    // Look for the 'root' of navigation aids: NAVAIDS tag
    node=rootNode.getChildNode(TEXT("NAVAIDS"));
    if(!node.isEmpty()) {
        StartupStore(TEXT(".. Found navigation aids waypoints.%s"), NEWLINE);
        wptFound=wptFound || ParseNavAids(node);
    }

    // Look for the 'root' of hot spots: HOTSPOTS tag
    node=rootNode.getChildNode(TEXT("HOTSPOTS"));
    if(!node.isEmpty()) {
        StartupStore(TEXT(".. Found hot spots waypoints.%s"), NEWLINE);
        wptFound=wptFound || ParseHotSpots(node);
    }

    if(!wptFound) StartupStore(TEXT(".. Waypoints of any kind not found in this OpenAIP file.%s"), NEWLINE);
    return wptFound;
}

bool ParseAirports(XMLNode &airportsNode)
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
    LPCTSTR dataStr=nullptr;
    for(int i=0;i<numOfAirports;i++) {
        AirportNode=airportsNode.getChildNode(i);

        // Skip not valid AIRPORT tags and TYPE attributes
        if(!GetAttribute(AirportNode,TEXT("TYPE"),dataStr)) continue;

        // Prepare the new waypoint
        WAYPOINT new_waypoint;
        new_waypoint.Details = nullptr;
        new_waypoint.Comment = nullptr;
        new_waypoint.Visible = true; // default all waypoints visible at start
        new_waypoint.FarVisible = true;
        new_waypoint.Format = LKW_CUP;
        new_waypoint.Number = WayPointList.size();
        new_waypoint.FileNum = globalFileNum;
        new_waypoint.Style = STYLE_AIRFIELDSOLID; // default style: solid
        new_waypoint.Flags = AIRPORT + LANDPOINT;

        std::basic_stringstream<TCHAR> comments;

        switch(dataStr[0]) {
        case 'A':
            if (_tcsicmp(dataStr,_T("AF_CIVIL"))==0)            comments<<"Civil Airfield"<<std::endl;
            else if(_tcsicmp(dataStr,_T("AF_MIL_CIVIL"))==0)    comments<<"Civil and Military Airport"<<std::endl;
            else if(_tcsicmp(dataStr,_T("APT"))==0)             comments<<"Airport resp. Airfield IFR"<<std::endl;
            else if(_tcsicmp(dataStr,_T("AD_CLOSED"))==0)       comments<<"CLOSED Airport"<<std::endl;
            else if(_tcsicmp(dataStr,_T("AD_MIL"))==0)          comments<<"Military Airport"<<std::endl;
            else if(_tcsicmp(dataStr,_T("AF_WATER"))==0)        { new_waypoint.Style=STYLE_AIRFIELDGRASS; comments<<"Waterfield"<<std::endl; }
            break;
        case 'G':
            if(_tcsicmp(dataStr,_T("GLIDING"))==0)              { new_waypoint.Style=STYLE_GLIDERSITE; comments<<"Glider site"<<std::endl; }
            break;
        case 'H':
            continue; // For now skip heliports
            //if (_tcsicmp(dataStr,_T("HELI_CIVIL"))==0) { new_waypoint.Style=STYLE_AIRFIELDSOLID; }
            //else if(_tcsicmp(dataStr,_T("HELI_MIL"))==0) { new_waypoint.Style=STYLE_AIRFIELDSOLID; }
            //break;
        case 'I':
            if(_tcsicmp(dataStr,_T("INTL_APT"))==0)             comments<<"International Airport"<<std::endl;
            break;
        case 'L':
            if(_tcsicmp(dataStr,_T("LIGHT_AIRCRAFT"))==0)       { new_waypoint.Style=STYLE_AIRFIELDGRASS; comments<<"Ultralight site"<<std::endl; }
            break;
        }
        // Skip unknown waypoints
        if(new_waypoint.Style==-1) continue;

        // Country
        if(GetContent(AirportNode, TEXT("COUNTRY"), dataStr)) {
            LK_tcsncpy(new_waypoint.Country, dataStr, CUPSIZE_COUNTRY);
            if (_tcslen(dataStr)>3) new_waypoint.Country[3]= _T('\0');
        }

        // Name
        if(GetContent(AirportNode, TEXT("NAME"), dataStr)) {
            LK_tcsncpy(new_waypoint.Name, dataStr, NAME_SIZE);
            if (_tcslen(dataStr)>NAME_SIZE) new_waypoint.Name[NAME_SIZE]= _T('\0');
        } else continue;

        // ICAO code
        if(GetContent(AirportNode, TEXT("ICAO"), dataStr)) {
            LK_tcsncpy(new_waypoint.Code, dataStr, CUPSIZE_CODE);
            if(_tcslen(dataStr)>CUPSIZE_CODE) new_waypoint.Code[CUPSIZE_CODE]=_T('\0');
        }

        // Geolocation
        if(!GetGeolocation(AirportNode, new_waypoint.Latitude, new_waypoint.Longitude, new_waypoint.Altitude)) continue;

        //Radio frequencies: if more than one just take the first "communication"
        int numOfNodes=AirportNode.nChildNode(TEXT("RADIO"));
        XMLNode node, subNode;
        bool found=false, toWrite(numOfNodes==1);
        for(int i=0;i<numOfNodes;i++) {
            node=AirportNode.getChildNode(TEXT("RADIO"),i);
            LPCTSTR type=nullptr;
            if(GetAttribute(node,TEXT("CATEGORY"),dataStr) && GetContent(node,TEXT("TYPE"),type)) {
                LPCTSTR freq=nullptr;
                if(!GetContent(node, TEXT("FREQUENCY"), freq)) continue;
                switch(dataStr[0]) {
                case 'C': //COMMUNICATION Frequency used for communication
                    comments<<"Comm "<<type<<": "<<freq<<" MHz "<<std::endl;
                    if(!found) toWrite=true;
                    break;
                case 'I': //INFORMATION Frequency to automated information service
                    comments<<"Info "<<type<<": "<<freq<<" MHz "<<std::endl;
                    break;
                case 'N': //NAVIGATION Frequency used for navigation
                    comments<<"Nav "<<type<<": "<<freq<<" MHz "<<std::endl;
                    break;
                case 'O': //OHER Other frequency purpose
                    comments<<"Other "<<type<<": "<<freq<<" MHz "<<std::endl;
                    break;
                default:
                    continue;
                }
                if(toWrite) {
                    LK_tcsncpy(new_waypoint.Freq, freq, CUPSIZE_FREQ);
                    if (_tcslen(dataStr)>CUPSIZE_FREQ) new_waypoint.Freq[CUPSIZE_FREQ]= _T('\0');
                    toWrite=false;
                    found=true;
                }
            }
        }

        // Runways: take the longest one
        double maxlength=0, maxdir=0;
        short maxstyle=STYLE_AIRFIELDGRASS;

        // For each runway...
        numOfNodes=AirportNode.nChildNode(TEXT("RWY"));
        for(int i=0;i<numOfNodes;i++) {
            node=AirportNode.getChildNode(TEXT("RWY"),i);

            // Consider only active runways
            if(!GetAttribute(node,TEXT("OPERATIONS"),dataStr) || _tcsicmp(dataStr,_T("ACTIVE"))!=0) continue;

            // Get runway name
            LPCTSTR name=nullptr;
            if(!GetContent(node, TEXT("NAME"), name)) continue;

            // Get surface type
            LPCTSTR surface=nullptr;
            if(!GetContent(node, TEXT("SFC"), surface)) continue;
            short style=surface[0]=='A' || surface[0]=='C' ? STYLE_AIRFIELDSOLID : STYLE_AIRFIELDGRASS; // Default grass
            /*switch(surface[0]) {
            case 'A': // ASPH Asphalt
                style=STYLE_AIRFIELDSOLID;
                break;
            case 'C': // CONC Concrete
                style=STYLE_AIRFIELDSOLID;
                break;
            case 'G': //GRAS Grass GRVL Gravel
                break;
            case 'I': // ICE
                break;
            case 'S': //SAND SNOW SOIL
                break;
            case 'U': //UNKN Unknown
                break;
            case 'W': //WATE Water
                break;
            default:
                continue;
            }*/

            // Runway length
            double length=0;
            if(!GetMeasurement(node,TEXT("LENGTH"),'M',length)) continue;

            // Runway direction
            subNode=node.getChildNode(TEXT("DIRECTION"),0);
            if(!GetAttribute(subNode,TEXT("TC"),dataStr)) continue;
            double dir=_tcstod(dataStr,nullptr);

            // Add runway to comments
            comments<<name<<" "<<surface<<" "<<length<<"m "<<dir<<gettext(_T("_@M2179_"))<<std::endl;

            // Check if we found the longest one
            if(length>maxlength) {
                maxlength=length;
                maxdir=dir;
                maxstyle=style;
            }
        } // for each runway

        if(maxlength>0) {
            new_waypoint.RunwayLen=maxlength;
            new_waypoint.RunwayDir=maxdir;
            if(new_waypoint.Style!=STYLE_GLIDERSITE) new_waypoint.Style=maxstyle; //if is not already a gliding site we just check if is "solid" surface or not...
        }

        // Add the comments
        std::basic_string<TCHAR> str(comments.str());
        new_waypoint.Comment = new TCHAR[str.length()+1];
        if (new_waypoint.Comment != nullptr) {
            std::copy(str.begin(),str.end(),new_waypoint.Comment);
            new_waypoint.Comment[str.length()]='\0';
        }

        // Add the new waypoint
        if (WaypointInTerrainRange(&new_waypoint))
            if(!AddWaypoint(new_waypoint)) return false; // failed to allocate
    }
    return true;
}

bool ParseNavAids(XMLNode &navAidsNode)
{
    //TODO:

    /*
    #define STYLE_NORMAL      1
    #define STYLE_AIRFIELDGRASS 2
    #define STYLE_OUTLANDING    3
    #define STYLE_GLIDERSITE    4
    #define STYLE_AIRFIELDSOLID 5
    #define STYLE_MTPASS        6
    #define STYLE_MTTOP     7
    #define STYLE_SENDER        8
    #define STYLE_VOR       9
    #define STYLE_NDB       10
    #define STYLE_COOLTOWER     11
    #define STYLE_DAM       12
    #define STYLE_TUNNEL        13
    #define STYLE_BRIDGE        14
    #define STYLE_POWERPLANT    15
    #define STYLE_CASTLE        16
    #define STYLE_INTERSECTION  17
    */

    StartupStore(TEXT(".. OpenAIP NavAids not yet supported in LK8000.%s"), NEWLINE);
    return false;
}

bool ParseHotSpots(XMLNode &hotSpotsNode) {
    //TODO:...
    StartupStore(TEXT(".. OpenAIP HotSpots not yet supported in LK8000.%s"), NEWLINE);
    return false;
}

bool GetGeolocation(XMLNode &parentNode, double &lat, double &lon, double &alt) {
    XMLNode node=parentNode.getChildNode(TEXT("GEOLOCATION"));
    if(!node.isEmpty()) {
        if(!GetValue(node,TEXT("LAT"),lat) || lat<-90  || lat>90 ) return false;
        if(!GetValue(node,TEXT("LON"),lon) || lon<-180 || lon>180) return false;
        if(!GetMeasurement(node,TEXT("ELEV"),'M',alt)) return false;
        return true;
    }
    return false;
}

bool GetContent(XMLNode &parentNode, LPCTSTR tagName, LPCTSTR &outputString) {
    XMLNode node=parentNode.getChildNode(tagName);
    return (!node.isEmpty() && (outputString=node.getText(0))!=nullptr && outputString[0]!='\0');
}

bool GetAttribute(XMLNode &node, LPCTSTR attributeName, LPCTSTR &outputString) {
    return (!node.isEmpty() && (outputString=node.getAttribute(attributeName))!=nullptr && outputString[0]!='\0');
}

bool GetValue(XMLNode &parentNode, LPCTSTR tagName, double &value) {
    LPCTSTR dataStr=nullptr;
    if(GetContent(parentNode, tagName, dataStr)) {
        value=_tcstod(dataStr,nullptr);
        return true;
    }
    return false;
}

bool GetMeasurement(XMLNode &parentNode, LPCTSTR tagName, char expectedUnit, double &value) {
    XMLNode node=parentNode.getChildNode(tagName);
    LPCTSTR dataStr = nullptr;
    if(GetAttribute(node,TEXT("UNIT"),dataStr) && _tcslen(dataStr)==1 && dataStr[0]==expectedUnit && (dataStr=node.getText(0))!=nullptr && dataStr[0]!='\0') {
        value = _tcstod(dataStr,nullptr);
        return true;
    }
    return false;
}
