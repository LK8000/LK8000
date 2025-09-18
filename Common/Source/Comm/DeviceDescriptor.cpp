/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   DeviceDescriptor.cpp
 */

#include "options.h"
#include "DeviceDescriptor.h"
#include "Utils.h"
#include "ComPort.h"
#include <cassert>
#include <array>

std::array<DeviceDescriptor_t, NUMDEV> DeviceList = make_device_list<NUMDEV>();

void DeviceDescriptor_t::Reset() {
  Name[0] = '\0';

  DirectLink = nullptr;
  ParseNMEA = nullptr;
  ParseStream = nullptr;
  PutMacCready = nullptr;
  PutBugs = nullptr;
  PutBallast = nullptr;
  PutVolume = nullptr;
  PutRadioMode = nullptr;
  PutSquelch = nullptr;
  PutFreqActive = nullptr;
  StationSwap = nullptr;
  PutFreqStandby = nullptr;
  Open = nullptr;
  Close = nullptr;
  LinkTimeout = nullptr;
  Declare = nullptr;

  PutQNH = nullptr;
  OnSysTicker = nullptr;
  Config = nullptr;
  HeartBeat = nullptr;
  NMEAOut = nullptr;
  PutTarget = nullptr;

  OnHeartRate = nullptr;
  OnBarometricPressure = nullptr;
  OnOutsideTemperature = nullptr;
  OnRelativeHumidity = nullptr;
  OnWindOriginDirection = nullptr;
  OnWindSpeed = nullptr;
  OnBatteryLevel = nullptr;

  DoEnableGattCharacteristic = nullptr;
  OnGattCharacteristic = nullptr;

  Disabled = true;

  SerialNumber = {};

  IsBaroSource = false;
  IsRadio = false;

  HB = 0;  // counter

  SharedPortNum.reset();
  m_bAdvancedMode = false;

#ifdef DEVICE_SERIAL
  HardwareId = 0;
  SoftwareVer = 0;
#endif

  nmeaParser.Reset();

  IgnoreMacCready.Reset();
  IgnoreBugs.Reset();
  IgnoreBallast.Reset();

  driver_data = {};
}

bool DeviceDescriptor_t::IsReady() const {
  if (Disabled) {
    return false;
  }
  return Com && Com->IsReady();
}

bool DeviceDescriptor_t::IsGPS() const {
  if (Disabled) {
    return false;
  }
  return IsReady() && nmeaParser.connected;
}

BOOL DeviceDescriptor_t::_PutMacCready(double McReady) {
  if (PutMacCready) {
    IgnoreMacCready.Update();
    return PutMacCready(this, McReady);
  }
  return FALSE;
}

BOOL DeviceDescriptor_t::_PutBugs(double Bugs) {
  if (PutBugs) {
    IgnoreBugs.Update();
    return PutBugs(this, Bugs);
  }
  return FALSE;
}

BOOL DeviceDescriptor_t::_PutBallast(double Ballast) {
  if (PutBallast) {
    IgnoreBallast.Update();
    return PutBallast(this, Ballast);
  }
  return FALSE;
}

BOOL DeviceDescriptor_t::_PutVolume(int Volume) {
  return PutVolume && PutVolume(this, Volume);
}

BOOL DeviceDescriptor_t::_PutRadioMode(int mode) {
  return PutRadioMode && PutRadioMode(this, mode);
}

BOOL DeviceDescriptor_t::_PutSquelch(int Squelch) {
  return PutSquelch && PutSquelch(this, Squelch);
}

BOOL DeviceDescriptor_t::_PutFreqActive(unsigned khz,
                                        const TCHAR* StationName) {
  return PutFreqActive && PutFreqActive(this, khz, StationName);
}

BOOL DeviceDescriptor_t::_StationSwap() {
  return StationSwap && StationSwap(this);
}

BOOL DeviceDescriptor_t::_PutFreqStandby(unsigned khz,
                                         const TCHAR* StationName) {
  return PutFreqStandby && PutFreqStandby(this, khz, StationName);
}

BOOL DeviceDescriptor_t::_PutTarget(const WAYPOINT& wpt) {
  return PutTarget && PutTarget(this, wpt);
}

BOOL DeviceDescriptor_t::_SendData(const NMEA_INFO& Basic,
                                   const DERIVED_INFO& Calculated) {
  return SendData && SendData(this, Basic, Calculated);
}

BOOL DeviceDescriptor_t::_PutQNH(double NewQNH) {
  return PutQNH && PutQNH(this, NewQNH);
}

BOOL DeviceDescriptor_t::_LinkTimeout() {
  return LinkTimeout && LinkTimeout(this);
}

BOOL DeviceDescriptor_t::_HeartBeat() {
  return HeartBeat && HeartBeat(this);
}

BOOL DeviceDescriptor_t::RecvMacCready(double McReady) {
  if (IgnoreMacCready.Check(5000)) {
    CheckSetMACCREADY(McReady, this);
    return TRUE;
  }
  return FALSE;
}

BOOL DeviceDescriptor_t::RecvBugs(double Bugs) {
  if (IgnoreBugs.Check(5000)) {
    CheckSetBugs(Bugs, this);
    return TRUE;
  }
  return FALSE;
}

BOOL DeviceDescriptor_t::RecvBallast(double Ballast) {
  if (IgnoreBallast.Check(5000)) {
    CheckSetBallast(Ballast, this);
    return TRUE;
  }
  return FALSE;
}
