/*
 *  LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 *  Released under GNU/GPL License v.2 or later
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
#include "devLK8EX1.h"
#include "FlarmCalculations.h"
#include "utils/lookup_table.h"
#include "Geoid.h"
#include "OS/Sleep.h"
#include <sstream>
#include <iomanip>

#ifdef ANDROID
#include "Android/Air3/PowerManagerUtils.h"
#endif

static_assert(IsLittleEndian(), "Big-Endian Arch is not supported");

extern FlarmCalculations flarmCalculations;

namespace {

int from_hex_digit(char hex) {
  char c = toupper(hex);
  return (c >= 'A' ? (c - 'A' + 10) : (c - '0'));
}

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

void fillpaddingZeros(TCHAR *String,TCHAR *String2,int len){
  int size = _tcslen(String2);
  if (size < len){
    for (int i = size;i < len;i++){
      _tcscat(String,_T("0"));
    }
  }
  _tcscat(String,String2);
}

void getIdFromMsg(TCHAR *String,TCHAR *cID,uint32_t *id){
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
int32_t to_int32_t(const uint8_t *buf) {
  int32_t value;
  ((uint8_t*)&value)[0] = buf[0];
  ((uint8_t*)&value)[1] = buf[1];
  ((uint8_t*)&value)[2] = buf[2];
  ((uint8_t*)&value)[3] = ((buf[2]&0x80) ? 0xFF : 0x00); // Sign extension
  return value;
}

/*
 * Convert 2 Byte to uint16_t
 */
uint16_t to_uint16_t(const uint8_t *buf) {
  uint16_t value;
  ((uint8_t*)&value)[0] = buf[0];
  ((uint8_t*)&value)[1] = buf[1];
  return value;
}

/*
 * Convert 6Byte buffer to Latitude/longitude
 */
void payload_absolut2coord(double &lat, double &lon, const uint8_t *buf) {
  if(buf == nullptr)
    return;
  lat = to_int32_t(&buf[0]) / 93206.0;
  lon = to_int32_t(&buf[3]) / 46603.0;
}

