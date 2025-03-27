/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   devAirControlDisplay.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 27 october 2023
 */

#include "externs.h"
#include "devAirControlDisplay.h"
#include "devNmeaOut.h"
#include <regex>
#include <string>
#include "utils/printf.h"
#include "Radio.h"
#include "Util/Clamp.hpp"
#include "Comm/UpdateQNH.h"
#include "Baro.h"

using std::string_view_literals::operator""sv;

namespace {

struct Configuration {
  static size_t build(char (&sNmea)[MAX_NMEA_LEN], char Type, const char* Code, const char* Item) {
    return lk::snprintf(sNmea, "$PAAVC,%c,%s,%s", Type, Code, Item);
  }

  static size_t build(char (&sNmea)[MAX_NMEA_LEN], char Type, const char* Code, const char* Item, unsigned Value) {
    size_t len = build(sNmea, Type, Code, Item);
    return len + lk::snprintf(sNmea + len, MAX_NMEA_LEN - len, ",%u", Value);
  }

  static size_t build(char (&sNmea)[MAX_NMEA_LEN], char Type, const char* Code, const char* Item, char Value) {
    size_t len = build(sNmea, Type, Code, Item);
    return len + lk::snprintf(sNmea + len, MAX_NMEA_LEN - len, ",%c", Value);
  }
};

struct Command {
  static size_t build(char (&sNmea)[MAX_NMEA_LEN], const char* Code, const char* Item) {
    return lk::snprintf(sNmea, "$PAAVX,%s,%s", Code, Item);
  }
};

template <typename Builder, typename... Args>
size_t Build(char (&sNmea)[MAX_NMEA_LEN], Args&&... args) {
  size_t len = Builder::build(sNmea, std::forward<Args>(args)...);
  return len + lk::snprintf(sNmea + len, MAX_NMEA_LEN - len, "*%02X\r\n", nmea_crc(sNmea + 1));
}

template <typename Builder, typename... Args>
BOOL Send(ComPort& port, Args&&... args) {
  char sNmea[MAX_NMEA_LEN];
  size_t len = Build<Builder>(sNmea, std::forward<Args>(args)...);
  return port.Write(sNmea, len);
}

template <typename Builder, typename... Args>
BOOL Send(DeviceDescriptor_t* d, Args&&... args) {
  return d && d->Com && Send<Builder, Args...>(*d->Com, std::forward<Args>(args)...);
}

template <typename... Args>
BOOL SendConfiguration(Args&&... args) {
  return Send<Configuration>(std::forward<Args>(args)...);
}

template <typename... Args>
BOOL SendCommand(Args&&... args) {
  return Send<Command>(std::forward<Args>(args)...);
}

bool start_with(const std::string_view& nmea, const std::string_view& prefix) {
  return nmea.substr(0, prefix.size()) == prefix;
}

template <typename Type>
bool compare_set(Type& dst, const Type& src) {
  if (dst != src) {
    dst = src;
    return true;
  }
  return false;
}

BOOL NMEAOut(DeviceDescriptor_t* d, const char* String) {
  static const std::regex re_filter(R"(^\$G.(GGA|RMC|GSA),.*[\n\r]*$)");
  if (std::regex_match(String, re_filter)) {
    return NmeaOut::NMEAOut(d, String);
  }
  return FALSE;
}

BOOL ParseConfiguration(DeviceDescriptor_t* d, const char* String, NMEA_INFO* GPS_INFO) {
  constexpr auto swver_prefix = "ACD,SWVER,"sv;
  if (start_with(String, swver_prefix)) {
    DoStatusMessage(_T("Air Control Display Connected"));
    return TRUE;
  }

  constexpr auto sql_prefix = "COM,SQL,"sv;
  if (start_with(String, sql_prefix)) {
    RadioPara.Changed |= compare_set<int>(RadioPara.Squelch, StrToDouble(String + sql_prefix.size(), nullptr) / 10);
    RadioPara.Changed |= compare_set<BOOL>(RadioPara.SqValid, true);
    return TRUE;
  }

  constexpr auto vol_prefix = "COM,RXVOL1,"sv;
  if (start_with(String, vol_prefix)) {
    RadioPara.Changed |= compare_set<int>(RadioPara.Volume, StrToDouble(String + vol_prefix.size(), nullptr) / 5);
    RadioPara.Changed |= compare_set<BOOL>(RadioPara.VolValid, true);
    return TRUE;
  }

  constexpr auto qnh_prefix = "ALT,QNH,"sv;
  if (start_with(String, qnh_prefix)) {
    UpdateQNH(StrToDouble(String + vol_prefix.size(), nullptr) / 100.);
    return TRUE;
  }

  // TODO :
  DebugLog(_T("ACD unknown: $PAAVC,A,%s"), to_tstring(String).c_str());

  return TRUE;
}

BOOL ParseCommand(DeviceDescriptor_t* d, const char* String, NMEA_INFO* GPS_INFO) {
  DebugLog(_T("ACD unknown : $PAAVX,%s"), to_tstring(String).c_str());
  return TRUE;
}

BOOL ParseCOM(DeviceDescriptor_t* d, const char* String, NMEA_INFO* GPS_INFO) {
  /* <CHN1>,<CHN2>,<RXVOL1>,<RXVOL2>,<DWATCH>,<RX1>,<RX2>,<TX1>

      Field    Description                      Values
      CHN1     Primary radio channel.           25kHz frequencies and 8.33kHz chan-
                                                nels as unsigned integer values bet-
                                                ween 118000 and 136990.

      CHN2     Secondary radio channel.         25kHz frequencies and 8.33kHz chan-
                                                nels as unsigned integer values bet-
                                                ween 118000 and 136990.

      RXVOL1   Primary radio channel volume.    Unsigned integer values, 0–100

      RXVOL2   Secondary radio channel volume.  Unsigned integer values, 0–100

      DWATCH   Dual watch mode.                 0: dual watch off
                                                1: dual watch on

      RX1      Primary channel rx state.        0: no signal received
                                                1: signal received

      RX2      Secondary channel rx state.      0: no signal received
                                                1: signal received

      TX1      Transmit active                  0: no transmission
                                                1: transmitting signal
   */

  char szTmp[MAX_NMEA_LEN];

  d->IsRadio = true;
  RadioPara.Enabled8_33 = true;
  RadioPara.Enabled = true;

  NMEAParser::ExtractParameter(String, szTmp, 0);  // CHN1
  if (compare_set(RadioPara.ActiveKhz, ExtractFrequency(szTmp))) {
    UpdateStationName(RadioPara.ActiveName, RadioPara.ActiveKhz);
    RadioPara.Changed = true;
  }
  RadioPara.Changed |= compare_set<BOOL>(RadioPara.ActiveValid, true);

  NMEAParser::ExtractParameter(String, szTmp, 1);  // CHN2
  if (compare_set(RadioPara.PassiveKhz, ExtractFrequency(szTmp))) {
    UpdateStationName(RadioPara.PassiveName, RadioPara.PassiveKhz);
    RadioPara.Changed = true;
  }
  RadioPara.Changed |= compare_set<BOOL>(RadioPara.PassiveValid, true);

  NMEAParser::ExtractParameter(String, szTmp, 2);  // RXVOL1
  RadioPara.Changed |= compare_set<int>(RadioPara.Volume, StrToDouble(szTmp, nullptr) / 5.);
  RadioPara.Changed |= compare_set<BOOL>(RadioPara.VolValid, true);

  NMEAParser::ExtractParameter(String, szTmp, 4);  // DWATCH
  RadioPara.Changed |= compare_set<BOOL>(RadioPara.Dual, ("1"sv == szTmp));
  RadioPara.Changed |= compare_set<BOOL>(RadioPara.DualValid, true);

  NMEAParser::ExtractParameter(String, szTmp, 5);  // RX1
  RadioPara.Changed |= compare_set<BOOL>(RadioPara.RX_active, ("1"sv == szTmp));

  NMEAParser::ExtractParameter(String, szTmp, 6);  // RX2
  RadioPara.Changed |= compare_set<BOOL>(RadioPara.RX_standy, ("1"sv == szTmp));

  NMEAParser::ExtractParameter(String, szTmp, 7);  // TX1
  RadioPara.Changed |= compare_set<BOOL>(RadioPara.TX, ("1"sv == szTmp));

  return TRUE;
}

BOOL ParseALT(DeviceDescriptor_t* d, const char* String, NMEA_INFO* GPS_INFO) {
  /* $PAAVS,ALT,<ALTQNE>,<ALTQNH>,<QNH>
    Field    Description                       Values
    
    ALTQNE   Current QNE altitude in meters.   Decimal number with two decimal places.
    
    ALTQNH   Current QNH altitude in meters.   Decimal number with two decimalplaces.
    
    QNH      Current QNH setting in pascal.    Unsigned integer values (e.g.101325).
  */

  char szTmp[MAX_NMEA_LEN];

  NMEAParser::ExtractParameter(String, szTmp, 2);  // QNH
  if (szTmp[0]) { // string not empty
    UpdateQNH(StrToDouble(szTmp, nullptr) / 100.);
  }

  NMEAParser::ExtractParameter(String, szTmp, 0);  // ALTQNE
  if (szTmp[0]) { // string not empty
    UpdateBaroSource( GPS_INFO, d, StrToDouble(szTmp, nullptr));
  }

  return TRUE;
}

BOOL ParseXPDR(DeviceDescriptor_t* d, const char* String, NMEA_INFO* GPS_INFO) {
  DebugLog(_T("ACD unknown : $PAAVS,XPDR,%s"), to_tstring(String).c_str());

  // TODO :
  return TRUE;
}

BOOL ParseNMEA(DeviceDescriptor_t* d, const char* String, NMEA_INFO* GPS_INFO) {
  auto wait_ack = d->lock_wait_ack();
  if (wait_ack && wait_ack->check(String)) {
    return TRUE;
  }

  auto configuration_prefix = "$PAAVC,A,"sv;
  if (start_with(String, configuration_prefix)) {
    return ParseConfiguration(d, String + configuration_prefix.size(), GPS_INFO);
  }

  auto command_prefix = "$PAAVX,"sv;
  if (start_with(String, command_prefix)) {
    return ParseCommand(d, String + command_prefix.size(), GPS_INFO);
  }

  auto com_prefix = "$PAAVS,COM,"sv;
  if (start_with(String, com_prefix)) {
    // COM radio
    return ParseCOM(d, String + com_prefix.size(), GPS_INFO);
  }

  auto alt_prefix = "$PAAVS,ALT,"sv;
  if (start_with(String, alt_prefix)) {
    // Altimeter
    return ParseALT(d, String + alt_prefix.size(), GPS_INFO);
  }

  auto xpdr_prefix = "$PAAVS,XPDR,"sv;
  if (start_with(String, xpdr_prefix)) {
    // Transponder
    return ParseXPDR(d, String + xpdr_prefix.size(), GPS_INFO);
  }

  return FALSE;
}

BOOL PutQNH(DeviceDescriptor_t* d, double NewQNH) {
  return SendConfiguration(d, 'S', "ALT", "QNH", static_cast<unsigned>(NewQNH * 100.));
}

BOOL PutFreqActive(DeviceDescriptor_t* d, unsigned khz, const TCHAR* StationName) {
  /* Active frequency is readonly, so workaround is :
   *   - set passive with new frequency
   *   - swap active/passive
   *   - restore old passive
   */
  unsigned old_khz = RadioPara.PassiveKhz;

  char nmea_ack[MAX_NMEA_LEN];
  Build<Configuration>(nmea_ack, 'A', "COM", "CHN2", khz);
  wait_ack_shared_ptr wait_ack = d->make_wait_ack(nmea_ack);
  if (SendConfiguration(d, 'S', "COM", "CHN2", khz) && wait_ack->wait(250)) {
    wait_ack = d->make_wait_ack("$PAAVX,COM,XCHN,OK*2A");
    SendCommand(d, "COM", "XCHN");
    wait_ack->wait(250);
    wait_ack.reset();
    return SendConfiguration(d, 'S', "COM", "CHN2", old_khz);
  }
 return FALSE;
}

BOOL PutFreqStandby(DeviceDescriptor_t* d, unsigned khz, const TCHAR* StationName) {
  return SendConfiguration(d, 'S', "COM", "CHN2", khz);
}

BOOL PutVolume(DeviceDescriptor_t* d, int Volume) {
  return SendConfiguration(d, 'S', "COM", "RXVOL1", Clamp<unsigned>(Volume * 5, 0, 100));
}

BOOL PutRadioMode(DeviceDescriptor_t* d, int mode) {
  return SendConfiguration(d, 'S', "COM", "DWATCH", mode ? '1' : '0');
}

BOOL PutSquelch(DeviceDescriptor_t* d, int Squelch) {
  return SendConfiguration(d, 'S', "COM", "SQL", Clamp<unsigned>(Squelch * 10, 0, 100));
}

BOOL StationSwap(DeviceDescriptor_t* d) {
  return SendCommand(d, "COM", "XCHN");
}

BOOL Open(DeviceDescriptor_t* d) {
  SendConfiguration(d, 'R', "ACD", "SWVER");
  SendConfiguration(d, 'R', "COM", "SQL");
  SendConfiguration(d, 'R', "COM", "RXVOL1");

  PutQNH(d, QNH);

  return TRUE;
}

}  // namespace

void AirControlDisplay::Install(DeviceDescriptor_t* d) {
  d->NMEAOut = NMEAOut;
  d->ParseNMEA = ParseNMEA;

  d->PutQNH = PutQNH;

  d->PutFreqActive = PutFreqActive;
  d->PutFreqStandby = PutFreqStandby;

  d->PutRadioMode = PutRadioMode;
  d->PutSquelch = PutSquelch;
  d->PutVolume = PutVolume;

  d->StationSwap = StationSwap;

  d->Open = Open;
}
