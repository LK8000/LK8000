/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Units.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#if !defined(__UNITS_H)
#define __UNITS_H

#include "Compiler.h"
#include "tchar.h"

namespace settings {
    class writer;
}

// inHg to hPa
#define TOHPA 33.86417

enum CoordinateFormats_t {
  cfDDMMSS=0,
  cfDDMMSSss,
  cfDDMMmmm,
  cfDDdddd,
  cfUTM
};

enum Units_t {
  unUndef,
  unKiloMeter,
  unNauticalMiles,
  unStatuteMiles,
  unKiloMeterPerHour,
  unKnots,
  unStatuteMilesPerHour,
  unMeterPerSecond,
  unFeetPerMinutes,
  unMeter,
  unFeet,
  unFligthLevel,
  unKelvin,
  unGradCelcius,    // K = °C + 273,15
  unGradFahrenheit, // K = (°F + 459,67) / 1,8
  unFeetPerSecond,
  unLastUnit // must be the last
};

namespace Units {

  // Units configurable in system config
  extern int SpeedUnit_Config;
  extern int TaskSpeedUnit_Config;
  extern int DistanceUnit_Config;
  extern int VerticalSpeedUnit_Config;
  extern int AltitudeUnit_Config;
  extern int LatLonUnits_Config;


  void ResetSettings();
  bool LoadSettings(const char *key, const char *value);
  void SaveSettings(settings::writer& write_settings);

  CoordinateFormats_t GetCoordinateFormat() gcc_pure;

  Units_t GetDistanceUnit() gcc_pure;
  Units_t GetAltitudeUnit() gcc_pure;
  Units_t GetAlternateAltitudeUnit() gcc_pure; // 100126
  Units_t GetHorizontalSpeedUnit() gcc_pure;
  Units_t GetTaskSpeedUnit() gcc_pure;
  Units_t GetVerticalSpeedUnit() gcc_pure;
  Units_t GetWindSpeedUnit() gcc_pure;

  void LongitudeToDMS(double Longitude, int *dd, int *mm, int *ss, bool *east) gcc_nonnull(2,3,4,5);
  void LatitudeToDMS(double Latitude, int *dd, int *mm, int *ss, bool *north) gcc_nonnull(2,3,4,5);

  bool CoordinateToString(double Longitude, double Latitude, TCHAR *Buffer, size_t size) gcc_nonnull(3);

  template<size_t size>
  inline bool CoordinateToString(double Longitude, double Latitude, TCHAR (&Buffer)[size]) {
    static_assert(size >= 33, "output Buffer size must be >= 33");
    return CoordinateToString(Longitude, Latitude, Buffer, size);
  }

  bool LatitudeToString(double Latitude, TCHAR *Buffer, size_t size) gcc_nonnull(2);

  template<size_t size>
  inline bool LatitudeToString(double Latitude, TCHAR (&Buffer)[size]) {
    static_assert(size >= 16, "output Buffer size must be >= 16");
    return LatitudeToString(Latitude, Buffer, size);
  }

  bool LongitudeToString(double Longitude, TCHAR *Buffer, size_t size) gcc_nonnull(2);

  template<size_t size>
  inline bool LongitudeToString(double Longitude, TCHAR (&Buffer)[size]) {
    static_assert(size >= 16, "output Buffer size must be >= 16");
    return LongitudeToString(Longitude, Buffer, size);
  }

  void NotifyUnitChanged();

  const TCHAR *GetName(Units_t Unit) gcc_pure;
  
  inline const TCHAR* GetHorizontalSpeedName() {
    return GetName(GetHorizontalSpeedUnit());
  }

  inline const TCHAR* GetVerticalSpeedName() {
    return GetName(GetVerticalSpeedUnit());
  }

  inline const TCHAR* GetDistanceName() {
    return GetName(GetDistanceUnit());
  }

  inline const TCHAR* GetAltitudeName() {
    return GetName(GetAltitudeUnit());
  }

  inline const TCHAR* GetAlternateAltitudeName() {
    return GetName(GetAlternateAltitudeUnit());
  }

  inline const TCHAR* GetTaskSpeedName() {
    return GetName(GetTaskSpeedUnit());
  }

  inline const TCHAR* GetWindSpeedName() {
    return GetName(GetWindSpeedUnit());
  }

  void FormatAltitude(double Altitude, TCHAR *Buffer, size_t size) gcc_nonnull(2);
  void FormatAlternateAltitude(double Altitude, TCHAR *Buffer, size_t size) gcc_nonnull(2);
  void FormatArrival(double Altitude, TCHAR *Buffer, size_t size) gcc_nonnull(2); // VENTA3
  void FormatDistance(double Distance, TCHAR *Buffer, size_t size) gcc_nonnull(2);
  void FormatMapScale(double Distance, TCHAR *Buffer, size_t size) gcc_nonnull(2);


  /**
   * convert value from System Unit to @unit
   */
  double To(Units_t unit, double value) gcc_pure;

  /**
   * convert value from System Unit to User Unit
   */
  inline double ToAltitude(double Altitude) {
    return To(GetAltitudeUnit(), Altitude); 
  }

  inline double ToAlternateAltitude(double Altitude) {
    return To(GetAlternateAltitudeUnit(), Altitude);
  }

  inline double ToDistance(double Distance) {
    return To(GetDistanceUnit(), Distance);
  }

  inline double ToWindSpeed(double speed) {
    return To(GetWindSpeedUnit(), speed);
  }

  inline double ToHorizontalSpeed(double speed) {
    return To(GetHorizontalSpeedUnit(), speed);
  }

  inline double ToVerticalSpeed(double speed) {
    return To(GetVerticalSpeedUnit(), speed);
  }

  inline double ToTaskSpeed(double speed) {
    return To(GetTaskSpeedUnit(), speed);
  }

  /**
   * convert value from @unit to System Unit
   */
  double From(Units_t unit, double value) gcc_pure;

  /**
   * convert value from User Unit to System Unit
   */
  inline double FromAltitude(double Altitude) {
    return From(GetAltitudeUnit(), Altitude);
  }

  inline double FromDistance(double Distance) {
    return From(GetDistanceUnit(), Distance);
  }

  inline double FromWindSpped(double speed) {
    return From(GetWindSpeedUnit(), speed);
  }

  inline double FromHorizontalSpeed(double speed) {
    return From(GetHorizontalSpeedUnit(), speed);
  }

  inline double FromVerticalSpeed(double speed) {
    return From(GetVerticalSpeedUnit(), speed);
  }

  inline double FromTaskSpeed(double speed) {
    return From(GetTaskSpeedUnit(), speed);
  }


  void TimeToText(TCHAR* text, size_t cb, int d) gcc_nonnull(1);

  template<size_t N>
  inline void TimeToText(TCHAR (&text)[N], int d) {
    TimeToText(text, N, d);
  }

  void TimeToTextSimple(TCHAR* text, size_t cb, int d) gcc_nonnull(1);

  template<size_t N>
  inline void TimeToTextSimple(TCHAR (&text)[N], int d) {
    TimeToTextSimple(text, N, d);
  }  

  bool TimeToTextDown(TCHAR* text, size_t cb, int d) gcc_nonnull(1);

  template<size_t N>
  inline bool TimeToTextDown(TCHAR (&text)[N], int d) {
    return TimeToTextDown(text, N, d);
  }

  void TimeToTextS(TCHAR* text, size_t cb, int d) gcc_nonnull(1);

  template<size_t N>
  inline void TimeToTextS(TCHAR (&text)[N], int d) {
    TimeToTextS(text, N, d);
  }
}

#endif
