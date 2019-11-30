/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
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
    double Latitude;
    double Longitude;
    double TrackBearing;
    double Speed;
    double Altitude;
    double TurnRate;
    double ClimbRate;
    double RelativeNorth;
    double RelativeEast;
    double RelativeAltitude;
    int ID;
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

#endif
