/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   DeviceDescriptor.h
 */

#ifndef _Comm_DeviceDescriptor_h_
#define _Comm_DeviceDescriptor_h_

#include "tchar.h"
#include "types.h"
#include "Sizes.h"
#include <optional>
#include <vector>
#include <unordered_map>
#include "utils/uuid.h"
#include "Comm/wait_ack.h"
#include "Util/tstring.hpp"
#include "Time/PeriodClock.hpp"
#include "Parser.h"

#define DEVNAMESIZE 32
#define	NUMDEV		 6U

class ComPort;
struct WAYPOINT;
struct NMEA_INFO;
struct DERIVED_INFO;

struct Declaration_t {
  TCHAR PilotName[64];
  TCHAR AircraftType[32];
  TCHAR AircraftRego[32];
  TCHAR CompetitionClass[32];
  TCHAR CompetitionID[32];
  int num_waypoints;
  const WAYPOINT* waypoint[MAXTASKPOINTS];
};

// Synchronization lock for protecting ComPort access and DeviceList modifications
// Note: This is a reentrant (recursive) mutex, so the same thread can acquire it multiple times
extern Mutex CritSec_Comm;

struct DriverData {
  virtual ~DriverData() {};
};

struct DeviceDescriptor_t {
  DeviceDescriptor_t() = delete;

  explicit DeviceDescriptor_t(int idx) : PortNumber(idx) {}

  DeviceDescriptor_t(const DeviceDescriptor_t&) = delete;
  DeviceDescriptor_t(DeviceDescriptor_t&&) = delete;
  DeviceDescriptor_t& operator=(const DeviceDescriptor_t&) = delete;
  DeviceDescriptor_t& operator=(DeviceDescriptor_t&&) = delete;

  void Reset();

  bool IsReady() const;
  bool IsGPS() const;

  ComPort* Com = nullptr;
  TCHAR Name[DEVNAMESIZE + 1];

  BOOL (*DirectLink)(DeviceDescriptor_t* d, BOOL bLinkEnable);
  BOOL (*ParseNMEA)(DeviceDescriptor_t* d, const char* String,
                    NMEA_INFO* GPS_INFO);
  BOOL (*ParseStream)(DeviceDescriptor_t* d, char* String, int len,
                      NMEA_INFO* GPS_INFO);
  BOOL (*PutMacCready)(DeviceDescriptor_t* d, double McReady);
  BOOL (*PutBugs)(DeviceDescriptor_t* d, double Bugs);
  BOOL (*PutBallast)(DeviceDescriptor_t* d, double Ballast);
  BOOL (*PutVolume)(DeviceDescriptor_t* d, int Volume);
  BOOL (*PutRadioMode)(DeviceDescriptor_t* d, int mode);
  BOOL (*PutSquelch)(DeviceDescriptor_t* d, int Squelch);
  BOOL (*PutFreqActive)(DeviceDescriptor_t* d, unsigned khz,
                        const TCHAR* StationName);
  BOOL (*StationSwap)(DeviceDescriptor_t* d);
  BOOL (*PutFreqStandby)(DeviceDescriptor_t* d, unsigned khz,
                         const TCHAR* StationName);
  BOOL (*Open)(DeviceDescriptor_t* d);
  BOOL (*Close)(DeviceDescriptor_t* d);
  BOOL (*LinkTimeout)(DeviceDescriptor_t* d);
  BOOL (*Declare)(DeviceDescriptor_t* d, const Declaration_t* decl,
                  unsigned errorBuffLen, TCHAR errBuffer[]);

  BOOL (*PutQNH)(DeviceDescriptor_t* d, double NewQNH);
  BOOL (*OnSysTicker)(DeviceDescriptor_t* d);
  BOOL (*Config)(DeviceDescriptor_t* d);
  BOOL (*HeartBeat)(DeviceDescriptor_t* d);
  BOOL (*NMEAOut)(DeviceDescriptor_t* d, const char* String);
  BOOL (*PutTarget)(DeviceDescriptor_t* d, const WAYPOINT& wpt);

