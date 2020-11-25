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
#include "Message.h"
#include "Sound/Sound.h"
#include "resource.h"
#include "OS/ByteOrder.hpp"
#include "Utils.h"

static_assert(IsLittleEndian(), "Big-Endian Arch is not supported");

static int from_hex_digit(char hex) {
  char c = toupper(hex);
  return (c >= 'A' ? (c - 'A' + 10) : (c - '0'));
}

static
uint8_t getByteFromHex(const TCHAR *in) {
  int tens = 0;
  int digits = 0;
   
  if (!isxdigit(in[0]))   // Valid hex digit character?
    return -1;

  if (!isxdigit(in[1])){
    digits = from_hex_digit(in[0]); //we have only one Hex-Sign --> only digits (no tens)
  }else{
    tens = from_hex_digit(in[0]);
    digits = from_hex_digit(in[1]);
  }
  return tens * 16 + digits;
}

static void fillpaddingZeros(TCHAR *String,TCHAR *String2,int len){
  int size = _tcslen(String2);
  if (size < len){
    for (int i = size;i < len;i++){
      _tcscat(String,_T("0"));
    }
  }
  _tcscat(String,String2);
}

static void getIdFromMsg(TCHAR *String,TCHAR *cID,uint32_t *id){
  TCHAR ctemp[10];
  cID[0] = 0; //zero-Termination of String;
  NMEAParser::ExtractParameter(String,ctemp,0);
  fillpaddingZeros(cID,ctemp,2);
  NMEAParser::ExtractParameter(String,ctemp,1);
  fillpaddingZeros(cID,ctemp,4);
  _stscanf(cID, TEXT("%x"), id); //convert HEX-String to long

}

/*
 * Convert 24bit signed integer to int32_t
 */
static int32_t to_int32_t(const uint8_t *buf) {
  int32_t value;
  ((uint8_t*)&value)[0] = buf[0];
  ((uint8_t*)&value)[1] = buf[1];
  ((uint8_t*)&value)[2] = buf[2];
  ((uint8_t*)&value)[3] = ((buf[2]&0x80) ? 0xFF : 0x00); // Sign extension
  return value;
}

/*
 * Convert 6Byte buffer to Latitude/longitude
 */
static void payload_absolut2coord(double &lat, double &lon, const uint8_t *buf) {
  if(buf == nullptr)
    return;
  lat = to_int32_t(&buf[0]) / 93206.0;
  lon = to_int32_t(&buf[3]) / 46603.0;
}

template<typename _Tp, size_t size>
static int FanetGetIndex(uint32_t ID, _Tp (&array)[size], bool bEmptyIndex){
  int iEmpyIndex = -1;
  for (size_t i = 0;i < size;i++){
    if (array[i].Time_Fix == 0){
      if ((iEmpyIndex < 0) && (bEmptyIndex)){
        iEmpyIndex = i;
      }
    } else if (array[i].ID == ID){
      //we found the right entry (same cn)
      return i;
    }
  }
  return iEmpyIndex;
}

template<typename _Tp, size_t size>
static bool FanetInsert(const _Tp& item, _Tp (&array)[size], double Time){
  int index = FanetGetIndex(item.ID, array, true);
  if (index < 0){
    return false;
  }
  array[index] = item; //copy data to structure
  array[index].Time_Fix = Time; //set time for clearing after nothing is received
  return true;
}

bool GetFanetName(uint32_t ID, const NMEA_INFO &info, TCHAR* szName, size_t size) {
  int index = FanetGetIndex(ID, info.FanetName, false);
  if (index >= 0) {
    _tcsncpy(szName, info.FanetName[index].Name, size);
    return true;
  }
  szName[0] = _T('\0'); // empty out string if name not found.
  return false;
}

static BOOL FanetParseType2Msg(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){
  TCHAR ctemp[80];  
  FANET_NAME fanetDevice;
  TCHAR HexDevId[7];

  getIdFromMsg(String,HexDevId,&fanetDevice.ID);

  NMEAParser::ExtractParameter(String,ctemp,5);
  uint8_t payloadLen = getByteFromHex(ctemp);

  NMEAParser::ExtractParameter(String,ctemp,6);
  int i;
  for (i = 0;i < payloadLen;i++){
    if (i < MAXFANETNAME){
      // undefined result if _tcslen(ctemp) < (payloadLen*2)
      fanetDevice.Name[i] = getByteFromHex(&ctemp[i*2]); 
    }else{
      break;
    }    
  }
  fanetDevice.Name[i] = 0; //0-termination of String
  uint32_t flarmId; 
  if (_stscanf(HexDevId, TEXT("%x"), &flarmId) == 1){
    if (AddFlarmLookupItem(flarmId, fanetDevice.Name, true)) { //check, if device is already in flarm-database
      int flarm_slot = FLARM_FindSlot(pGPS, flarmId); //check if Flarm is already in List
      if (flarm_slot>=0) {
        pGPS->FLARM_Traffic[flarm_slot].UpdateNameFlag = true;
      }
      
    }
  }

  FanetInsert(fanetDevice, pGPS->FanetName, pGPS->Time);
  return TRUE;

}

