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
    LPCTSTR dataStr=nullptr;
    for(int i=0;i<numOfAirports;i++) {
        AirportNode=airportsNode.getChildNode(i);

        // Skip not valid AIRPORT tags and TYPE attributes
        if(AirportNode.isEmpty() || (dataStr=AirportNode.getAttribute(TEXT("TYPE")))==nullptr || dataStr[0]=='\0') continue;

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
        XMLNode subNode;
        if(!node.isEmpty()) {
            subNode=node.getChildNode(TEXT("LAT"));
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

        //Radio frequencies: if more than one just take the first "communication"
        int numOfNodes=AirportNode.nChildNode(TEXT("RADIO"));
        bool found=false, toWrite(numOfNodes==1);
        for(int i=0;i<numOfNodes;i++) {
            node=AirportNode.getChildNode(TEXT("RADIO"),i);
            LPCTSTR type=nullptr;
            if(!node.isEmpty() && (dataStr=node.getAttribute(TEXT("CATEGORY")))!=nullptr && !(subNode=node.getChildNode(TEXT("TYPE"))).isEmpty() && (type=subNode.getText(0))!=nullptr && type[0]!='\0') {
                subNode=node.getChildNode(TEXT("FREQUENCY"));
                LPCTSTR freq=nullptr;
                if(subNode.isEmpty() || (freq=subNode.getText(0))==nullptr || freq[0]=='\0') continue;
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
            if(node.isEmpty() || (dataStr=node.getAttribute(TEXT("OPERATIONS")))==nullptr || dataStr[0]=='\0' || _tcsicmp(dataStr,_T("ACTIVE"))!=0) continue;

            // Get runway name
            subNode=node.getChildNode(TEXT("NAME"));
            if(subNode.isEmpty() || (dataStr=subNode.getText(0))==nullptr || dataStr[0]=='\0') continue;
            LPCTSTR name=dataStr;

            // Get surface type
            subNode=node.getChildNode(TEXT("SFC"));
            if(subNode.isEmpty() || (dataStr=subNode.getText(0))==nullptr || dataStr[0]=='\0') continue;
            short style=STYLE_AIRFIELDGRASS; // Default grass
            LPCTSTR surface=nullptr;
            switch(dataStr[0]) {
            case 'A': // ASPH Asphalt
                style=STYLE_AIRFIELDSOLID;
                surface=TEXT("asphalt");
                break;
            case 'C': // CONC Concrete
                style=STYLE_AIRFIELDSOLID;
                surface=TEXT("concrete");
                break;
            case 'G': //GRAS Grass GRVL Gravel
                if(_tcsicmp(dataStr,_T("GRAS"))==0) surface=TEXT("grass");
                else if(_tcsicmp(dataStr,_T("GRVL"))==0) surface=TEXT("gravel");
                else continue;
                break;
            case 'I': // ICE
                comments<<"ice";
                break;
            case 'S': //SAND SNOW SOIL
                if(_tcsicmp(dataStr,_T("SAND"))!=0) surface=TEXT("sand");
                else if(_tcsicmp(dataStr,_T("SNOW"))==0) surface=TEXT("snow");
                else if(_tcsicmp(dataStr,_T("SOIL"))==0) surface=TEXT("soil");
                else continue;
                break;
            case 'U': //UNKN Unknown
                surface=TEXT("unknown");
                break;
            case 'W': //WATE Water
                surface=TEXT("water");
                break;
            default:
                continue;
            }
            if(surface==nullptr) continue;

            // Runway length
            XMLNode subNode=node.getChildNode(TEXT("LENGTH"));
            if(subNode.isEmpty() || (dataStr=subNode.getAttribute(TEXT("UNIT")))==nullptr || dataStr[0]!='M' || (dataStr=subNode.getText(0))==nullptr || dataStr[0]=='\0') continue;
            double length=_tcstod(dataStr,nullptr);

            // Runway direction
            subNode=node.getChildNode(TEXT("DIRECTION"),0);
            if(subNode.isEmpty() || (dataStr=subNode.getAttribute(TEXT("TC")))==nullptr || dataStr[0]=='\0') continue;
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

bool ParseNavAids(ZZIP_FILE *fp, XMLNode &navAidsNode)
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

bool ParseHotSpots(ZZIP_FILE *fp, XMLNode &hotSpotsNode)
{
    //TODO:...
    StartupStore(TEXT(".. OpenAIP HotSpots not yet supported in LK8000.%s"), NEWLINE);
    return false;
}

