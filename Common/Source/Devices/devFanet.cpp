/*
 *  LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 *  Released under GNU/GPL License v.2
 *  See CREDITS.TXT file for authors and copyrights
 *
 *  File:   devFanet.h
 *  Author: Gerald Eichler
 *
 *  Created on 13 march 2020, 14:45
 */



#include "externs.h"
#include "Baro.h"
#include "devFanet.h"
#include "Fanet.h"
#include "Parser.h"

static BOOL FanetParse(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS);

BOOL FanetParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){
    //StartupStore(_T("... FANETParse %s"),String);

  (void)d;

  if (pGPS == NULL){
    return FALSE;
  }

  if(_tcsncmp(TEXT("#FNF"), String, 4)==0)
    {
      StartupStore(_T("... FANETMsg %s"),String);
      return FanetParse(d, &String[5], pGPS);      
    }
  return FALSE;

}

static BOOL FanetInstall(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("FANET"));
  d->ParseNMEA = FanetParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = NULL;
  d->Declare = NULL;
  d->IsGPSSource = NULL;
  d->IsBaroSource = NULL;

  return(TRUE);

}


BOOL FanetRegister(void){
  return(devRegister(
    TEXT("FANET"),
    (1l << dfBaroAlt)
    | (1l << dfVario),
    FanetInstall
  ));
}


uint8_t getByteFromHex(TCHAR *in){
  int tens = 0;
  int digits = 0;
   
  if (!isxdigit(in[0]))   // Valid hex digit character?
    return -1;

  in[0] = toupper(in[0]);   // Use upper case
  if (!isxdigit(in[1])){
    digits = in[0] >= 'A' ? (in[0] - 'A' + 10) : in[0] - '0'; //we have only one Hex-Sign --> only digits (no tens)
  }else{
    in[1] = toupper(in[1]); 
    tens = in[0] >= 'A' ? (in[0] - 'A' + 10) : in[0] - '0';
    digits = in[1] >= 'A' ? (in[1] - 'A' + 10) : in[1] - '0';

  }
  return tens * 16 + digits;
}

/* ------------------------------------------------------------------------- */
/*
 *  Created on: 06 Dec 2017
 *      Author: Linar Yusupov
 */
static void payload_absolut2coord(double *lat, double *lon, uint8_t *buf)
{
  int32_t lat_i = 0;
  int32_t lon_i = 0;

  if(buf == NULL || lat == NULL || lon == NULL)
    return;

  ((uint8_t*)&lat_i)[0] = buf[0];
  ((uint8_t*)&lat_i)[1] = buf[1];
  ((uint8_t*)&lat_i)[2] = buf[2];

  ((uint8_t*)&lon_i)[0] = buf[3];
  ((uint8_t*)&lon_i)[1] = buf[4];
  ((uint8_t*)&lon_i)[2] = buf[5];

  *lat = (double) lat_i / 93206.0d;
  *lon = (double) lon_i / 46603.0d;
}

static INT FanetGetWeatherIndex(FANET_WEATHER *weather,NMEA_INFO *pGPS){
  int iEmpyIndex = -1;
  for (int i = 0;i < MAXFANETWEATHER;i++){
    if (pGPS->FANET_Weather[i].Time_Fix == 0){
      if (iEmpyIndex < 0){
        iEmpyIndex = i;
      }
    }else if ((pGPS->FANET_Weather[i].Cn[0] == weather->Cn[0]) && (pGPS->FANET_Weather[i].Cn[1] == weather->Cn[1]) && (pGPS->FANET_Weather[i].Cn[2] == weather->Cn[2])){
      //we found the right entry (same cn)
      return i;
    }
  }
  return iEmpyIndex;
}

static BOOL FanetInsertWeatherData(FANET_WEATHER *weather,NMEA_INFO *pGPS){
  int index = FanetGetWeatherIndex(weather,pGPS);
  StartupStore(_T("... FanetgetWeatheSlot %d"),index);
  if (index < 0){
    return FALSE;
  }
  pGPS->FANET_Weather[index] = *weather; //copy data to structure
  pGPS->FANET_Weather[index].Time_Fix = pGPS->Time; //set time for clearing after nothing is received  
  StartupStore(_T("... FanetWeatherdata ready %0.f"),(float)pGPS->FANET_Weather[index].Time_Fix);
  return TRUE;
}

