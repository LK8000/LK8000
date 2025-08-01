/***********************************************************************
**
**   windanalyser.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**
***********************************************************************/

/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

*/

#include "externs.h"
#include <numeric>
#include "windanalyser.h"


/*
  About Windanalysation

  Currently, the wind is being analyzed by finding the minimum and the maximum
  groundspeeds measured while flying a circle. The direction of the wind is taken
  to be the direction in wich the speed reaches it's maximum value, the speed
  is half the difference between the maximum and the minimum speeds measured.
  A quality parameter, based on the number of circles allready flown (the first
  circles are taken to be less accurate) and the angle between the headings at
  minimum and maximum speeds, is calculated in order to be able to weigh the
  resulting measurement.

  There are other options for determining the windspeed. You could for instance
  add all the vectors in a circle, and take the resuling vector as the windspeed.
  This is a more complex method, but because it is based on more heading/speed
  measurements by the GPS, it is probably more accurate. If equiped with
  instruments that pass along airspeed, the calculations can be compensated for
  changes in airspeed, resulting in better measurements. We are now assuming
  the pilot flies in perfect circles with constant airspeed, wich is of course
  not a safe assumption.
  The quality indication we are calculation can also be approched differently,
  by calculating how constant the speed in the circle would be if corrected for
  the windspeed we just derived. The more constant, the better. This is again
  more CPU intensive, but may produce better results.

  Some of the errors made here will be averaged-out by the WindStore, wich keeps
  a number of windmeasurements and calculates a weighted average based on quality.
*/

namespace {

// Utility: convert course angle (radian) and speed to a vector
Vector PolarToVector(double speed, double angle) {
  return {speed * cos(angle), speed * sin(angle)};
}

// Normalize angle to [-PI, PI]
double NormalizeAngle(double angle) {
  while (angle > PI) {
    angle -= 2 * PI;
  }
  while (angle < -PI) {
    angle += 2 * PI;
  }
  return angle;
}

} // namespace

// Detect circling: Determines if a full circle (360 degrees) has been flown based on heading changes in the wind samples.
bool WindAnalyser::DetectCircling() {
  if (windsamples.size() < 5) {
    // Not enough samples to detect a circle.
    // full circle can't be faster then 10sec 
    // and at least 0.5Hz gps rate are required
    return false;
  }

  double totalAngleChange = 0.0;
  auto rit = windsamples.rbegin();
  double prevAngle = rit->angle;
  size_t circleStartIdx = windsamples.size() - 1;

  // Iterate backwards through the wind samples, summing the change in heading angle
  for (++rit; rit != windsamples.rend(); ++rit) {
    double currAngle = rit->angle;
    double delta = NormalizeAngle(currAngle - prevAngle); // Normalize to handle wrap-around at 360/0 degrees
    totalAngleChange += delta;
    prevAngle = currAngle;

    // If the total angle change exceeds 360 degrees (2*PI), a full circle is detected
    if (std::abs(totalAngleChange) > (2 * PI)) {
      circleStartIdx = std::distance(rit, windsamples.rend()) - 1;
      break;
    }
  }
  if (std::abs(totalAngleChange) > (2 * PI)) {
    // Remove samples before the start of the detected circle to keep only the most recent circle's data
    if (circleStartIdx > 0 && circleStartIdx < windsamples.size()) {
      windsamples.erase(windsamples.begin(), std::next(windsamples.begin(), circleStartIdx));
    }
    return true; // Full circle detected
  }
  return false; // No full circle detected
}

double WindAnalyser::AverageGroundSpeed() const {
  // Calculate average ground speed
  double avg_speed = std::accumulate(windsamples.begin(), windsamples.end(), 0, [](double v, auto& sample) {
    return v + sample.mag;
  });
  return avg_speed / windsamples.size();
}

/** Called if a new sample is available in the samplelist. */
void WindAnalyser::slot_newSample(NMEA_INFO *nmeaInfo,
                                  DERIVED_INFO *derivedInfo) {

  // remove sample older than 50s
  double min_time = nmeaInfo->Time - 50.;
  // find first value with t > (nmeaInfo->Time - 50.)
  auto it = std::find_if(windsamples.begin(), windsamples.end(),
                        [&](const auto& sample) {
                          return min_time < sample.t;
                        });
  windsamples.erase(windsamples.begin(), it);

  windsamples.push_back({nmeaInfo->Time, nmeaInfo->Speed, nmeaInfo->TrackBearing * DEG_TO_RAD});

  bool fullCircle = DetectCircling();
  if (fullCircle) {
      circleCount++;  //increase the number of circles flown (used to determine the quality)
  }

  if (fullCircle) { //we have completed a full circle!

    _calcWind(nmeaInfo, derivedInfo);    //calculate the wind for this circle

    // Remove the first half of the samples from the current circle.
    // This keeps the most recent half circle, allowing the next wind estimation
    // to use fresher data and reducing overlap between consecutive circles.
    auto mid_it = std::next(windsamples.begin(), windsamples.size() / 3);
    windsamples.erase(windsamples.begin(), mid_it);
  }

  windstore.slot_Altitude(nmeaInfo, derivedInfo);
}

