/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GeoPoint.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 19, 2019, 12:59 AM
 */

#include "GeoPoint.h"
#define _USE_MATH_DEFINES
#include <cmath>

double CrossTrackError(const GeoPoint& from, const GeoPoint& to, const GeoPoint& current) {
    // Calculate cross-track distance & azimuths
    double azi2 = from.Bearing(to);

    double cte, azi1;
    from.Reverse(current, azi1, cte);

    // Calculate the cross-track error
    return sin((azi1 - azi2) * M_PI / 180) * cte;
}