static BOOL FanetParseType4Msg(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){
  TCHAR ctemp[80];
  FANET_WEATHER weather;

  StartupStore(_T("... FanetParseType4Msg"));

  NMEAParser::ExtractParameter(String,ctemp,0);
  weather.Cn[0] = getByteFromHex(ctemp);

  NMEAParser::ExtractParameter(String,ctemp,1);
  weather.Cn[1] = getByteFromHex(&ctemp[0]);
  weather.Cn[2] = getByteFromHex(&ctemp[2]);

  NMEAParser::ExtractParameter(String,ctemp,5);
  uint8_t payloadLen = getByteFromHex(ctemp);
  uint8_t msg[payloadLen];

  NMEAParser::ExtractParameter(String,ctemp,6);
  for (int i = 0;i < payloadLen;i++){
    msg[i] = getByteFromHex(&ctemp[i*2]);
  }

  int index = 1;
  if (msg[0] & 0x01){ //check extended header
    index++;
  }
  payload_absolut2coord(&weather.Latitude,&weather.Longitude,&msg[index]);
  index+=6;
  if ((msg[0] >> 6) & 0x01){
    //Temperature (+1byte in 0.5 degree, 2-Complement)
    weather.temp = (float)((int8_t)msg[index])/2;
    index++;
  }else{
    weather.temp = 0;
  }
  if ((msg[0] >> 5) & 0x01){
    //Wind (+3byte: 1byte Heading in 360/256 degree, 1byte speed and 1byte gusts in 0.2km/h (each: bit 7 scale 5x or 1x, bit 0-6))
    weather.windDir =  ((float)msg[index]) * 360.0 / 256.0;
    index++;
    weather.windSpeed = (float)(msg[index] & 0x7F);
    if (!((msg[index] >> 7) & 0x01)) weather.windSpeed /= 5;
    index++;
    weather.windGust = (float)(msg[index] & 0x7F);
    if (!((msg[index] >> 7) & 0x01)) weather.windGust /= 5;
    index++;
  }else{
    weather.windDir = 0;
    weather.windSpeed = 0;
    weather.windGust = 0;
  }
  if ((msg[0] >> 4) & 0x01){
    //Humidity (+1byte: in 0.4% (%rh*10/4))
    weather.hum = (float)msg[index]*4/10;
    index++;
  }else{
    weather.hum = 0;
  }
  if ((msg[0] >> 3) & 0x01){
    //Barometric pressure normailized (+2byte: in 10Pa, offset by 430hPa, unsigned little endian (hPa-430)*10)
    uint16_t pPress;
    pPress = (uint16_t)msg[index];
    index++;
    pPress += (uint16_t)msg[index] * 256;
    index++;
    weather.pressure = ((float)pPress / 10.0) + 430.0;
  }else{
    weather.pressure = 0;
  }

  if ((msg[0] >> 1) & 0x01){
    //State of Charge  (+1byte lower 4 bits: 0x00 = 0%, 0x01 = 6.666%, .. 0x0F = 100%)
    weather.Battery = (float)(msg[index] & 0x0F)*100.0/15.0;
    index++;
  }else{
    weather.Battery = 0;
  }

  FanetInsertWeatherData(&weather,pGPS); //insert data into weather-structure
  return TRUE;
}


/**************************************************************************************************************************************************************/

static BOOL FanetParse(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){
  TCHAR ctemp[80];
  NMEAParser::ExtractParameter(String,ctemp,4);
  uint8_t type = (uint8_t)StrToDouble(ctemp,NULL);
	StartupStore(_T("... FANET parse data %s"),String);
  switch (type) {
    case 4: //weatherdata
      return FanetParseType4Msg(d, String, pGPS);

  }
  return TRUE;

}