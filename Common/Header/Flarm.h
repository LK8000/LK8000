/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Flarm.h
 */

#ifndef _FLARM_H
#define _FLARM_H

// Max number of local ids in file
#define MAXFLARMLOCALS	50

// Max Simultaneous traffic aka MAXTRAFFIC
#define FLARM_MAX_TRAFFIC	50
#define MAX_FLARM_TRACES	5000

// These are always used +1 for safety
#define	MAXFLARMNAME	30	// used by Local Ids
#define MAXFLARMCN	3	// used by Local and Flarmnet, not changeble see Parser

struct FLARM_TRAFFIC {
    uint32_t RadioId;
    double Latitude;
    double Longitude;
    double TrackBearing;
    double Speed;
    double Altitude;
    double TurnRate;
    double ClimbRate;
    TCHAR Name[MAXFLARMNAME+1];
    TCHAR Cn[MAXFLARMCN+1];
    unsigned short IDType;
    unsigned short AlarmLevel;
    double Time_Fix;
    unsigned short Type;
    unsigned short Status; // 100120
    bool Locked; // 100120
    // When set true, name has been changed and Cn must be updated
    bool UpdateNameFlag;
    double Average30s;
    // These are calculated values, updated only inside an offline copy
    double Distance;
    double Bearing;
    double AltArriv;
    double GR;
    double EIAS;
};

struct FLARM_TRACE {
    double fLat;
    double fLon;
    double fAlt;
    int iColorIdx;
};

struct NMEA_INFO;
struct FlarmId;

void CheckBackTarget(NMEA_INFO &Info, int slot);
void UpdateFlarmTarget(NMEA_INFO &Info);

void OpenFLARMDetails();
void CloseFLARMDetails();

const TCHAR* LookupFLARMCn(uint32_t RadioId);

const TCHAR* LookupFLARMDetails(uint32_t RadioId);
uint32_t LookupFLARMDetails(TCHAR *cn);

bool AddFlarmLookupItem(uint32_t RadioId, TCHAR *name, bool saveFile);

const FlarmId* LookupFlarmId(uint32_t RadioId);

#endif
