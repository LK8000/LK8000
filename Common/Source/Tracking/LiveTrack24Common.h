/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */

#ifndef TRACKING_LIVETRACK24_COMMON_H_
#define TRACKING_LIVETRACK24_COMMON_H_

#include <ctime>

/**
 * Represents a single tracking point for LiveTrack24.
 */
struct livetracker_point_t {
  /** UNIX timestamp (seconds since epoch) */
  time_t unix_timestamp;
  /** Flying status flag: 1 if flying, 0 if on ground */
  int flying;
  /** Latitude in decimal degrees (range: -90 to 90) */
  double latitude;
  /** Longitude in decimal degrees (range: -180 to 180) */
  double longitude;
  /** Altitude in meters above sea level */
  double alt;
  /** Ground speed in meters per second */
  double ground_speed;
  /** Course over ground in degrees (range: 0 to 360) */
  double course_over_ground;
};

#endif  // TRACKING_LIVETRACK24_COMMON_H_
