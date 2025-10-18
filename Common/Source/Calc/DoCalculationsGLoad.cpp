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

void DoCalculationsGLoad(const NMEA_INFO& Basic, DERIVED_INFO& Calculated) {
    if (Basic.Acceleration.available()) {
      Calculated.Acceleration = Basic.Acceleration;
    }
    if (Basic.Gload.available()) {
      Calculated.Gload = Basic.Gload;
    }
    else if (Basic.Acceleration.available()) {
      Calculated.Gload = Calculated.Acceleration.length();
    }

    // if Acceleration nor GLoad available, Gload Will be Calculated later by Heading() function
}
