/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: devPVCOM.h
 */

#ifndef DEV_PVCOM_H
#define DEV_PVCOM_H

#include "Devices/DeviceRegister.h"

struct TAtmosphericInfo {
  double pStatic;
  double pTotal;
  double pAlt;
  double qnh;
  double windDirection;
  double windSpeed;
  double tas;
  double vzp;
  double oat;
  double humidity;
  double cloudBase;
  double cloudTemp;
  double groundTemp;
};

struct TSpaceInfo {
  double eulerRoll;
  double eulerPitch;
  double rollRate;
  double pitchRate;
  double yawRate;
  double accelX;
  double accelY;
  double accelZ;
  double virosbandometer;
  double trueHeading;
  double magneticHeading;
  double localDeclination;
};

void PVCOMInstall(PDeviceDescriptor_t d);

inline constexpr
DeviceRegister_t PVCOMRegister(void){
  return devRegister( _T("PVCOM"), (1l << dfRadio), PVCOMInstall);
}

#endif
