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

    // Look for the 'root' AIRSPACES tag
    XMLNode airspacesNode=rootNode.getChildNode(TEXT("WAYPOINTS"));
    if(airspacesNode.isEmpty()) { //ERROR no AIRSPACES tag found in AIP file
        StartupStore(TEXT(".. WAYPOINTS tag not found.%s"), NEWLINE);
        return false;
    }

    //TODO: continue here!

    return true;
}



