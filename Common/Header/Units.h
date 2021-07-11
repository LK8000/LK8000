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

  static const TCHAR *GetUnitName(Units_t Unit);

  static Units_t GetUserDistanceUnit(void);
  static Units_t SetUserDistanceUnit(Units_t NewUnit);

  static Units_t GetUserAltitudeUnit(void);
  static Units_t GetUserInvAltitudeUnit(void); // 100126
  static Units_t SetUserAltitudeUnit(Units_t NewUnit);

  static Units_t GetUserHorizontalSpeedUnit(void);
  static Units_t SetUserHorizontalSpeedUnit(Units_t NewUnit);

  static Units_t GetUserTaskSpeedUnit(void);
  static Units_t SetUserTaskSpeedUnit(Units_t NewUnit);

  static Units_t GetUserVerticalSpeedUnit(void);
  static Units_t SetUserVerticalSpeedUnit(Units_t NewUnit);

  static Units_t GetUserWindSpeedUnit(void);
  static Units_t SetUserWindSpeedUnit(Units_t NewUnit);

  static Units_t GetUserUnitByGroup(UnitGroup_t UnitGroup);

  static void LongitudeToDMS(double Longitude,
                             int *dd,
                             int *mm,
                             int *ss,
                             bool *east);
  static void LatitudeToDMS(double Latitude,
                            int *dd,
                            int *mm,
                            int *ss,
                            bool *north);

  static bool CoordinateToString(double Longitude, double Latitude, TCHAR *Buffer, size_t size);

  template<size_t size>
  static bool CoordinateToString(double Longitude, double Latitude, TCHAR (&Buffer)[size]) {
    static_assert(size >= 33, "output Buffer size must be >= 33");
    return CoordinateToString(Longitude, Latitude, Buffer, size);
  }

  static bool LongitudeToString(double Longitude, TCHAR *Buffer, size_t size);

  template<size_t size>
  static bool LongitudeToString(double Longitude, TCHAR (&Buffer)[size]) {
    static_assert(size >= 16, "output Buffer size must be >= 16");
    return LongitudeToString(Longitude, Buffer, size);
  }

  static bool LatitudeToString(double Latitude, TCHAR *Buffer, size_t size);

  template<size_t size>
  static bool LatitudeToString(double Latitude, TCHAR (&Buffer)[size]) {
    static_assert(size >= 16, "output Buffer size must be >= 16");
    return LatitudeToString(Latitude, Buffer, size);
  }


  static void NotifyUnitChanged(void);

  static const TCHAR *GetHorizontalSpeedName();

  static const TCHAR *GetVerticalSpeedName();

  static const TCHAR *GetDistanceName();

  static const TCHAR *GetAltitudeName();
  static const TCHAR *GetInvAltitudeName();

  static const TCHAR *GetTaskSpeedName();

  static bool FormatUserAltitude(double Altitude, TCHAR *Buffer, size_t size);
  static bool FormatAlternateUserAltitude(double Altitude, TCHAR *Buffer, size_t size);
  static bool FormatUserArrival(double Altitude, TCHAR *Buffer, size_t size); // VENTA3
  static bool FormatUserDistance(double Distance, TCHAR *Buffer, size_t size);
  static bool FormatUserMapScale(Units_t *Unit, double Distance, TCHAR *Buffer, size_t size);

  static double ToUserAltitude(double Altitude);
  static double ToSysAltitude(double Altitude);

  static double ToUserDistance(double Distance);
  static double ToSysDistance(double Distance);
  static void TimeToText(TCHAR* text, int d);
  static void TimeToTextSimple(TCHAR* text, int d);
  static bool TimeToTextDown(TCHAR* text, int d);
  static void TimeToTextS(TCHAR* text, int d);

};

#endif
