/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   ParseOpenAIP.cpp
   Created on: Apr 7, 2016
   Author: Alberto Realis-Luc
*/

#include "externs.h"
#include "Waypointparser.h"
#include "utils/stringext.h"
#include "utils/zzip_stream.h"
#include <sstream>
#include "LKStyle.h"
#include "Util/TruncateString.hpp"
#include "Library/rapidxml/rapidxml.hpp"

extern int globalFileNum;

namespace {
    using xml_document = rapidxml::xml_document<char>;
    using xml_node = rapidxml::xml_node<char>;
    using xml_attribute = rapidxml::xml_attribute<char>;
}

static bool ParseAirports(const xml_node* airportsNode);
static bool ParseNavAids(const xml_node* navAidsNode);
static bool ParseHotSpots(const xml_node* hotSpotsNode);
static bool GetGeolocation(const xml_node* parentNode, double &lat, double &lon, double &alt);
static bool GetContent(const xml_node* parentNode, const char* tagName, const char* &outputString);
static bool GetAttribute(const xml_node* parentNode, const char* attributeName, const char* &outputString);
static bool GetValue(const xml_node* parentNode, const char* tagName, double &value);
static bool GetMeasurement(const xml_node* parentNode, const char* tagName, char expectedUnit, double &value);

bool ParseOpenAIP(zzip_stream& stream)
{
    std::string ss;
    xml_document xmldoc;
    try {
        std::istreambuf_iterator<char> it(&stream), end;
        ss.assign(it, end);
        constexpr int Flags = rapidxml::parse_trim_whitespace | rapidxml::parse_normalize_whitespace;
        xmldoc.parse<Flags>(ss.data());
    } catch (std::exception& e) {
        StartupStore(TEXT(".. OPENAIP parse failed : %s"), to_tstring(e.what()).c_str());
        return false;
    }
    
    // Get 'root' node OPENAIP
    const xml_node* root_node = xmldoc.first_node("OPENAIP");    
    if(!root_node) {
        StartupStore(TEXT(".. OPENAIP tag not found.%s"), NEWLINE);
        return false;
    }

    // Check version of OpenAIP format
    const xml_attribute* data_format = root_node->first_attribute("DATAFORMAT");
    if(!data_format || strtod(data_format->value(), nullptr) != 1.1) {
        StartupStore(TEXT(".. DATAFORMAT attribute missing or not at the expected version: 1.1.%s"), NEWLINE);
        return false;
    }

    bool wptFound=false;
    // Look for the 'root' of airports: WAYPOINTS tag
    const xml_node* waypoints_node = root_node->first_node("WAYPOINTS");
    if(waypoints_node) {
        StartupStore(TEXT(".. Found airports waypoints.%s"), NEWLINE);
        wptFound=ParseAirports(waypoints_node);
    }

    // Look for the 'root' of navigation aids: NAVAIDS tag
    const xml_node* navaids_node = root_node->first_node("NAVAIDS");
    if(navaids_node) {
        StartupStore(TEXT(".. Found navigation aids waypoints.%s"), NEWLINE);
        wptFound=wptFound || ParseNavAids(navaids_node);
    }

    // Look for the 'root' of hot spots: HOTSPOTS tag
    const xml_node* hotspots_node = root_node->first_node("HOTSPOTS");
    if(hotspots_node) {
        StartupStore(TEXT(".. Found hot spots waypoints.%s"), NEWLINE);
        wptFound=wptFound || ParseHotSpots(hotspots_node);
    }

    if(!wptFound) {
      StartupStore(TEXT(".. Waypoints of any kind not found in this OpenAIP file.%s"), NEWLINE);
    }
    return wptFound;
}

