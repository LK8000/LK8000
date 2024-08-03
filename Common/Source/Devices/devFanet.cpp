/*
 *  LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 *  Released under GNU/GPL License v.2 or later
 *  See CREDITS.TXT file for authors and copyrights
 *
 *  File:   devFanet.cpp
 *  Author: Gerald Eichler
 *
 *  Created on 13 march 2020, 14:45
 */
#include "externs.h"

#include <sstream>
#include <iomanip>

#include "devFanet.h"
#include "Fanet/Fanet.h"
#include "utils/lookup_table.h"
#include "devLK8EX1.h"
#include "Comm/device.h"
#include "Units.h"
#include "Geoid.h"
#include "OS/Sleep.h"

#ifdef ANDROID
#include "Android/Air3/PowerManagerUtils.h"
#endif

namespace {

int from_hex_digit(char hex) {
  char c = toupper(hex);
  return (c >= 'A' ? (c - 'A' + 10) : (c - '0'));
}

uint8_t getByteFromHex(const char *in) {
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

void fillpaddingZeros(char *String, const char *String2,int len){


  int size = strlen(String2);
  if (size < len){
    for (int i = size;i < len;i++){
      strcat(String, "0");
    }
  }
  strcat(String, String2);
}

uint32_t getIdFromMsg(const char *String) {
  char ctemp[10];
  char cID[7];
  cID[0] = 0; //zero-Termination of String;
  NMEAParser::ExtractParameter(String,ctemp,0);
  fillpaddingZeros(cID,ctemp,2);
  NMEAParser::ExtractParameter(String,ctemp,1);
  fillpaddingZeros(cID,ctemp,4);

  return strtoul(cID, nullptr, 16); //convert HEX-String to long
}

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

std::vector<uint8_t> from_payload(const char* string, uint8_t payload_length) {
  std::vector<uint8_t> msg;
  msg.reserve(payload_length);
  for (int i = 0; i < payload_length; i++) {
    // undefined result if _tcslen(ctemp) < (payloadLen*2)
    msg.push_back(getByteFromHex(&string[i * 2]));
  }
  return msg;
}

template<typename Table>
BOOL FanetParse(Table& table, DeviceDescriptor_t* d, const char* String, NMEA_INFO *pGPS) {
  char ctemp[MAX_NMEA_LEN];
  NMEAParser::ExtractParameter(String,ctemp,4);
  uint8_t type = strtol(ctemp, nullptr, 10);
  uint32_t id = getIdFromMsg(String);

  NMEAParser::ExtractParameter(String,ctemp,5);
  uint8_t payloadLen = getByteFromHex(ctemp);
  NMEAParser::ExtractParameter(String,ctemp,6);
  std::vector<uint8_t> payload = from_payload(ctemp, payloadLen);

  fanet_parse_function parse = table.get(type, FanetParseUnknown);
  return parse(d, pGPS, id, payload);
}

} // namespace

namespace GXAirCom {
namespace {

constexpr auto function_table = lookup_table<uint8_t, fanet_parse_function>({
    // GXAircom send both Fanet and FLarn nmea sentence for Friend Position, ignore the first ...
  { FRM_TYPE_NAME, &FanetParseType2Msg },
  { FRM_TYPE_MESSAGE, &FanetParseType3Msg },
  { FRM_TYPE_SERVICE, &FanetParseType4Msg },
  { FRM_TYPE_GROUNDTRACKING, &FanetParseType7Msg },
  { FRM_TYPE_THERMAL, &FanetParseType9Msg }
});

BOOL ParseNMEA(DeviceDescriptor_t* d, const char* String, NMEA_INFO *pGPS){
  if(pGPS && strncmp("#FNF", String, 4)==0) {
    return FanetParse(function_table, d, &String[5], pGPS);      
  }
  if(LK8EX1ParseNMEA(d, String, pGPS)) {
      return TRUE;
  }
  return FALSE;
}

} // namespace

void Install(DeviceDescriptor_t* d) {
  d->ParseNMEA = ParseNMEA;
}

} // GXAirCom

namespace Fanet {
/*
 * Skytraxx Fanet+ Module : https://github.com/3s1d/fanet-stm32/raw/master/fanet_module.pdf
 *   currently only device tested is "Air3 7.3+", module si available on port /dev/ttyMT2:115200:8bit
 */

namespace {

constexpr auto function_table = lookup_table<uint8_t, fanet_parse_function>({
  { FRM_TYPE_TRACKING, &FanetParseType1Msg },
  { FRM_TYPE_NAME, &FanetParseType2Msg },
  { FRM_TYPE_MESSAGE, &FanetParseType3Msg },
  { FRM_TYPE_SERVICE, &FanetParseType4Msg },
  { FRM_TYPE_GROUNDTRACKING, &FanetParseType7Msg },
  { FRM_TYPE_THERMAL, &FanetParseType9Msg }
});

BOOL ParseNMEA(DeviceDescriptor_t* d, const char* String, NMEA_INFO *pGPS) {
  if (!pGPS) {
    return FALSE;
  }

  if(strncmp("#FN", String, 3) == 0) {
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

  if(strncmp("#DGV", String, 4) == 0) {
    // <− #DGV build −201709261354\n
    d->nmeaParser.setFlarmAvailable(pGPS);
    return TRUE;
  }

  if(strncmp("#FAX", String, 4) == 0) {
    // <− #FAX 118,0,31
    d->nmeaParser.setFlarmAvailable(pGPS);
    return TRUE;
  }

  if(strncmp("#DGR", String, 4) == 0) {
    // <− #DGR OK\n
    d->nmeaParser.setFlarmAvailable(pGPS);
    return TRUE;
  }

  if(strncmp("#FAP", String, 4) == 0) {
    // <− #FAP OK\n
    d->nmeaParser.setFlarmAvailable(pGPS);
    return TRUE;
  }

  return FALSE;
}

BOOL Open(DeviceDescriptor_t* d) {
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

BOOL Close(DeviceDescriptor_t* d) {
  d->Com->WriteString("#FAP 0\n"); // Power Off FLARM beacon
  d->Com->WriteString("#DGP 0\n"); // Power Off Module

#ifdef ANDROID
  PowerManagerUtils::closeModuleFanet();
#endif

  return TRUE;
}

std::string build_state_string(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated) {
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

  if (Basic.NAVWarning) {
    return {};
  }

  std::ostringstream ss;
  ss << "#FNS ";
  ss << Basic.Latitude << ",";
  ss << Basic.Longitude << ",";
  ss << Basic.Altitude << ",";
  ss << Units::To(unKiloMeterPerHour, Basic.Speed) << ",";
  ss << Calculated.Heading << ",";
  ss << Basic.Vario << ",";
  ss << Basic.Year - 1900 << ",";
  ss << Basic.Month << ",";
  ss << Basic.Day << ",";
  ss << Basic.Hour << ",";
  ss << Basic.Minute << ",";
  ss << Basic.Second << ",";
  ss << LookupGeoidSeparation(Basic.Latitude, Basic.Longitude) << ",\n";

  return ss.str();
}

BOOL SendData(DeviceDescriptor_t* d, const NMEA_INFO& Basic, const DERIVED_INFO& Calculated) {
  static PeriodClock timeState;
  if (timeState.CheckUpdate(5000)) {
    // Every 5 second : Send current state
    std::string frm_to_send = build_state_string(Basic, Calculated);
    if (!frm_to_send.empty()) {  // empty if no valid fix
      d->Com->Write(frm_to_send.data(), frm_to_send.size());
    }
  }

  static PeriodClock timeName;
  if (timeName.CheckUpdate(60000)) {
    // Every 1 minute : Send Pilot Name if not empty
    if (PilotName_Config[0]) {
      std::string payload = to_payload(to_utf8(PilotName_Config));
      d->Com->WriteString("#FNT 2,00,0000,1,0,");
      d->Com->Write(payload.data(), payload.size());
      d->Com->WriteString("\n");
    }
  }
  return TRUE;
}

} // namespace

void Install(DeviceDescriptor_t* d) {
  d->ParseNMEA = ParseNMEA;
  d->SendData = SendData;
  d->Open = Open;
  d->Close = Close;
}

} // Fanet
