/***********************************************************************
**
**   windanalyser.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id: windanalyser.h,v 1.1 2011/12/21 10:35:29 root Exp root $
**
***********************************************************************/

#ifndef WINDANALYSER_H
#define WINDANALYSER_H

#include "windstore.h"
#include <vector>

/**The windanalyser analyses the list of flightsamples looking for windspeed and direction.
  *@author André Somers
  */


class WindSample {
 public:
  double t; // s
  double mag; // m/s
  double angle; // rad
};


class WindAnalyser final {

public:
    WindAnalyser() = default;

    /**
     * Called if the flightmode changes
     */
    void slot_newFlightMode();

    /**
     * Called if a new sample is available in the samplelist.
     */
    void slot_newSample(NMEA_INFO *nmeaInfo,
                        DERIVED_INFO *derivedInfo);

    /**
     * used to update output if altitude changes
     */ 
    void slot_Altitude(NMEA_INFO *nmeaInfo, DERIVED_INFO *derivedInfo) {
        windstore.slot_Altitude(nmeaInfo, derivedInfo);
    }

    void slot_newEstimate(NMEA_INFO *nmeaInfo,
                          DERIVED_INFO *derivedInfo,
                          Vector v, int quality);

    Vector getWind(double Time, double h, bool *found) {
        return windstore.getWind(Time, h, found);
    }


private: // Private attributes
    int circleCount = 0; // we are counting the number of circles, the first onces are probably not very round

    WindStore windstore;
    std::vector<WindSample> windsamples;

private: // Private memberfunctions

    bool DetectCircling();

    double AverageGroundSpeed() const;

    void _calcWind(NMEA_INFO *nmeaInfo,
                   DERIVED_INFO *derivedInfo);
};

#endif
