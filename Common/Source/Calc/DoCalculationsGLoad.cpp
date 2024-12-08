/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *  
 * File:   DoCalculationsGLoad.cpp
 * Author: Bruno de Lacheisserie
 */
#include "DoCalculationsGLoad.h"
#include "NMEA/Info.h"
#include "NMEA/Derived.h"
#include <numeric>

namespace {

Point3D  Average(const std::vector<Point3D>& Acceleration) {
    Point3D sum = std::accumulate(Acceleration.begin(),
                                  Acceleration.end(),
                                  Point3D{0., 0., 0.});

    return sum / Acceleration.size();
}

void FromSensor(const NMEA_INFO& Basic, DERIVED_INFO& Calculated) {
    Calculated.Acceleration = Average(Basic.Acceleration);
    Calculated.Gload = Calculated.Acceleration.length();
}

} // namespace

void DoCalculationsGLoad(const NMEA_INFO& Basic, DERIVED_INFO& Calculated) {
    if (AccelerationAvailable(Basic)) {
        FromSensor(Basic, Calculated);
    }
    // if Acceleration not available, Gload Will be Calculated later by Heading() function
}
