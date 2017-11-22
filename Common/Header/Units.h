/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Units.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#if !defined(__UNITS_H)
#define __UNITS_H

#include "externs.h"

#define UNITBITMAPNORMAL      0
#define UNITBITMAPINVERS      1
#define UNITBITMAPGRAY        2

// inHg to hPa
#define TOHPA 33.86417

typedef enum {
  cfDDMMSS=0,
  cfDDMMSSss,
  cfDDMMmmm,
  cfDDdddd,
  cfUTM
}CoordinateFormats_t;

typedef enum {
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
  unGradCelcius,                    // K = C� + 273,15
  unGradFahrenheit,                 // K = (�F + 459,67) / 1,8
  unLastUnit
}Units_t;


typedef enum {
  ugNone,
  ugDistance,
  ugAltitude,
  ugHorizontalSpeed,
  ugVerticalSpeed,
  ugWindSpeed,
  ugTaskSpeed,
  ugInvAltitude		// 100126
} UnitGroup_t;

typedef struct{
  const TCHAR * const Name;
  double  ToUserFact;
  double  ToUserOffset;
}UnitDescriptor_t;

class Units {

private:
  static UnitDescriptor_t UnitDescriptors[unGradFahrenheit+1];
  static Units_t UserDistanceUnit;
  static Units_t UserAltitudeUnit;
  static Units_t UserHorizontalSpeedUnit;
  static Units_t UserVerticalSpeedUnit;
  static Units_t UserWindSpeedUnit;
  static Units_t UserTaskSpeedUnit;
public:

  static CoordinateFormats_t CoordinateFormat;

  static const TCHAR *GetUnitName(Units_t Unit) gcc_pure;

  static Units_t GetUserDistanceUnit() gcc_pure;
  static Units_t SetUserDistanceUnit(Units_t NewUnit);

  static Units_t GetUserAltitudeUnit() gcc_pure;
  static Units_t GetUserInvAltitudeUnit() gcc_pure; // 100126
  static Units_t SetUserAltitudeUnit(Units_t NewUnit);

  static Units_t GetUserHorizontalSpeedUnit() gcc_pure;
  static Units_t SetUserHorizontalSpeedUnit(Units_t NewUnit);

  static Units_t GetUserTaskSpeedUnit() gcc_pure;
  static Units_t SetUserTaskSpeedUnit(Units_t NewUnit);

  static Units_t GetUserVerticalSpeedUnit() gcc_pure;
  static Units_t SetUserVerticalSpeedUnit(Units_t NewUnit);

  static Units_t GetUserWindSpeedUnit() gcc_pure;
  static Units_t SetUserWindSpeedUnit(Units_t NewUnit);

  static Units_t GetUserUnitByGroup(UnitGroup_t UnitGroup) gcc_pure;

  static void LongitudeToDMS(double Longitude, int *dd, int *mm, int *ss, bool *east) gcc_nonnull(2,3,4,5);
  static void LatitudeToDMS(double Latitude, int *dd, int *mm, int *ss, bool *north) gcc_nonnull(2,3,4,5);


  static bool CoordinateToString(double Longitude, double Latitude, TCHAR *Buffer, size_t size);

  template<size_t size>
  static bool CoordinateToString(double Longitude, double Latitude, TCHAR (&Buffer)[size]) {
    static_assert(size >= 33, "output Buffer size must be >= 33");
    return CoordinateToString(Longitude, Latitude, Buffer, size);
  }

  static bool LongitudeToString(double Longitude, TCHAR *Buffer, size_t size) gcc_nonnull(2);

  template<size_t size>
  static bool LongitudeToString(double Longitude, TCHAR (&Buffer)[size]) {
    static_assert(size >= 16, "output Buffer size must be >= 16");
    return LongitudeToString(Longitude, Buffer, size);
  }

  static bool LatitudeToString(double Latitude, TCHAR *Buffer, size_t size) gcc_nonnull(2);

  template<size_t size>
  static bool LatitudeToString(double Latitude, TCHAR (&Buffer)[size]) {
    static_assert(size >= 16, "output Buffer size must be >= 16");
    return LatitudeToString(Latitude, Buffer, size);
  }


  static void NotifyUnitChanged(void);

  static const TCHAR *GetHorizontalSpeedName() gcc_pure;
  static const TCHAR *GetVerticalSpeedName() gcc_pure;
  static const TCHAR *GetDistanceName() gcc_pure;
  static const TCHAR *GetAltitudeName() gcc_pure;
  static const TCHAR *GetInvAltitudeName() gcc_pure;
  static const TCHAR *GetTaskSpeedName() gcc_pure;
  static const TCHAR *GetWindSpeedName() gcc_pure;

  static bool FormatUserAltitude(double Altitude, TCHAR *Buffer, size_t size) gcc_nonnull(2);
  static bool FormatAlternateUserAltitude(double Altitude, TCHAR *Buffer, size_t size) gcc_nonnull(2);
  static bool FormatUserArrival(double Altitude, TCHAR *Buffer, size_t size) gcc_nonnull(2); // VENTA3
  static bool FormatUserDistance(double Distance, TCHAR *Buffer, size_t size) gcc_nonnull(2);
  static bool FormatUserMapScale(Units_t *Unit, double Distance, TCHAR *Buffer, size_t size) gcc_nonnull(3);


  /**
   * convert value from System Unit to @unit
   */
  static double ToUser(Units_t unit, double value) gcc_pure;

  /**
   * convert value from System Unit to User Unit
   */
  static double ToUserAltitude(double Altitude) gcc_pure;
  static double ToInvUserAltitude(double Altitude) gcc_pure;
  static double ToUserDistance(double Distance) gcc_pure;
  static double ToUserWindSpeed(double speed) gcc_pure;
  static double ToUserHorizontalSpeed(double speed) gcc_pure;
  static double ToUserVerticalSpeed(double speed) gcc_pure;
  static double ToUserTaskSpeed(double speed) gcc_pure;

  /**
   * convert value from @unit to System Unit
   */
  static double ToSys(Units_t unit, double value) gcc_pure;

  /**
   * convert value from User Unit to System Unit
   */
  static double ToSysAltitude(double Altitude) gcc_pure;
  static double ToSysDistance(double Distance) gcc_pure;
  static double ToSysWindSpped(double speed) gcc_pure;
  static double ToSysHorizontalSpeed(double speed) gcc_pure;
  static double ToSysVerticalSpeed(double speed) gcc_pure;
  static double ToSysTaskSpeed(double speed) gcc_pure;


  static void TimeToText(TCHAR* text, size_t cb, int d) gcc_nonnull(1);

  template<size_t N>
  static void TimeToText(TCHAR (&text)[N], int d) {
    TimeToText(text, N, d);
  }

  static void TimeToTextSimple(TCHAR* text, size_t cb, int d) gcc_nonnull(1);

  template<size_t N>
  static void TimeToTextSimple(TCHAR (&text)[N], int d) {
    TimeToTextSimple(text, N, d);
  }  

  static bool TimeToTextDown(TCHAR* text, int d) gcc_nonnull(1);
  static void TimeToTextS(TCHAR* text, int d) gcc_nonnull(1);
};

#endif