  BOOL (*OnHeartRate)(DeviceDescriptor_t& d, NMEA_INFO& info, unsigned bpm);
  BOOL (*OnBarometricPressure)(DeviceDescriptor_t& d, NMEA_INFO& info,
                               double Pa);
  BOOL (*OnOutsideTemperature)(DeviceDescriptor_t& d, NMEA_INFO& info,
                               double temp);
  BOOL (*OnRelativeHumidity)(DeviceDescriptor_t& d, NMEA_INFO& info, double hr);
  BOOL (*OnWindOriginDirection)(DeviceDescriptor_t& d, NMEA_INFO& info,
                                double direction);
  BOOL (*OnWindSpeed)(DeviceDescriptor_t& d, NMEA_INFO& info, double speed);
  BOOL (*OnBatteryLevel)(DeviceDescriptor_t& d, NMEA_INFO& info, double level);

  /**
   *  @timestamp : monotonic clock, nanoseconds.
   *  @gx, @gy, @gz : acceleration in G
   */
  BOOL (*OnAcceleration)(DeviceDescriptor_t& d, NMEA_INFO& info, uint64_t timestamp,
                         double gx, double gy, double gz);

  bool (*DoEnableGattCharacteristic)(DeviceDescriptor_t&, uuid_t, uuid_t);
  void (*OnGattCharacteristic)(DeviceDescriptor_t&, NMEA_INFO&, uuid_t, uuid_t,
                               const std::vector<uint8_t>&);

  /**
   * called at the the end of calculation thread loop for each GPS FIX
   */
  BOOL (*SendData)(DeviceDescriptor_t* d, const NMEA_INFO& Basic,
                   const DERIVED_INFO& Calculated);

  bool IsBaroSource;
  bool IsRadio;

  bool m_bAdvancedMode;
  std::optional<unsigned> SharedPortNum;
  const unsigned PortNumber;
  bool Disabled;

  tstring SerialNumber;

  // Com port diagnostic
  unsigned Rx = 0;
  unsigned ErrRx = 0;
  unsigned Tx = 0;
  unsigned ErrTx = 0;

  // Com ports hearth beats, based on LKHearthBeats
  unsigned HB;
#ifdef DEVICE_SERIAL
  int HardwareId;
  double SoftwareVer;
#endif
  NMEAParser nmeaParser;
  //  DeviceIO PortIO[NUMDEV];

  BOOL _PutMacCready(double McReady);
  BOOL _PutBugs(double Bugs);
  BOOL _PutBallast(double Ballast);
  BOOL _PutVolume(int Volume);
  BOOL _PutRadioMode(int mode);
  BOOL _PutSquelch(int Squelch);
  BOOL _PutFreqActive(unsigned khz, const TCHAR* StationName);
  BOOL _StationSwap();
  BOOL _PutFreqStandby(unsigned khz, const TCHAR* StationName);
  BOOL _PutTarget(const WAYPOINT& wpt);

  BOOL _SendData(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated);

  BOOL _PutQNH(double NewQNH);
  BOOL _LinkTimeout();
  BOOL _HeartBeat();

  BOOL RecvMacCready(double McReady);
  PeriodClock IgnoreMacCready;

  BOOL RecvBugs(double Bugs);
  PeriodClock IgnoreBugs;

  BOOL RecvBallast(double Ballast);
  PeriodClock IgnoreBallast;

  wait_ack_shared_ptr make_wait_ack(const char* success_str,
                                    const char* error_str = nullptr) {
    auto wait_ack = make_wait_ack_shared(CritSec_Comm, success_str, error_str);
    wait_ack_weak = wait_ack;
    return wait_ack;
  }

  wait_ack_shared_ptr lock_wait_ack() {
    return wait_ack_weak.lock();
  }

  template <typename T>
  T* get_data(unsigned tag) {
    auto ib = driver_data.emplace(tag, nullptr);
    if (ib.second) {
      ib.first->second = std::make_unique<T>();
    }
    return dynamic_cast<T*>(ib.first->second.get());
  }

 private:
  wait_ack_weak_ptr wait_ack_weak;

  using DriverDataPtr = std::shared_ptr<DriverData>;
  std::unordered_map<unsigned, DriverDataPtr> driver_data;
};

// generic maker: only needs the size N
template <std::size_t N, std::size_t... Is>
constexpr auto make_device_list(std::index_sequence<Is...>) {
  return std::array<DeviceDescriptor_t, N>{DeviceDescriptor_t(Is)...};
}

template <std::size_t N>
constexpr auto make_device_list() {
  return make_device_list<N>(std::make_index_sequence<N>{});
}

extern std::array<DeviceDescriptor_t, NUMDEV> DeviceList;

#endif  // _Comm_DeviceDescriptor_h_