template<typename _Tp, size_t size>
int FanetGetIndex(uint32_t ID, _Tp (&array)[size], bool bEmptyIndex){
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
bool FanetInsert(const _Tp& item, _Tp (&array)[size], double Time){
  int index = FanetGetIndex(item.ID, array, true);
  if (index < 0){
    return false;
  }
  array[index] = item; //copy data to structure
  array[index].Time_Fix = Time; //set time for clearing after nothing is received
  return true;
}

enum class aircraft_t : uint8_t {
  other = 0,
  paraglider = 1,
  hangglider = 2,
  balloon = 3,
  glider = 4,
  powered_aircraft = 5,
  helicopter = 6,
  uav = 7,
};

enum class flarm_aircraft_t : uint16_t {
  unknown = 0,
  glider = 1,
  tow_plane = 2,
  helicopter = 3,
  parachute = 4,
  drop_plane = 5,
  fixed_hang_glider = 6,
  soft_para_glider = 7,
  powered_aircraft = 8,
  jet_aircraft = 9,
  UFO = 0x0A,
  balloon = 0x0B,
  blimp = 0x0C,
  zeppelin = 0x0D,
  UAV =  0x0E,
  static_object = 0x0F
};

constexpr auto aircraft_type_table = lookup_table<aircraft_t, flarm_aircraft_t>({
  { aircraft_t::paraglider, flarm_aircraft_t::soft_para_glider },
  { aircraft_t::hangglider, flarm_aircraft_t::fixed_hang_glider },
  { aircraft_t::balloon, flarm_aircraft_t::balloon },
  { aircraft_t::glider, flarm_aircraft_t::glider },
  { aircraft_t::powered_aircraft, flarm_aircraft_t::powered_aircraft },
  { aircraft_t::helicopter, flarm_aircraft_t::helicopter },
  { aircraft_t::uav, flarm_aircraft_t::UAV },
});

std::string to_payload(const std::string& data) {
  std::ostringstream ss;
  ss << std::uppercase << std::setfill('0') << std::hex;
  ss << data.size() << ",";
  ss << std::hex << std::setw(2);
  for (uint8_t v: data) {
    ss << static_cast<unsigned>(v);
  }
  return ss.str();
}

void UpdateName(FLARM_TRAFFIC& traffic) {
  traffic.UpdateNameFlag = false; // clear flag first

  const TCHAR *fname = LookupFLARMDetails(traffic.RadioId);
  if (fname) {
    _tcsncpy(traffic.Name, fname, MAXFLARMNAME);
    const TCHAR *cname = LookupFLARMCn(traffic.RadioId);
    if (cname) {
      int cnamelen = _tcslen(cname);
      if (cnamelen <= MAXFLARMCN) {
        _tcscpy(traffic.Cn, cname);
      } else {
        // else probably it is the Name again, and we create a fake Cn
        traffic.Cn[0] = cname[0];
        traffic.Cn[1] = cname[cnamelen-2];
        traffic.Cn[2] = cname[cnamelen-1];
        traffic.Cn[3] = _T('\0');
      }
    }
    else {
      _tcscpy(traffic.Cn, _T("Err"));
    }
  }
  else {
    // Else we NEED to set a name, otherwise it will constantly search for it over and over..
    _tcscpy(traffic.Name, _T("?"));
    _tcscpy(traffic.Cn, _T("?"));
  }
}

BOOL FanetParseType1Msg(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS) {
  /*
    Tracking (Type = 1)
    [recommended interval: floor((#neighbors/10 + 1) * 5s) ]
    Note: Done by app layer of the fanet module

    [Byte 0-2]	Position	(Little Endian, 2-Complement)
    bit 0-23	Latitude 	(Absolute, see below)
    [Byte 3-5]	Position	(Little Endian, 2-Complement)
    bit 0-23	Longitude 	(Absolute, see below)

    [Byte 6-7]	Type		(Little Endian)
    bit 15 		Online Tracking
    bit 12-14	Aircraft Type
          0: Other
          1: Paraglider
          2: Hangglider
          3: Balloon
          4: Glider
          5: Powered Aircraft
          6: Helicopter
          7: UAV
    bit 11		Altitude Scaling 1->4x, 0->1x
    bit 0-10	Altitude in m

    [Byte 8]	Speed		(max 317.5km/h)
    bit 7		Scaling 	1->5x, 0->1x
    bit 0-6		Value		in 0.5km/h		

    [Byte 9]	Climb		(max +/- 31.5m/s, 2-Complement)
    bit 7		Scaling 	1->5x, 0->1x
    bit 0-6		Value		in 0.1m/s

    [Byte 10]	Heading
    bit 0-7		Value		in 360/256 deg

    [optional]
    [Byte 11]	Turn rate 	(max +/- 64deg/s, positive is clock wise, 2-Complement)
    bit 7		Scaling 	1->4x, 0->1x
    bit 0-6		Value 		in 0.25deg/s	

    [optional, if used byte 11 is mandatory as well]
    [Byte 12]	QNE offset 	(=QNE-GPS altitude, max +/- 254m, 2-Complement)
    bit 7		Scaling 	1->4x, 0->1x
    bit 0-6		Value 		in m	  
   */
  uint32_t RadioId;
  TCHAR HexDevId[7];
  getIdFromMsg(String, HexDevId, &RadioId);

  int flarm_slot = FLARM_FindSlot(pGPS, RadioId);
  if (flarm_slot < 0) {
    // no more slots available,
    DebugLog(_T("... NO SLOTS for Flarm traffic, too many ids!"));
    return FALSE;
  }

  d->nmeaParser.setFlarmAvailable(pGPS);

  FLARM_TRAFFIC &traffic = pGPS->FLARM_Traffic[flarm_slot];

  if (traffic.RadioId != RadioId) {
    traffic = {};
  } else {
    // before changing timefix, see if it was an old target back locked in!
    CheckBackTarget(*pGPS, flarm_slot);
  }

  traffic.RadioId = RadioId;
  traffic.Time_Fix = pGPS->Time;
  traffic.Status = LKT_REAL;

  TCHAR ctemp[80];
  NMEAParser::ExtractParameter(String, ctemp, 5);
  uint8_t payload_length = getByteFromHex(ctemp);
  if ((payload_length < 11) || (payload_length > 13)) {
    return FALSE;
  }

  uint8_t msg[payload_length];
  NMEAParser::ExtractParameter(String, ctemp, 6);
  for (int i = 0; i < payload_length; i++) {
    // undefined result if _tcslen(ctemp) < (payloadLen*2)
    msg[i] = getByteFromHex(&ctemp[i * 2]);
  }

  payload_absolut2coord(traffic.Latitude, traffic.Longitude, &msg[0]);
  uint16_t Type = to_uint16_t(&msg[6]);
  uint16_t altitude = Type & 0x7FF;
  // OnlineTracking = (Type & 0x8000);
  if (Type & 0x0800) {
    altitude *= 4;
  }
  traffic.Altitude = altitude;
  traffic.Average30s = flarmCalculations.Average30s(traffic.RadioId, pGPS->Time, traffic.Altitude);

  NMEAParser::ExtractParameter(String, ctemp, 5);
  uint8_t legacyAircraftType = getByteFromHex(ctemp);

  if (legacyAircraftType != 0) {
    traffic.Type = static_cast<uint16_t>(aircraft_type_table.get(
            (aircraft_t) (0x80 + legacyAircraftType), flarm_aircraft_t::unknown));
  } else {
    traffic.Type = static_cast<uint16_t>(aircraft_type_table.get((aircraft_t) ((Type >> 12) & 0x07),
                                                                 flarm_aircraft_t::unknown));
  }

  uint16_t speed = to_uint16_t(&msg[8]);
  if (speed & 0x80) {
    speed = (speed & 0x007F) * 5;
  }
  traffic.Speed = speed * 0.5 / TOKPH;

  auto climb = static_cast<uint8_t>(msg[9]);
  auto climb2 = static_cast<int8_t>((climb & 0x7F) | (climb & (1 << 6)) << 1); //create 2-complement
  if (climb & 0x80) {
    traffic.ClimbRate = climb2 * 5.0 / 10.0;
  } else {
    traffic.ClimbRate = climb2 / 10.0;
  }

  traffic.Bearing = AngleLimit360(msg[10] * 360. / 255.);

  if (!traffic.Name[0] || traffic.UpdateNameFlag) {
    UpdateName(traffic);
  }
  return TRUE;
}

BOOL FanetParseType2Msg(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS) {
  /*
    Name (Type = 2)
    [recommended interval: every 4min]

    8bit String (of arbitrary length, \0 termination not required)

   */

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

BOOL FanetParseType3Msg(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS) {
  /*
    Message (Type = 3)

    [Byte 0]	Header
    bit 0-7 	Subheader, Subtype (TBD)
          0: Normal Message
          
    8bit String (of arbitrary length)  
   */

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

BOOL FanetParseType4Msg(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS) {
  /*
    Service (Type = 4)
    [recommended interval: 40sec]

    [Byte 0]	Header	(additional payload will be added in order 6 to 1, followed by Extended Header payload 7 to 0 once defined)
        bit 7		Internet Gateway (no additional payload required, other than a position)
        bit 6		Temperature (+1byte in 0.5 degree, 2-Complement)
        bit 5		Wind (+3byte: 1byte Heading in 360/256 degree, 1byte speed and 1byte gusts in 0.2km/h (each: bit 7 scale 5x or 1x, bit 0-6))
        bit 4		Humidity (+1byte: in 0.4% (%rh*10/4))
        bit 3		Barometric pressure normailized (+2byte: in 10Pa, offset by 430hPa, unsigned little endian (hPa-430)*10)
        bit 2		Support for Remote Configuration (Advertisement)
        bit 1		State of Charge  (+1byte lower 4 bits: 0x00 = 0%, 0x01 = 6.666%, .. 0x0F = 100%)
        bit 0		Extended Header (+1byte directly after byte 0)
    The following is only mandatory if no additional data will be added. Broadcasting only the gateway/remote-cfg flag doesn't require pos information. 
    [Byte 1-3 or Byte 2-4]	Position	(Little Endian, 2-Complement)		
        bit 0-23	Latitude	(Absolute, see below)
    [Byte 4-6 or Byte 5-7]	Position	(Little Endian, 2-Complement)
        bit 0-23	Longitude   (Absolute, see below)
    + additional data according to the sub header order (bit 6 down to 1)

   */

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

BOOL IgnoredMsg(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS) {
  DebugLog(_T("Ignored\"%s\""), String);
  return TRUE;
}


using parse_function = BOOL(*)(PDeviceDescriptor_t, TCHAR*, NMEA_INFO*);

template<typename Table>
BOOL FanetParse(Table& table, PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS) {
  TCHAR ctemp[80];
  NMEAParser::ExtractParameter(String,ctemp,4);
  uint8_t type = _tcstol(ctemp, nullptr, 10);
  parse_function parse = table.get(type, IgnoredMsg);
  return parse(d, String, pGPS);
}

} // namespace

bool GetFanetName(uint32_t ID, const NMEA_INFO &info, TCHAR* szName, size_t size) {
  int index = FanetGetIndex(ID, info.FanetName, false);
  if (index >= 0) {
    _tcsncpy(szName, info.FanetName[index].Name, size);
    return true;
  }
  szName[0] = _T('\0'); // empty out string if name not found.
  return false;
}

namespace GXAirCom {
namespace {

constexpr auto function_table = lookup_table<uint8_t, parse_function>({
    // GXAircom send both Fanet and FLarn nmea sentence for Friend Position, ignore the first ...
  { 0x02, &FanetParseType2Msg },
  { 0x03, &FanetParseType3Msg },
  { 0x04, &FanetParseType4Msg }
});

BOOL ParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){
  if(pGPS && _tcsncmp(TEXT("#FNF"), String, 4)==0) {
    return FanetParse(function_table, d, &String[5], pGPS);      
  }
  if(LK8EX1ParseNMEA(d, String, pGPS)) {
      return TRUE;
  }
  return FALSE;
}

} // namespace

void Install(DeviceDescriptor_t* d) {
  _tcscpy(d->Name, DeviceName);
  d->ParseNMEA = ParseNMEA;
  d->IsBaroSource = LK8EX1IsBaroSource;
}

} // GXAirCom

namespace Fanet {
/*
 * Skytraxx Fanet+ Module : https://github.com/3s1d/fanet-stm32/raw/master/fanet_module.pdf
 *   currently only device tested is "Air3 7.3+", module si available on port /dev/ttyMT2:115200:8bit
 */

namespace {

constexpr auto function_table = lookup_table<uint8_t, parse_function>({
  { 0x01, &FanetParseType1Msg },
  { 0x02, &FanetParseType2Msg },
  { 0x03, &FanetParseType3Msg },
  { 0x04, &FanetParseType4Msg }
});

BOOL ParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS) {
  if (!pGPS) {
    return FALSE;
  }

  if(_tcsncmp(TEXT("#FN"), String, 3) == 0) {
    switch (String[3]) {
      case 'F':
        return FanetParse(function_table, d, &String[5], pGPS);
      case 'R':
        d->nmeaParser.setFlarmAvailable(pGPS);
        break;
      case 'A':
        // Address: #FNA manufacturer(hex),id(hex)
        break;
      case 'C':
        // <− #FNC OK\n
        break;
    }
    return TRUE;
  }

  if(_tcsncmp(TEXT("#DGV"), String, 4) == 0) {
    // <− #DGV build −201709261354\n
    d->nmeaParser.setFlarmAvailable(pGPS);
    return TRUE;
  }

  if(_tcsncmp(TEXT("#FAX"), String, 4) == 0) {
    // <− #FAX 118,0,31
    d->nmeaParser.setFlarmAvailable(pGPS);
    return TRUE;
  }

  if(_tcsncmp(TEXT("#DGR"), String, 4) == 0) {
    // <− #DGR OK\n
    d->nmeaParser.setFlarmAvailable(pGPS);
    return TRUE;
  }

  if(_tcsncmp(TEXT("#FAP"), String, 4) == 0) {
    // <− #FAP OK\n
    d->nmeaParser.setFlarmAvailable(pGPS);
    return TRUE;
  }

  return FALSE;
}

BOOL Open(DeviceDescriptor_t *d) {
#ifdef ANDROID
  if (PowerManagerUtils::openModuleFanet()) {
    Sleep(10000); // sleep 10sec after Enable Fanet Module
  }
#endif

  d->Com->WriteString("#DGV\n"); // Get Module Version
  d->Com->WriteString("#FNA\n"); // Get local Address
  d->Com->WriteString("#FAX\n"); // get FLARM Expiration date

  d->Com->WriteString("#DGL 868,12\n"); // Region "868 Mhz, 12dBm"
  d->Com->WriteString("#FNC 1,1,1\n"); // paraglider, onlineLogging, hiking ?
  d->Com->WriteString("#FNM 0\n"); // flying

  d->Com->WriteString("#DGP 1\n"); // Power On Receiver
  d->Com->WriteString("#FAP 1\n"); // Power On FLARM beacon

  return TRUE;
}

BOOL Close(DeviceDescriptor_t *d) {
  d->Com->WriteString("#FAP 0\n"); // Power Off FLARM beacon
  d->Com->WriteString("#DGP 0\n"); // Power Off Module

#ifdef ANDROID
  PowerManagerUtils::closeModuleFanet();
#endif

  return TRUE;
}

BOOL HeartBeat(DeviceDescriptor_t *d) {
  static PeriodClock timeName;

 /*
  * Every 5 second
  *
  * State: 		#FNS lat(deg),lon(deg),alt(m MSL),speed(km/h),climb(m/s),heading(deg)
  * 						[,year(since 1900),month(0-11),day,hour,min,sec,sep(m)[,turnrate(deg/sec)[,QNEoffset(m)]]]
  * 					note: all values in float/int (NOT hex), time is required for FLARM in struct tm format
  * 					note2: FLARM uses the ellipsoid altitudes ->
  * 							sep = Height of geoid (mean sea level) above WGS84 ellipsoid
  * 					note3: QNEoffset is optional: QNEoffset = QNE - GPS altitude
  */

  std::string frm_to_send = WithLock(CritSec_FlightData, []() -> std::string {
    if (GPS_INFO.NAVWarning) {
        return {};
    }

    std::ostringstream ss;
    ss << "#FNS ";
    ss << GPS_INFO.Latitude << ",";
    ss << GPS_INFO.Longitude << ",";
    ss << GPS_INFO.Altitude << ",";
    ss << GPS_INFO.Speed * TOKPH << ",";
    ss << GPS_INFO.TrackBearing << ",";
    ss << GPS_INFO.Vario << ",";
    ss << GPS_INFO.Year - 1900 << ",";
    ss << GPS_INFO.Month << ",";
    ss << GPS_INFO.Day << ",";
    ss << GPS_INFO.Hour << ",";
    ss << GPS_INFO.Minute << ",";
    ss << GPS_INFO.Second << ",";
    ss << LookupGeoidSeparation(GPS_INFO.Latitude, GPS_INFO.Longitude) << ",\n";

    return ss.str();
  });

  if (!frm_to_send.empty()) { // empty if no valid fix

    d->Com->Write(frm_to_send.data(), frm_to_send.size());

    if (timeName.CheckUpdate(60000)) {
      // Every 1 : minute Send Pilot Name if not empty
      if (PilotName_Config[0]) {
        std::string payload = to_payload(to_utf8(PilotName_Config));
        d->Com->WriteString("#FNT 2,00,0000,1,0,");
        d->Com->Write(payload.data(), payload.size());
        d->Com->WriteString("\n");
      }
    }
  }
  return TRUE;
}

} // namespace

void Install(DeviceDescriptor_t* d) {
  _tcscpy(d->Name, DeviceName);
  d->ParseNMEA = ParseNMEA;
  d->HeartBeat = HeartBeat;
  d->Open = Open;
  d->Close = Close;
}

} // Fanet