bool ParseAirports(const xml_node* airportsNode)
{
    if(!airportsNode) {
      return false;
    }
          
    for( const xml_node* AirportNode = airportsNode->first_node("AIRPORT"); AirportNode; AirportNode = AirportNode->next_sibling("AIRPORT")) {

        const char* dataStr = nullptr;
        if(!GetAttribute(AirportNode,"TYPE",dataStr)) continue;


        // Prepare the new waypoint
        WAYPOINT new_waypoint;
        new_waypoint.Details = nullptr;
        new_waypoint.Comment = nullptr;
        new_waypoint.Visible = true; // default all waypoints visible at start
        new_waypoint.FarVisible = true;
        new_waypoint.Format = LKW_OPENAIP;
        new_waypoint.Number = WayPointList.size();
        new_waypoint.FileNum = globalFileNum;
        new_waypoint.Style = STYLE_AIRFIELDSOLID; // default style: solid
        new_waypoint.Flags = AIRPORT + LANDPOINT;

        std::basic_stringstream<TCHAR> comments;

        switch(dataStr[0]) {
        case 'A':
            if (strcasecmp(dataStr, "AF_CIVIL")==0)            comments<<"Civil Airfield"<<std::endl;
            else if(strcasecmp(dataStr, "AF_MIL_CIVIL")==0)    comments<<"Civil and Military Airport"<<std::endl;
            else if(strcasecmp(dataStr, "APT")==0)             comments<<"Airport resp. Airfield IFR"<<std::endl;
            else if(strcasecmp(dataStr, "AD_CLOSED")==0)       comments<<"CLOSED Airport"<<std::endl;
            else if(strcasecmp(dataStr, "AD_MIL")==0)          comments<<"Military Airport"<<std::endl;
            else if(strcasecmp(dataStr, "AF_WATER")==0)        { new_waypoint.Style=STYLE_AIRFIELDGRASS; comments<<"Waterfield"<<std::endl; }
            break;
        case 'G':
            if(strcasecmp(dataStr, "GLIDING")==0)              { new_waypoint.Style=STYLE_GLIDERSITE; comments<<"Glider site"<<std::endl; }
            break;
        case 'H':
            if(!ISGAAIRCRAFT) continue; // Consider heliports only for GA aircraft
            if (strcasecmp(dataStr, "HELI_CIVIL")==0)          { new_waypoint.Style=STYLE_AIRFIELDSOLID; comments<<"Civil Heliport"<<std::endl; }
            else if(strcasecmp(dataStr, "HELI_MIL")==0)        { new_waypoint.Style=STYLE_AIRFIELDSOLID; comments<<"Military Heliport"<<std::endl; }
            break;
        case 'I':
            if(strcasecmp(dataStr, "INTL_APT")==0)             comments<<"International Airport"<<std::endl;
            break;
        case 'L':
            if(strcasecmp(dataStr, "LIGHT_AIRCRAFT")==0)       { new_waypoint.Style=STYLE_AIRFIELDGRASS; comments<<"Ultralight site"<<std::endl; }
            break;
        default:
            continue;
        }
        // Skip unknown waypoints
        if(new_waypoint.Style==-1) continue;

        // Country
        if(GetContent(AirportNode, "COUNTRY", dataStr)) {
            from_utf8(dataStr, new_waypoint.Country);
            if (strlen(dataStr)>3) new_waypoint.Country[3]= _T('\0');
        }

        // Name
        if(GetContent(AirportNode, "NAME", dataStr)) {
            from_utf8(dataStr, new_waypoint.Name);
        } else continue;

        // ICAO code
        if(GetContent(AirportNode, "ICAO", dataStr)) {
            from_utf8(dataStr, new_waypoint.Code);
            if(strlen(dataStr)>CUPSIZE_CODE) new_waypoint.Code[CUPSIZE_CODE]=_T('\0');
        }

        // Geolocation
        if(!GetGeolocation(AirportNode, new_waypoint.Latitude, new_waypoint.Longitude, new_waypoint.Altitude)) continue;

        //Radio frequencies: if more than one just take the first "communication"
        bool found=false, toWrite = true;
        for( const xml_node* node = AirportNode->first_node("RADIO"); node; node = node->next_sibling("RADIO")) {        
            const char* type=nullptr;
            if(GetAttribute(node,"CATEGORY",dataStr) && GetContent(node,"TYPE",type)) {
                const char* freq=nullptr;
                if(!GetContent(node, "FREQUENCY", freq)) continue;
                switch(dataStr[0]) {
                case 'C': //COMMUNICATION Frequency used for communication
                    comments<<"Comm "<<type<<": "<<freq<<" MHz "<<std::endl;
                    if(!found) toWrite=false;
                    break;
                case 'I': //INFORMATION Frequency to automated information service
                    comments <<type<< " "<<new_waypoint.Name <<" "<<freq<<" MHz "<<std::endl;
                    break;
                case 'N': //NAVIGATION Frequency used for navigation
                    comments <<type<< " "<<new_waypoint.Name <<" "<<freq<<" MHz "<<std::endl;
                    break;
                case 'O': //OHER Other frequency purpose
                    comments <<type<<" "<<new_waypoint.Name <<" "<<freq<<" MHz "<<std::endl;
                    break;
                default:
                    continue;
                }
                if(!found) {
                    from_utf8(freq, new_waypoint.Freq);
                    if (strlen(freq)>CUPSIZE_FREQ) new_waypoint.Freq[CUPSIZE_FREQ]= _T('\0');
                    if(!toWrite) {
                      found=true;
                    }
                }
            }
        }



        // Runways: take the longest one
        double maxlength=0, maxdir=0;
        short maxstyle=STYLE_AIRFIELDGRASS;

        // For each runway...
        for( const xml_node* node = AirportNode->first_node("RWY"); node; node = node->next_sibling("RWY")) {        
            // Consider only active runways
            if(!GetAttribute(node,"OPERATIONS",dataStr) || strcasecmp(dataStr,"ACTIVE")!=0) continue;

            // Get runway name
            const char* name=nullptr;
            if(!GetContent(node, "NAME", name)) continue;

            // Get surface type
            const char* surface=nullptr;
            if(!GetContent(node, "SFC", surface)) continue;
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
            if(!GetMeasurement(node,"LENGTH",'M',length)) continue;

            // Runway direction
            const xml_node* subNode = node->first_node("DIRECTION");
            if(!GetAttribute(subNode,"TC",dataStr)) continue;
            double dir=strtod(dataStr,nullptr);

            // Add runway to comments
            comments<<name<<" "<<surface<<" "<<length<<"m "<<dir<<MsgToken(2179)<<std::endl;

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
        SetWaypointComment(new_waypoint, comments.str().c_str());

        // Add the new waypoint
        if (WaypointInTerrainRange(&new_waypoint)) {
            if(AddWaypoint(new_waypoint)) {
                // ownership of this 2 pointer has benn transfered to WaypointList
                new_waypoint.Details = nullptr;
                new_waypoint.Comment = nullptr;
            }
        }
        if(new_waypoint.Comment) { 
            free(new_waypoint.Comment);
            new_waypoint.Comment = nullptr;
        }
        if(new_waypoint.Details) {
            free(new_waypoint.Details);
            new_waypoint.Details = nullptr;
        }
    }
    return true;
}

bool ParseNavAids(const xml_node* navAidsNode)
{
    if(!navAidsNode) {
      return false;
    }
        
    for( const xml_node* NavAidNode = navAidsNode->first_node("NAVAID"); NavAidNode; NavAidNode = NavAidNode->next_sibling("NAVAID")) {

        const char* dataStr=nullptr;

        // Skip not valid NAVAID tags and TYPE attributes
        if(!GetAttribute(NavAidNode,"TYPE",dataStr)) continue;

        // Prepare the new waypoint
        WAYPOINT new_waypoint;
        new_waypoint.Details = nullptr;
        new_waypoint.Comment = nullptr;
        new_waypoint.Visible = true; // default all waypoints visible at start
        new_waypoint.FarVisible = true;
        new_waypoint.Format = LKW_OPENAIP;
        new_waypoint.Number = WayPointList.size();
        new_waypoint.FileNum = globalFileNum;
        new_waypoint.Style = STYLE_NORMAL; // default style: normal

        std::basic_stringstream<TCHAR> comments;

        switch(dataStr[0]) {
        case 'D': //STYLE_VOR //
            if(strcasecmp(dataStr,"DME")==0 || strcasecmp(dataStr,"DVOR")==0 || strcasecmp(dataStr,"DVOR-DME")==0 || strcasecmp(dataStr,"DVORTAC")==0) new_waypoint.Style=STYLE_VOR;
            break;
        case 'N':
            if(strcasecmp(dataStr,"NDB")==0) new_waypoint.Style=STYLE_NDB;
            break;
        case 'V':
            if(strcasecmp(dataStr,"VOR")==0 || strcasecmp(dataStr,"VOR-DME")==0 || strcasecmp(dataStr,"VORTAC")==0) new_waypoint.Style=STYLE_VOR;
            break;
        case 'T':
            if(strcasecmp(dataStr,"TACAN")==0) new_waypoint.Style=STYLE_VOR;
            break;
        default:
            continue;
        }
        // Skip unknown waypoints
        if(new_waypoint.Style==STYLE_NORMAL) continue;

        // Write down in the comments what it is
        comments<<dataStr<<std::endl;

        // Country
        if(GetContent(NavAidNode, "COUNTRY", dataStr)) {
            from_utf8(dataStr, new_waypoint.Country);
            if (strlen(dataStr)>3) new_waypoint.Country[3]= _T('\0');
        }

        // Name
        if(GetContent(NavAidNode, "NAME", dataStr)) {
            from_utf8(dataStr, new_waypoint.Name);
        } else continue;

        // Navigational aid ID
        if(GetContent(NavAidNode, "ID", dataStr)) {
            from_utf8(dataStr, new_waypoint.Code);
            if(strlen(dataStr)>CUPSIZE_CODE) new_waypoint.Code[CUPSIZE_CODE]=_T('\0');
        }

        // Geolocation
        if(!GetGeolocation(NavAidNode, new_waypoint.Latitude, new_waypoint.Longitude, new_waypoint.Altitude)) continue;

        //Radio frequency
        const xml_node* node=NavAidNode->first_node("RADIO");
        if(!GetContent(node, "FREQUENCY", dataStr)) continue;
        comments<<"Frequency: "<<dataStr<<" MHz";
        from_utf8(dataStr, new_waypoint.Freq);
        if (strlen(dataStr)>CUPSIZE_FREQ) new_waypoint.Freq[CUPSIZE_FREQ]= _T('\0');
        if(GetContent(node, "CHANNEL", dataStr)) comments<<" Channel: "<<dataStr;
        comments<<std::endl;

        // Parameters
        node = NavAidNode->first_node("PARAMS");
        double value=0;
        if(GetValue(node,"RANGE",value)) comments<<"Range: "<<value<<" NM ";
        if(GetValue(node,"DECLINATION",value)) comments<<"Declination: "<<value<<MsgToken(2179);
        if(GetContent(node,"ALIGNEDTOTRUENORTH",dataStr)) {
            if(strcasecmp(dataStr,"TRUE")==0) comments<<" True north";
            else if(strcasecmp(dataStr,"TRUE")==0) comments<<" Magnetic north";
        }

        // Add the comments
        SetWaypointComment(new_waypoint, comments.str().c_str());

        // Add the new waypoint
        if (WaypointInTerrainRange(&new_waypoint)) {
            if(AddWaypoint(new_waypoint)) {
                // ownership of this 2 pointer has been transfered to WaypointList
                new_waypoint.Details = nullptr;
                new_waypoint.Comment = nullptr;
            } 
        }
        if(new_waypoint.Comment) { 
            free(new_waypoint.Comment);
            new_waypoint.Comment = nullptr;
        }
        if(new_waypoint.Details) {
            free(new_waypoint.Details);
            new_waypoint.Details = nullptr;
        }
    } // end of for each nav aid
    return true;
}

bool ParseHotSpots(const xml_node* hotSpotsNode) {

    if(!hotSpotsNode) {
      return false;
    }
    
    for( const xml_node* HotSpotNode = hotSpotsNode->first_node("HOTSPOT"); HotSpotNode; HotSpotNode = HotSpotNode->next_sibling("HOTSPOT")) {
        const char* dataStr=nullptr;

        // Skip not valid HOTSPOT tags and TYPE attributes
        if(!GetAttribute(HotSpotNode,"TYPE",dataStr)) continue;

        // Thermal type
        switch(dataStr[0]) {
        case 'A':
            if(strcasecmp(dataStr,"ARTIFICIAL")!=0) continue;
            break;
        case 'N':
            if(strcasecmp(dataStr,"NATURAL")!=0) continue;
            break;
        default:
            continue;
        }

        // Write type down in the comments
        std::basic_stringstream<TCHAR> comments;
        comments<<dataStr;

        // Aircraftcategories: if glider ignore small thermals for paragliders
        const xml_node* node=HotSpotNode->first_node("AIRCRAFTCATEGORIES");
        if(node) {
            bool gliders=false, hangGliders=false /*, paraGliders=false*/;
            for( const xml_node* subNode = node->first_node("AIRCRAFTCATEGORY"); subNode; subNode = subNode->next_sibling("AIRCRAFTCATEGORY")) {
                if(subNode  && (dataStr=subNode->value())!=nullptr && dataStr[0]!='\0') {
                    if(strcasecmp(dataStr,"GLIDER")==0) gliders=true;
                    else if(strcasecmp(dataStr,"HANG_GLIDER")==0) hangGliders=true;
                    //else if(_tcsicmp(dataStr,_T("PARAGLIDER"))==0) paraGliders=true;
                }
            }
            if((ISGLIDER || ISGAAIRCRAFT) && !gliders && !hangGliders) continue;
        }

        // Prepare the new waypoint
        WAYPOINT new_waypoint;
        new_waypoint.Details = nullptr;
        new_waypoint.Comment = nullptr;
        new_waypoint.Visible = true; // default all waypoints visible at start
        new_waypoint.FarVisible = true;
        new_waypoint.Format = LKW_OPENAIP;
        new_waypoint.Number = WayPointList.size();
        new_waypoint.FileNum = globalFileNum;
        new_waypoint.Style = STYLE_THERMAL; // default style: thermal

        // Country
        if(GetContent(HotSpotNode, "COUNTRY", dataStr)) {
            from_utf8(dataStr, new_waypoint.Country);
            if (strlen(dataStr)>3) new_waypoint.Country[3]= _T('\0');
        }

        // Name
        if(GetContent(HotSpotNode, "NAME", dataStr)) {
            from_utf8(dataStr, new_waypoint.Name);
        } else continue;

        // Geolocation
        if(!GetGeolocation(HotSpotNode, new_waypoint.Latitude, new_waypoint.Longitude, new_waypoint.Altitude)) continue;

        // Reliability
        double reliability=0;
        if(!GetValue(HotSpotNode,"RELIABILITY",reliability)) continue;
        comments<<" "<<reliability*100<<"% ";

        // Occourrence
        if(!GetContent(HotSpotNode,"OCCURRENCE",dataStr)) continue;
        comments<<dataStr<<std::endl;

        // Comment
        if(GetContent(HotSpotNode,"COMMENT",dataStr)) comments<<dataStr;

        // Add the comments
        SetWaypointComment(new_waypoint, comments.str().c_str());

        // Add the new waypoint
        if (WaypointInTerrainRange(&new_waypoint)) {
            if(AddWaypoint(new_waypoint)) {
                // ownership of this 2 pointer has been transfered to WaypointList
                new_waypoint.Details = nullptr;
                new_waypoint.Comment = nullptr;
            } 
        }
        if(new_waypoint.Comment) {
            free(new_waypoint.Comment);
            new_waypoint.Comment = nullptr;
        }
        if(new_waypoint.Details) {
            free(new_waypoint.Details);
            new_waypoint.Details = nullptr;
        }
    } // end of for each nav aid
    return true;
}

bool GetGeolocation(const xml_node* parentNode, double &lat, double &lon, double &alt) {
    if(!parentNode) {
      return false;
    }
    const xml_node* node=parentNode->first_node("GEOLOCATION");
    if(node) {
        if(!GetValue(node,"LAT",lat) || lat<-90  || lat>90 ) return false;
        if(!GetValue(node,"LON",lon) || lon<-180 || lon>180) return false;
        if(!GetMeasurement(node,"ELEV",'M',alt)) return false;
        return true;
    }
    return false;
}

bool GetContent(const xml_node *parentNode, const char* tagName, const char* &outputString) {
    if(!parentNode) {
      return false;
    }
    xml_node* node = parentNode->first_node(tagName);
    if(!node) {
      return false;
    }
    outputString = node->value();
    return (outputString && outputString[0] != '\0');
}

bool GetAttribute(const xml_node*node, const char* attributeName, const char* &outputString) {
    if(!node) {
      return false;
    }
    const xml_attribute * attr = node->first_attribute(attributeName);
    if(!attr) {
      return false;
    }
    outputString = attr->value();
    return (outputString && outputString[0] != '\0');
}

bool GetValue(const xml_node* parentNode, const char* tagName, double &value) {
    if(!parentNode) {
      return false;
    }
    const char* dataStr=nullptr;
    if(GetContent(parentNode, tagName, dataStr)) {
        value=strtod(dataStr,nullptr);
        return true;
    }
    return false;
}

bool GetMeasurement(const xml_node* parentNode, const char* tagName, char expectedUnit, double &value) {
    if(!parentNode) {
      return false;
    }
    const xml_node* node=parentNode->first_node(tagName);
    if(node) {
      const char* dataStr = nullptr;
      if(GetAttribute(node,"UNIT",dataStr) && strlen(dataStr)==1 && dataStr[0]==expectedUnit && (dataStr=node->value())!=nullptr && dataStr[0]!='\0') {
          value = strtod(dataStr,nullptr);
          return true;
      }
    }
    return false;
}