/** Called if the flightmode changes */
void WindAnalyser::slot_newFlightMode() {
    circleCount=0; //reset the circlecounter for each flightmode
		   //change. The important thing to measure is the
		   //number of turns in this thermal only.
}

void WindAnalyser::_calcWind(NMEA_INFO *nmeaInfo,
                             DERIVED_INFO *derivedInfo) {

  // Require at least one full circle for a valid measurement
  if (circleCount < 1) {
    return;
  }

  if (windsamples.empty()) {
    // should never happen, we can't detect circle without sample...
    return;
  }

  const double circle_time = (windsamples.back().t - windsamples.front().t);

  // reject if circle time greater than 50 second
  if(circle_time > 50.0) {
    // this should never happen, since sample older than 50s is removed by `slot_newSample`
    return;
  }

  const int sample_count = windsamples.size();
  
  // reject if average time step greater than 2.0 seconds
  if ( (circle_time / (sample_count-1)) > 2.0) {
    return;
  }

  // Calculate average ground speed
  const double avg_speed = AverageGroundSpeed();
  if (avg_speed < 4) {
    return; // ignore to slow average speed (< 14.4km/h)
  }

  // Find the indices (jmax, jmin) corresponding to the maximum and minimum wind sample "phase"
  // by evaluating a weighted sum (rthisp) for each sample, which helps estimate the points
  // of maximum and minimum groundspeed in the circle.
  double rthismax = 0;
  double rthismin = 0;
  int jmax= -1;
  int jmin= -1;

  for (int j=0; j<sample_count; j++) {
    double rthisp= 0;
    for (int i = 1; i < sample_count; i++) {
      const int ithis = (i + j) % sample_count;
      int idiff = std::min(i, sample_count - i);
      rthisp += windsamples[ithis].mag * idiff;
    }
    if ((rthisp < rthismax) || (jmax == -1)) {
      rthismax = rthisp;
      jmax = j;
    }
    if ((rthisp > rthismin) || (jmin == -1)) {
      rthismin = rthisp;
      jmin = j;
    }
  }

  if (jmin < 0 || jmax < 0) {
    return;
  }
  // jmax is the point where most wind samples are below
  // jmin is the point where most wind samples are above

  const double mag = (windsamples[jmax].mag - windsamples[jmin].mag) / 2.;
  if (mag > 30) {
    // limit to reasonable values (60 knots), reject otherwise
    return;
  }


  // attempt to fit cycloid
  double rthis = 0;
  // Calculate fit error (rthis) between measured and fitted cycloid
  for (int i = 0; i < sample_count; i++) {
    // Calculate the phase angle for this sample relative to jmax
    const double phase = ((i + jmax) % sample_count) * PI * 2.0 / sample_count;
    // Compute the fitted ground speed vector for this phase
    Vector w = {
      cos(phase) * avg_speed + mag,
      sin(phase) * avg_speed
    };
    // Calculate the magnitude of the fitted ground speed vector
    const double fitted_mag = Length(w);
    // Get the measured ground speed magnitude for this sample
    const double measured_mag = windsamples[i].mag;
    // Compute the difference between fitted and measured magnitude
    const double cmag = fitted_mag - measured_mag;
    // Accumulate the squared error
    rthis += cmag * cmag;
  }
  // Average the squared error over all samples
  rthis /= sample_count;

  if (rthis < 0) {
    // rthis is the mean squared error, always non-negative by construction.
    // sqrt(rthis) is also always >= 0.
    // Therefore, rthis < 0 cannot happen unless there is a floating point error,
    // which is extremely unlikely in this context.
    return;
  }
  rthis = sqrt(rthis);

  int quality;

  // Calculate quality based on wind magnitude and fit error
  if (mag > 1) {
    quality = 5 - iround((rthis / mag) * 3);
  } else {
    quality = 5 - iround(rthis);
  }

  // Penalize quality for low circle count
  if (circleCount < 5) {
    // first + 4 next half circle
    quality--;
  }
  if (circleCount < 3) {
    // first + 2 next half circle
    quality--;
  }
  if (quality < 1) {
    return;  // measurement quality too low
  }

  quality = min(quality, 5);  // 5 is maximum quality, make sure we honor that.

  if (windsamples[jmax].mag == 0) {
    return;
  }

  Vector wind = PolarToVector(-mag, windsamples[jmax].angle);
  slot_newEstimate(nmeaInfo, derivedInfo, wind, quality);
}


void WindAnalyser::slot_newEstimate(NMEA_INFO *nmeaInfo,
                                    DERIVED_INFO *derivedInfo,
                                    Vector a, int quality)
{
  windstore.slot_measurement(nmeaInfo, derivedInfo, a, quality);
}
