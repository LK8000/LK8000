/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   ITrackingHandler.h
 * Author: Bruno de Lacheisserie
 */
#ifndef TRACKING_ITRACKINGHANDLER_H
#define TRACKING_ITRACKINGHANDLER_H

struct NMEA_INFO;
struct DERIVED_INFO;

class ITrackingHandler {
public:
    virtual ~ITrackingHandler() = default;
    virtual void Update(const NMEA_INFO &Basic, const DERIVED_INFO &Calculated) = 0;
};

#endif // TRACKING_ITRACKINGHANDLER_H