static BOOL FanetParseType3Msg(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){
  TCHAR ctemp[80];
  uint32_t ID; //ID of station (3 Bytes)
  TCHAR MSG[80];
  TCHAR HexDevId[7];

  getIdFromMsg(String,HexDevId,&ID);

  NMEAParser::ExtractParameter(String,ctemp,5);
  uint8_t payloadLen = getByteFromHex(ctemp);

  NMEAParser::ExtractParameter(String,ctemp,6);
  int i;
  for (i = 1;i < payloadLen;i++){
    if (i < 80){
      // undefined result if _tcslen(ctemp) < (payloadLen*2)
      MSG[i-1] = getByteFromHex(&ctemp[i*2]);
    }else{
      break;
    }    
  }
  MSG[i-1] = 0; //0-termination of String
	TCHAR text[150]; // at least (31 + 2 + 80)
  if(!GetFanetName(ID, *pGPS, text)) {
    _tcscpy(text, HexDevId); // no name, use ID
  }
  _tcscat(text, _T("\r\n"));
  _tcscat(text, MSG);
  PlayResource(TEXT("IDR_WAV_DRIP")); //play sound
  Message::AddMessage(10000, MSG_COMMS, text); // message time 10s
  return TRUE;
}

static BOOL FanetParseType4Msg(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){
  TCHAR ctemp[80];
  TCHAR HexDevId[7];
  FANET_WEATHER weather;

  getIdFromMsg(String,HexDevId,&weather.ID);

  NMEAParser::ExtractParameter(String,ctemp,5);
  uint8_t payloadLen = getByteFromHex(ctemp);
  uint8_t msg[payloadLen]; // unchecked buffer overflow in the following code if payloadLen < 17...
  NMEAParser::ExtractParameter(String,ctemp,6);
  for (int i = 0;i < payloadLen;i++) {
    // undefined result if _tcslen(ctemp) < (payloadLen*2)
    msg[i] = getByteFromHex(&ctemp[i*2]);
  }

  int index = 1;
  if (msg[0] & 0x01){ //check extended header
    ++index;
  }
  payload_absolut2coord(weather.Latitude,weather.Longitude,&msg[index]);
  index+=6;
  if ((msg[0] >> 6) & 0x01){
    //Temperature (+1byte in 0.5 degree, 2-Complement)
    weather.temp = ((int8_t)msg[index++])/2.f;
  }else{
    weather.temp = 0;
  }
  if ((msg[0] >> 5) & 0x01){
    //Wind (+3byte: 1byte Heading in 360/256 degree, 1byte speed and 1byte gusts in 0.2km/h (each: bit 7 scale 5x or 1x, bit 0-6))
    weather.windDir =  msg[index++] * 360.0f / 256.0f;
    weather.windSpeed = (float)(msg[index] & 0x7F);
    if (!((msg[index++] >> 7) & 0x01)){
      weather.windSpeed /= 5;
    }
    weather.windSpeed /= 3.6; //convert to m/s
    weather.windGust = (float)(msg[index] & 0x7F);
    if (!((msg[index++] >> 7) & 0x01)) {
      weather.windGust /= 5;
    }
    weather.windGust /= 3.6; //convert to m/s
  }else{
    weather.windDir = 0;
    weather.windSpeed = 0;
    weather.windGust = 0;
  }
  if ((msg[0] >> 4) & 0x01){
    //Humidity (+1byte: in 0.4% (%rh*10/4))
    weather.hum = msg[index++] * 4.f / 10.f;
  }else{
    weather.hum = 0;
  }
  if ((msg[0] >> 3) & 0x01){
    //Barometric pressure normalized (+2byte: in 10Pa, offset by 430hPa, unsigned little endian (hPa-430)*10)
    uint16_t pPress = msg[index++];
    pPress += msg[index++] * 256U;
    weather.pressure = (pPress / 10.0f) + 430.0f;
  }else{
    weather.pressure = 0;
  }

  if ((msg[0] >> 1) & 0x01){
    //State of Charge  (+1byte lower 4 bits: 0x00 = 0%, 0x01 = 6.666%, .. 0x0F = 100%)
    weather.Battery = (msg[index++] & 0x0F) * 100.0f / 15.0f;
  }else{
    weather.Battery = 0;
  }

  FanetInsert(weather,pGPS->FANET_Weather, pGPS->Time); //insert data into weather-structure
  return TRUE;
}


/**************************************************************************************************************************************************************/

static BOOL FanetParse(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){
  TCHAR ctemp[80];
  NMEAParser::ExtractParameter(String,ctemp,4);
  uint8_t type = (uint8_t)StrToDouble(ctemp,NULL);
  switch (type) {
    case 2: //device-name
      return FanetParseType2Msg(d, String, pGPS);
    case 3: //Msg
      return FanetParseType3Msg(d, String, pGPS);
    case 4: //weatherdata
      return FanetParseType4Msg(d, String, pGPS);
  }
  return TRUE;
}

static BOOL FanetParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){
  if(pGPS && _tcsncmp(TEXT("#FNF"), String, 4)==0) {
    return FanetParse(d, &String[5], pGPS);      
  }
  return FALSE;
}

static BOOL FanetInstall(PDeviceDescriptor_t d){
  _tcscpy(d->Name, TEXT("FANET"));
  d->ParseNMEA = FanetParseNMEA;
  return(TRUE);
}

BOOL FanetRegister(void){
  return(devRegister(TEXT("FANET"), 0, FanetInstall));
}
