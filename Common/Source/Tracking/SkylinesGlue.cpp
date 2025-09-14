/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   SkylinesGlue.cpp
 * Author: Bruno de Lacheisserie
 */

#include "SkylinesGlue.h"
#include "Parser.h"
#include "Defines.h"
#include "NavFunctions.h"
#include "Util/TruncateString.hpp"
#include "NMEA/Info.h"

extern NMEA_INFO GPS_INFO;
extern Mutex CritSec_FlightData;
extern double LastFlarmCommandTime;

#ifdef HAVE_SKYLINES_TRACKING_HANDLER

void SkylinesGlue::OnTraffic(uint32_t pilot_id, unsigned time_of_day_ms,
                 const GeoPoint &location, int altitude) {

    TrackingGlue::OnTraffic(pilot_id, time_of_day_ms, location, altitude);

    const SkyLinesTracking::Data& data = GetSkyLinesData();

    tstring pilote_name = WithLock(data.mutex, &SkyLinesTracking::Data::GetUserName, data, pilot_id);

    const ScopeLock protect(CritSec_FlightData);

	double Time_Fix = GPS_INFO.Time;

    GPS_INFO.FLARM_Available = true;
    LastFlarmCommandTime = Time_Fix;

    int flarm_slot = FLARM_FindSlot(&GPS_INFO, pilot_id);
    if (flarm_slot < 0) {
        return;
    }

    FLARM_TRAFFIC& Traffic = GPS_INFO.FLARM_Traffic[flarm_slot];

    bool newtraffic = (Traffic.Status == LKT_EMPTY);
    // before changing timefix, see if it was an old target back locked in!
    CheckBackTarget(GPS_INFO, flarm_slot);

    double Average30s = 0;
    double TrackBearing = 0;
    double Speed = 0;

    if (newtraffic) {
        Traffic.RadioId = pilot_id;

        Traffic.AlarmLevel = 0;
        Traffic.TurnRate = 0;

    } else {

        double deltaT = Time_Fix - Traffic.Time_Fix;
        if (deltaT > 0) {
            double Distance = 0;
            double Bearing = 0;

            DistanceBearing(Traffic.Latitude, Traffic.Longitude, 
                            location.latitude, location.longitude,
                            &Distance, &Bearing);

            double deltaH = altitude - Traffic.Altitude;
            TrackBearing = Bearing;
            Speed = Distance / deltaT;
            Average30s = deltaH / deltaT;
        }
    }

    Traffic.UpdateNameFlag=false; // clear flag first
    CopyTruncateString(Traffic.Name, MAXFLARMNAME, pilote_name.c_str());


    Traffic.Status = LKT_REAL;
    Traffic.Time_Fix = Time_Fix; //GPS_INFO.Time;
    Traffic.Latitude = location.latitude;
    Traffic.Longitude = location.longitude;
    Traffic.Altitude = altitude;
    Traffic.Speed = Speed;
    Traffic.TrackBearing = TrackBearing; // to be replaced by Livetrack24 cog
    Traffic.Average30s = Average30s;
}

void SkylinesGlue::OnUserName(uint32_t user_id, const TCHAR *name) {
    TrackingGlue::OnUserName(user_id, name);

    const ScopeLock protect(CritSec_FlightData);

    int flarm_slot = FLARM_FindSlot(&GPS_INFO, user_id);
    if (flarm_slot < 0) {
        return;
    }

    FLARM_TRAFFIC& Traffic = GPS_INFO.FLARM_Traffic[flarm_slot];
    Traffic.UpdateNameFlag=false; // clear flag first
    CopyTruncateString(Traffic.Name, MAXFLARMNAME, name);
}

#endif // HAVE_SKYLINES_TRACKING_HANDLER
