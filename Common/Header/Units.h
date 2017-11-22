/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Units.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#if !defined(__UNITS_H)
#define __UNITS_H

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


enum UnitGroup_t {
  ugNone,
  ugDistance,
  ugAltitude,
  ugHorizontalSpeed,
  ugVerticalSpeed,
  ugWindSpeed,
  ugTaskSpeed,
  ugInvAltitude		// 100126
};

namespace Units {

  extern CoordinateFormats_t CoordinateFormat;

  const TCHAR *GetUnitName(Units_t Unit) gcc_pure;

  Units_t GetUserDistanceUnit() gcc_pure;
  Units_t SetUserDistanceUnit(Units_t NewUnit);

  Units_t GetUserAltitudeUnit() gcc_pure;
  Units_t GetUserInvAltitudeUnit() gcc_pure; // 100126
  Units_t SetUserAltitudeUnit(Units_t NewUnit);

  Units_t GetUserHorizontalSpeedUnit() gcc_pure;
  Units_t SetUserHorizontalSpeedUnit(Units_t NewUnit);

  Units_t GetUserTaskSpeedUnit() gcc_pure;
  Units_t SetUserTaskSpeedUnit(Units_t NewUnit);

  Units_t GetUserVerticalSpeedUnit() gcc_pure;
  Units_t SetUserVerticalSpeedUnit(Units_t NewUnit);

  Units_t GetUserWindSpeedUnit() gcc_pure;
  Units_t SetUserWindSpeedUnit(Units_t NewUnit);

  Units_t GetUserUnitByGroup(UnitGroup_t UnitGroup) gcc_pure;

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

  inline const TCHAR* GetHorizontalSpeedName() {
    return GetUnitName(GetUserHorizontalSpeedUnit());
  }

  inline const TCHAR* GetVerticalSpeedName() {
    return GetUnitName(GetUserVerticalSpeedUnit());
  }

  inline const TCHAR* GetDistanceName() {
    return GetUnitName(GetUserDistanceUnit());
  }

  inline const TCHAR* GetAltitudeName() {
    return GetUnitName(GetUserAltitudeUnit());
  }

  inline const TCHAR* GetInvAltitudeName() {
    return GetUnitName(GetUserInvAltitudeUnit());
  }

  inline const TCHAR* GetTaskSpeedName() {
    return GetUnitName(GetUserTaskSpeedUnit());
  }

  inline const TCHAR* GetWindSpeedName() {
    return GetUnitName(GetUserWindSpeedUnit());
  }

  void FormatUserAltitude(double Altitude, TCHAR *Buffer, size_t size) gcc_nonnull(2);
  void FormatAlternateUserAltitude(double Altitude, TCHAR *Buffer, size_t size) gcc_nonnull(2);
  void FormatUserArrival(double Altitude, TCHAR *Buffer, size_t size) gcc_nonnull(2); // VENTA3
  void FormatUserDistance(double Distance, TCHAR *Buffer, size_t size) gcc_nonnull(2);
  bool FormatUserMapScale(Units_t *Unit, double Distance, TCHAR *Buffer, size_t size) gcc_nonnull(3);


  /**
   * convert value from System Unit to @unit
   */
  double ToUser(Units_t unit, double value) gcc_pure;

  /**
   * convert value from System Unit to User Unit
   */
  inline double ToUserAltitude(double Altitude) {
    return ToUser(GetUserAltitudeUnit(), Altitude); 
  }

  inline double ToInvUserAltitude(double Altitude) {
    return ToUser(GetUserInvAltitudeUnit(), Altitude);
  }

  inline double ToUserDistance(double Distance) {
    return ToUser(GetUserDistanceUnit(), Distance);
  }

  inline double ToUserWindSpeed(double speed) {
    return ToUser(GetUserWindSpeedUnit(), speed);
  }

  inline double ToUserHorizontalSpeed(double speed) {
    return ToUser(GetUserHorizontalSpeedUnit(), speed);
  }

  inline double ToUserVerticalSpeed(double speed) {
    return ToUser(GetUserVerticalSpeedUnit(), speed);
  }

  inline double ToUserTaskSpeed(double speed) {
    return ToUser(GetUserTaskSpeedUnit(), speed);
  }

  /**
   * convert value from @unit to System Unit
   */
  double ToSys(Units_t unit, double value) gcc_pure;

  /**
   * convert value from User Unit to System Unit
   */
  inline double ToSysAltitude(double Altitude) {
    return ToSys(GetUserAltitudeUnit(), Altitude);
  }

  inline double ToSysDistance(double Distance) {
    return ToSys(GetUserDistanceUnit(), Distance);
  }

  inline double ToSysWindSpped(double speed) {
    return ToSys(GetUserWindSpeedUnit(), speed);
  }

  inline double ToSysHorizontalSpeed(double speed) {
    return ToSys(GetUserHorizontalSpeedUnit(), speed);
  }

  inline double ToSysVerticalSpeed(double speed) {
    return ToSys(GetUserVerticalSpeedUnit(), speed);
  }

  inline double ToSysTaskSpeed(double speed) {
    return ToSys(GetUserTaskSpeedUnit(), speed);
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
