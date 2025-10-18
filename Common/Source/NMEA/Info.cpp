
/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Info.cpp
 */

#include "Info.h"

void NMEA_INFO::reset_availability(std::optional<unsigned> idx) {
  BaroAltitude.reset(idx);
  Vario.reset(idx);
  OutsideAirTemperature.reset(idx);
  RelativeHumidity.reset(idx);
  NettoVario.reset(idx);
  Gload.reset(idx);
  HeartRate.reset(idx);
  MagneticHeading.reset(idx);
  ExternalWind.reset(idx);
  Acceleration.reset(idx);
  IndicatedAirSpeed.reset(idx);
  TrueAirSpeed.reset(idx);
}
