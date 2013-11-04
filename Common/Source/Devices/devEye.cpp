/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"
#include "devEye.h"
#include "externs.h"

#include "utils/heapcheck.h"


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

const TCHAR *CDevEye::GetName()
{
  return(_T("Eye"));
}


bool CDevEye::PEYA(PDeviceDescriptor_t d, const TCHAR *sentence, NMEA_INFO *info)
{
  TAtmosphericInfo data = {0};
  unsigned fieldIdx = 0;
  bool status = true;
  double value;

  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.pStatic = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.pTotal = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.pAlt = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.qnh = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.windDirection = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.windSpeed = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.tas = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.vzp = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.oat = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.humidity = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.cloudBase = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.cloudTemp = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.groundTemp = value;
  
  if(status) {
    info->BaroAltitudeAvailable = true;
    info->BaroAltitude = data.pAlt;
    if(QNH != data.qnh) {
      QNH = data.qnh;
      CAirspaceManager::Instance().QnhChangeNotify(QNH);
    }
    
    info->AirspeedAvailable = true;
    info->TrueAirspeed = data.tas / TOKPH;
    info->IndicatedAirspeed = info->TrueAirspeed / AirDensityRatio(info->BaroAltitude);

    info->ExternalWindAvailable = true;
    info->ExternalWindSpeed = data.windSpeed / TOKPH;
    info->ExternalWindDirection = data.windDirection;
    
    info->VarioAvailable = true;
    info->Vario = data.vzp;

    info->TemperatureAvailable = true;
    info->OutsideAirTemperature = data.oat;

    info->HumidityAvailable = true;
    info->RelativeHumidity = data.humidity;
  }
  
  return status;
}


bool CDevEye::PEYI(PDeviceDescriptor_t d, const TCHAR *sentence, NMEA_INFO *info)
{
  TSpaceInfo data = {0};
  unsigned fieldIdx = 0;
  bool status = true;
  double value;
  
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.eulerRoll = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.eulerPitch = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.rollRate = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.pitchRate = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.yawRate = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.accelX = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.accelY = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.accelZ = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.virosbandometer = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.trueHeading = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.magneticHeading = value;
  if(status &= ParToDouble(sentence, fieldIdx++, &value))
    data.localDeclination = value;
  
  if(status) {
    info->GyroscopeAvailable = true;
    info->Pitch = data.eulerPitch;
    info->Roll = data.eulerRoll;
    
    info->MagneticHeadingAvailable = true;
    info->MagneticHeading = data.magneticHeading;
    
    info->AccelerationAvailable = true;
    info->AccelX = data.accelX;
    info->AccelY = data.accelY;
    info->AccelZ = data.accelZ;
  }
  
  return status;
}


BOOL CDevEye::ParseNMEA(PDeviceDescriptor_t d, TCHAR *sentence, NMEA_INFO *info)
{

  if (!NMEAParser::NMEAChecksum(sentence) || (info == NULL)){
    return FALSE;
  }

  if(_tcsncmp(_T("$PEYA"), sentence, 5) == 0)
    return PEYA(d, sentence + 6, info);
  else if(_tcsncmp(_T("$PEYI"), sentence, 5) == 0)
    return PEYI(d, sentence + 6, info);
  
  return FALSE;
}


BOOL CDevEye::Install(PDeviceDescriptor_t d)
{
  _tcscpy(d->Name, GetName());
  d->ParseNMEA    = ParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs      = NULL;
  d->PutBallast   = NULL;
  d->Open         = NULL;
  d->Close        = NULL;
  d->Init         = NULL;
  d->LinkTimeout  = NULL;
  d->Declare      = NULL;
  d->IsLogger     = NULL;
  d->IsGPSSource  = GetTrue;
  d->IsBaroSource = GetTrue;
  
  return TRUE;
}


bool CDevEye::Register()
{
  return devRegister(GetName(), cap_gps | cap_baro_alt | cap_speed | cap_vario | cap_wind, Install);
}
