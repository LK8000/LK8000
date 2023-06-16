/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   devXCVario.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 30 may 2023
 */
#include "externs.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "Util/Clamp.hpp"
#include "Comm/UpdateQNH.h"
#include "devXCVario.h"

using std::string_view_literals::operator""sv;

namespace {

bool ReadChecked(TCHAR* String, double& value_r) {
  if (!String || _tcslen(String) == 0) {
    return false;  // empty or empty string
  }
  value_r = StrToDouble(String, nullptr);
  return true;
}

bool ReadChecked(TCHAR* String, int& value_r) {
  if (!String || _tcslen(String) == 0) {
    return false;  // empty or empty string
  }
  value_r = _tcstol(String, nullptr, 10);
  return true;
}

BOOL PXCV(PDeviceDescriptor_t d, TCHAR** params, size_t nparams, NMEA_INFO* pGPS) {
  /*
  Sentence has following format:
  $PXCV,
  BBB.B, // Vario, -30 to +30 m/s, negative sign for sink
  C.C, // MacCready 0 to 10 m/s
  EE, // Bugs degradation, 0 = clean to 30 %
  F.FF, // Ballast 1.00 to 1.60
  G, // 1 in climb, 0 in cruise, Note: Original Borgelt docushows vice versa
  HH.H, // Outside airtemp in degrees celcius ( may have leading negative sign )
  QQQQ.Q, // QNH e.g. 1013.2
  PPPP.P, // Static pressure in hPa
  QQQQ.Q, // Dynamic pressure in Pa
  RRR.R, // Roll angle
  III.I, // Pitch angle
  X.XX, // Acceleration in X-Axis
  Y.YY, // Acceleration in Y-Axis
  Z.ZZ, // Acceleration in Z-Axis
  *CHK = standard NMEA checksum
  <CR><LF>
  */

  double value;
  if (ReadChecked(params[1], value)) {
    // Vario, -30 to +30 m/s, negative sign for sink
    UpdateVarioSource(*pGPS, *d, value);
  }

  if (ReadChecked(params[2], value)) {
    // MacCready 0 to 10 m/s
    d->RecvMacCready(value);
  }
  if (ReadChecked(params[3], value)) {
    // Bugs degradation, 0 = clean to 30 %
    d->RecvBugs(1 - Clamp(value, 0., 30.) / 100.);
  }

  /**
   * unusable
  if (ReadChecked(params[4], value)) {
    // Ballast 1.00 to 1.60
    d->RecvBallast(value);
  }
  */

  int ival;
  if (ReadChecked(params[5], ival)) {
    // 1 in climb, 0 in cruise, Note: Original Borgelt docushows vice versa
    if (ival == 1) {
      ExternalTriggerCruise = true;
      ExternalTriggerCircling = false;
      MapWindow::mode.UserForcedMode(MapWindow::Mode::MODE_FLY_CRUISE);
    } else {
      ExternalTriggerCruise = false;
      ExternalTriggerCircling = true;
      MapWindow::mode.UserForcedMode(MapWindow::Mode::MODE_FLY_CIRCLING);
    }
  }

  if (ReadChecked(params[6], value)) {
    // Outside airtemp in degrees celcius ( may have leading negative sign )
    pGPS->TemperatureAvailable = true;
    pGPS->OutsideAirTemperature = value;
  }

  if (ReadChecked(params[7], value)) {
    // QNH e.g. 1013.2
    UpdateQNH(value);
  }

  double dyn_press, static_press;
  if (ReadChecked(params[8], static_press)
      && ReadChecked(params[9], dyn_press)) 
  {
    // Static && Dynamic pressure in hPa
    pGPS->AirspeedAvailable = true;
    pGPS->IndicatedAirspeed = sqrt(163.2653061 * dyn_press / 100.);
    double qne_alt = StaticPressureToQNEAltitude(static_press);
    pGPS->TrueAirspeed = TrueAirSpeed(pGPS->IndicatedAirspeed, qne_alt);
  }

  double Roll, Pitch;
  if (ReadChecked(params[10], Roll)
      && ReadChecked(params[11], Pitch))
  {
    pGPS->GyroscopeAvailable = true;
    pGPS->Roll = Roll;
    pGPS->Pitch = Pitch;
  }

  double x, y, z;
  if (ReadChecked(params[12], x) 
      && ReadChecked(params[13], y) 
      && ReadChecked(params[14], z))
  {
    // Acceleration in X-Y-Z Axis
    pGPS->AccelerationAvailable = true;
    pGPS->AccelX = x;
    pGPS->AccelY = y;
    pGPS->AccelZ = z;
  }

  return TRUE;
}

BOOL XCV(PDeviceDescriptor_t d, TCHAR** params, size_t nparams, NMEA_INFO* pGPS) {
  if (params[1] == _T("bal-water"sv)) {
    double liters;
    if (ReadChecked(params[2], liters)) {
      d->RecvBallast(WEIGHTS[2] / liters);
    }
  }
  return TRUE;
}

BOOL ParseNMEA(DeviceDescriptor_t* d, TCHAR* String, NMEA_INFO* pGPS) {
  if (!pGPS) {
    return FALSE;
  }
  TCHAR ctemp[MAX_NMEA_LEN];
  TCHAR* params[MAX_NMEA_PARAMS];

  size_t n_params = NMEAParser::ValidateAndExtract(String, ctemp, MAX_NMEA_LEN, params, MAX_NMEA_PARAMS);
  if (n_params > 0) {
    if (params[0] == _T("$PXCV"sv)) {
      return PXCV(d, params, n_params, pGPS);
    }
    if (params[0] == _T("!xcv"sv)) {
      return XCV(d, params, n_params, pGPS);
    }
  }
  return FALSE;
}

BOOL PutMacCready(DeviceDescriptor_t* d, double McReady) {
  char szTmp[32];
  sprintf(szTmp, "!g,m%d\r", iround(McReady / KNOTSTOMETRESSECONDS * 10));
  d->Com->WriteString(szTmp);
  return TRUE;
}

BOOL PutBugs(DeviceDescriptor_t* d, double Bugs) {
  char szTmp[32];
  sprintf(szTmp, "!g,u%d\r", iround(Bugs * 100));
  d->Com->WriteString(szTmp);
  return TRUE;
}

BOOL PutBallast(DeviceDescriptor_t* d, double Ballast) {
  char  szTmp[32];
  sprintf(szTmp, "!g,b%.3f\r", Ballast * 10.);
  d->Com->WriteString(szTmp);
  return TRUE;
}

BOOL PutQNH(DeviceDescriptor_t* d, double NewQNH) {
  char  szTmp[32];
  sprintf(szTmp, "!g,q%d\r", iround(NewQNH));
  d->Com->WriteString(szTmp);
  return TRUE;
}

bool PutVersion(DeviceDescriptor_t* d) {
  return d && d->Com && d->Com->WriteString("!xcs,version,2*20\r\n");
}

BOOL Open(DeviceDescriptor_t* d) {
  PutVersion(d);

  PutMacCready(d, MACCREADY);
  PutBugs(d, BUGS);
  PutBallast(d, BALLAST);
  PutQNH(d, QNH);

  return TRUE;
}

}  // namespace

void XCVario::Install(DeviceDescriptor_t* d) {
  _tcscpy(d->Name, Name);
  d->Open = Open;
  d->ParseNMEA = ParseNMEA;
  d->PutMacCready = PutMacCready;
  d->PutBugs = PutBugs;
  d->PutBallast = PutBallast;
  d->PutQNH = PutQNH;
}
