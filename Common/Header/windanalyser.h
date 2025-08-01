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
  Vector v;
  double t;
  double mag;
};


class WindAnalyser  {

public:
    WindAnalyser();
    ~WindAnalyser();

    WindStore windstore;

// Signals
    /**
      * Send if a new windmeasurement has been made. The result is included in wind,
      * the quality of the measurement (1-5; 1 is bad, 5 is excellent) in quality.
      */
    void newMeasurement(Vector wind, int quality);

// Public slots
    /**
     * Called if the flightmode changes
     */
    void slot_newFlightMode(NMEA_INFO *nmeaInfo,
                            DERIVED_INFO *derivedInfo,
                            bool left, int);
    /**
     * Called if a new sample is available in the samplelist.
     */
    void slot_newSample(NMEA_INFO *nmeaInfo,
                        DERIVED_INFO *derivedInfo);

    // used to update output if altitude changes
    void slot_Altitude(NMEA_INFO *nmeaInfo,
                       DERIVED_INFO *derivedInfo);

    void slot_newEstimate(NMEA_INFO *nmeaInfo,
                          DERIVED_INFO *derivedInfo,
                          Vector v, int quality);

    //    void calcThermalDrift();
private: // Private attributes
    int circleCount; //we are counting the number of circles, the first onces are probably not very round
    bool circleLeft; //true=left, false=right
    bool active;     //active is set to true or false by the slot_newFlightMode slot
    int startmarker;
    int startheading;
    int circleDeg;
    int lastHeading;
    bool pastHalfway;
    Vector minVector;
    Vector maxVector;
    int satCnt;
    int minSatCnt;
    bool curModeOK;
    bool first;
    int startcircle;

    Vector climbstartpos;
    Vector climbendpos;
    double climbstarttime;
    double climbendtime;

    std::vector<WindSample> windsamples;

private: // Private memberfunctions
    void _calcWind(NMEA_INFO *nmeaInfo,
                   DERIVED_INFO *derivedInfo);

};

#endif
