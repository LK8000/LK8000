/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"
#include "Sound/Sound.h"
#include "InputEvents.h"
#include "Calc/Vario.h"
#include <optional>
#include "Baro.h"
#include "Comm/ExternalWind.h"
#include "utils/printf.h"

extern bool GotFirstBaroAltitude; // used by UpdateBaroSource

#ifndef NDEBUG
  #define DEBUGNPM	1
#endif

namespace {

/**
 * Searches first device for which predicate "pred" returns true
 */
template<typename Predicate>
std::optional<unsigned> find_device(Predicate&& p) {
  for (auto& dev : DeviceList) {
    if (p(dev)) {
      return dev.PortNumber;
    }
  }
  return {};
}

/**
 * Call function "f" for all device
 */
template<typename Function>
void for_each_device(Function&& f) {
  for (auto& dev : DeviceList) {
    f(dev);
  }
}

/**
 * Predicate : port Enabled
 */
struct port_enabled {
  bool operator()(DeviceDescriptor_t& dev) {
    return !dev.Disabled;
  }
};

/**
 * Predicate : connected gps
 */
struct gps_connected {
  bool operator()(DeviceDescriptor_t& dev) {
    return !dev.Disabled && dev.nmeaParser.connected;
  }
};

/**
 * Predicate : active port
 */
struct port_active {
  bool operator()(DeviceDescriptor_t& dev) {
    return !dev.Disabled && dev.nmeaParser.activeGPS;
  }
};

/**
 * Predicate : connected gps with valid fix
 */
struct gps_fix {
  bool operator()(DeviceDescriptor_t& dev) {
    return !dev.Disabled && dev.nmeaParser.gpsValid;
  }
};

/**
 * Predicate : connected port receiving data
 */
struct port_with_data {
  bool operator()(DeviceDescriptor_t& dev) {
    return !dev.Disabled && ((LKHearthBeats - dev.HB) < 10);
  }
};

/**
 * check expired gps fix on all configured port
 */
void check_gps_valid() {
  for_each_device([](auto & dev) {
    if (!dev.Disabled) {
      dev.nmeaParser.CheckGpsValid();
    }
  });
};

/**
 * activate "idx" port and desactivate all others
 */
void set_active_gps(unsigned idx) {
  for_each_device([=](auto& dev) {
    dev.nmeaParser.activeGPS = (dev.PortNumber == idx);
  });
}

/**
 * check expired RMZ on all configured port 
 */
void check_rmz() {
  for_each_device([](auto& dev) {
    if (!dev.Disabled) {
      dev.nmeaParser.CheckRMZ();
    }
  });
}

/**
 * @return active GPS DeviceDescriptor
 */
DeviceDescriptor_t& get_active_gps() {
  auto active = find_device(port_active());
  return DeviceList[active.value_or(0)];
}

void reset_nmea_info_availability(std::optional<unsigned> idx = {}) {
  ScopeLock lock(CritSec_FlightData);
  GPS_INFO.reset_availability(idx);
  EnableExternalTriggerCruise = false;
}

class SourceMonitor {
 public:
  using GetterT = std::function<unsigned()>;

  SourceMonitor(GetterT&& getter, const TCHAR* name)
      : getter_(std::forward<GetterT>(getter)), name_(name) {}

  void check() {
    unsigned current = WithLock(CritSec_FlightData, getter_);
    if (last_value_ != current) {
      last_value_ = current;
      StartupStore(_T(". %s source changed to device %c @%s"), name_,
                   devLetter(current), WhatTimeIsIt());
    }
  }

 private:
  GetterT getter_;     // callable extracting the device index from GPS_INFO
  const TCHAR* name_;  // for logging
  unsigned last_value_ = NUMDEV;
};

//
// Run every 5 seconds, approx.
// This is the hearth of LK. Questions? Ask Paolo..
// THIS IS RUNNING WITH LockComm  from ConnectionProcessTimer .
//
bool UpdateMonitor() {
  static SourceMonitor monitors[] = {
    { [] { return GPS_INFO.BaroAltitude.index(); },          _T("Baro") },
    { [] { return GPS_INFO.Vario.index(); },                 _T("Vario") },
    { [] { return GPS_INFO.NettoVario.index(); },            _T("NettoVario") },
    { [] { return GPS_INFO.OutsideAirTemperature.index(); }, _T("OutsideAirTemperature") },
    { [] { return GPS_INFO.RelativeHumidity.index(); },      _T("RelativeHumidity") },
    { [] { return GPS_INFO.Gload.index(); },                 _T("GLoad") },
    { [] { return GPS_INFO.HeartRate.index(); },             _T("HeartRate") },
    { [] { return GPS_INFO.MagneticHeading.index(); },       _T("MagneticHeading") },
    { [] { return GPS_INFO.ExternalWind.index(); },          _T("Wind") },
    { [] { return GPS_INFO.Acceleration.index(); },          _T("Acceleration") },
    { [] { return GPS_INFO.IndicatedAirSpeed.index(); },     _T("IndicatedAirSpeed") },
    { [] { return GPS_INFO.TrueAirSpeed.index(); },          _T("TrueAirSpeed") }
  };

  for (auto& sm : monitors) {
      sm.check();
  }

  static bool lastvalidBaro = false;
  static bool wasSilent[std::size(DeviceList)] = {false};

  ScopeLock Lock(CritSec_Comm);

  // save current active port.
  auto last_active = find_device(port_active());

  // first reset gpsValid if device stop to send GPS data...
  check_gps_valid();

  /**
   * find and activate first valid port.
   *   select by priority :
   *      - first GPS with valid fix
   *      - first GPS without valid fix
   *      - first receiving data device
   *      - first enabled device
   *      - first device
   */

  // get first device with valid gps fix
  auto active = find_device(gps_fix());

  if (!active) {
    // if no activity on any port, let first connected (no valid fix) port active.
    active = find_device(gps_connected());
  }

  if (!active) {
    // No valid fix on any port. We use the first port with at least some data going through!
    // This will keep probably at least the time updated since the gps may still be receiving a 
    // valid time, good for us.
    active = find_device(port_with_data());
  }

  if (!active) {
    // if no data on any port, use first enabled port.
    active = find_device(port_enabled());
  }

  if (!active) {
    // if no enabled port, let first port active.
    active = 0U;
  }

  set_active_gps(active.value());

  DeviceDescriptor_t& active_dev = get_active_gps();

  // wait for 10 seconds before monitoring, after startup
  if (LKHearthBeats < 20) {
    return active_dev.nmeaParser.connected;
  }

  if (!active_dev.nmeaParser.connected || !active_dev.nmeaParser.gpsValid) {

    bool reseted = WithLock(CritSec_FlightData, []() {
      if (!GPS_INFO.NAVWarning) {
        GPS_INFO.NAVWarning = true; // reset wrong Valid Fix
        return true;
      }
      return false;
    });

    if (reseted) {
      DebugLog(_T("Active device don't have valid gps fix : Reset 'NAVWarning'"));
    }
  }

  /* check if Flarm disappeared after 30 seconds no activity */
  if (GPS_INFO.FLARM_Available && ((GPS_INFO.Time -LastFlarmCommandTime)> (LKTime_Real*2)) ) {
    static unsigned short MessageCnt =0;
    if(MessageCnt < 10) {
      MessageCnt++;
      StartupStore(_T(". FLARM lost! Disable FLARM functions !%s"),NEWLINE);
      DoStatusMessage(MsgToken<947>()); // _@M947_ "FLARM SIGNAL LOST"
    }
    GPS_INFO.FLARM_Available = false;
    GPS_INFO.FLARM_HW_Version =0.0;
    GPS_INFO.FLARM_SW_Version =0.0;
  }

  short validBaro = 0;
  // Check each Port with no serial activity in last seconds
  for (auto& dev : DeviceList) {
    // ignore disabled port
    if (dev.Disabled) {
      continue;
    }

    LKASSERT(dev.PortNumber < std::size(wasSilent));
    if ((LKHearthBeats - dev.HB) > 10) {
      // if this is active and supposed to have a valid fix.., but no HB..
      if ((active.value() == dev.PortNumber) && dev.nmeaParser.gpsValid) {
        StartupStore(_T("... Port %c no hearthbeats, but still gpsValid: forced invalid  %s"), devLetter(dev.PortNumber), WhatTimeIsIt());
        dev.nmeaParser.Reset();
      }

      // We reset some flags globally only once in case of device gone silent
      if (!wasSilent[dev.PortNumber]) {
        StartupStore(_T("... Port %c gone silent, reset data availability %s"), devLetter(dev.PortNumber), WhatTimeIsIt());
        dev.IsBaroSource = false;
        dev.IsRadio = false;
        dev.nmeaParser.Reset();

        reset_nmea_info_availability(dev.PortNumber);

        wasSilent[dev.PortNumber] = true;
      }
    } else {
      wasSilent[dev.PortNumber] = false;
      // We have hearth beats, is baro available?
      if (devIsBaroSource(dev)) { // 100411
        validBaro++;
      }

    }
  }

  // do we really still have a baro altitude available?
  // If some baro source disappeared, let's reset it for safety. Parser will re-enable them immediately if available.
  // Assuming here that if no Baro is available, no airdata is available also
  //
  if (validBaro==0) {
    if (GPS_INFO.BaroAltitude.available()) {
      StartupStore(_T("... no active baro source, and still BaroAltitudeAvailable, forced off %s"), WhatTimeIsIt());
      if (EnableNavBaroAltitude) {
        // LKTOKEN  _@M122_ = "BARO ALTITUDE NOT AVAILABLE, USING GPS ALTITUDE"
        DoStatusMessage(MsgToken<122>());
        PortMonitorMessages++;
      } else {
        // LKTOKEN  _@M121_ = "BARO ALTITUDE NOT AVAILABLE"
        DoStatusMessage(MsgToken<121>());
      }

      // We also reset data availability, just in case we are through a mux
      ResetBaroAvailable(GPS_INFO);

      // 120824 Check this situation better> Reset is setting activeGPS true for both devices!
      lastvalidBaro=false;
    }
  } else if (!lastvalidBaro) {

    if (GotFirstBaroAltitude) {
      if (EnableNavBaroAltitude) {
        DoStatusMessage(MsgToken<1796>()); // USING BARO ALTITUDE
      } else {
        DoStatusMessage(MsgToken<1795>()); // BARO ALTITUDE IS AVAILABLE
      }
      StartupStore(_T("... baro source back available %s"),WhatTimeIsIt());
      lastvalidBaro=true;
    } else {
      static bool said = false;
      if (!said) {
        StartupStore(_T("... BARO SOURCE PROBLEM, unmanaged port activity. Wrong device? %s"), WhatTimeIsIt());
        said = true;
      }
    }
  }

  // Very important check for multiplexers: if RMZ did not get through in the past seconds, we want to
  // be very sure that there still is one incoming, otherwise we shall be UpdatingBaroSource using the old one,
  // never really updated!! This is because GGA and RMC are using RMZAvailable to UpdateBaroSource, no matter if
  // there was a real RMZ in the NMEA stream lately.
  // Normally RMZAvailable, RMCAvailable, GGA etc.etc. are reset to false when the com port is silent.
  // But RMZ is special, because it can be sent through the multiplexer from a flarm box.
  check_rmz();

  WithLock(CritSec_FlightData, [] {
    // if device stop to send Baro altitude for more than 6s, reset it's availability 
    // to allow to switch to device with lower priority
    CheckBaroAltitudeValidity(GPS_INFO);

    // Check baro altitude problems. This can happen for several reasons: mixed input on baro on same port,
    // faulty device, etc. The important thing is that we shall not be using baro altitude for navigation in such cases.
    // So we do this check only for the case we are actually using baro altitude.
    // A typical case is: mixed devices on same port, baro altitude disappearing because of mechanical switch,
    // but other traffic still incoming, so hearthbeats are ok. 
    static double	lastBaroAltitude=-1, lastGPSAltitude=-1;
    static unsigned int	counterSameBaro=0, counterSameHGPS=0;
    static unsigned short firstrecovery=0;

    if (EnableNavBaroAltitude && !GPS_INFO.NAVWarning && GPS_INFO.BaroAltitude.available()) {
      if (GPS_INFO.BaroAltitude==lastBaroAltitude) {
        counterSameBaro++;
      } else {
        lastBaroAltitude=GPS_INFO.BaroAltitude;
        counterSameBaro=0;
      }
      if (GPS_INFO.Altitude==lastGPSAltitude) {
        counterSameHGPS++;
      } else {
        lastGPSAltitude=GPS_INFO.Altitude;
        counterSameHGPS=0;
      }

      // This is suspicious enough, because the baro altitude is a floating value, should not be the same..
      // but ok, lets assume it is filtered.
      // if HBAR is steady for some time ... and HGPS is not steady 
      unsigned short timethreshold = 15; // first three times,  timeout at about 1 minute
      if (firstrecovery >=3 ) {
        timethreshold = 40; // then about every 3 minutes
      }

      if ( ((counterSameBaro > timethreshold) && (counterSameHGPS<2)) && (fabs(GPS_INFO.Altitude-GPS_INFO.BaroAltitude)>100.0) && !CALCULATED_INFO.OnGround ) {
        DoStatusMessage(MsgToken<122>()); // Baro not available, Using GPS ALTITUDE
        EnableNavBaroAltitude=false;
        StartupStore(_T("... WARNING, NavBaroAltitude DISABLED due to possible fault: baro steady at %f, HGPS=%f @%s%s"), GPS_INFO.BaroAltitude.value(), GPS_INFO.Altitude,WhatTimeIsIt(),NEWLINE);
        lastBaroAltitude=-1;
        lastGPSAltitude=-1;
        counterSameBaro=0;
        counterSameHGPS=0;
        // We do only ONE attempt to recover a faulty device, to avoid flipflopping.
        // In case of big problems, we shall have disabled the use of the faulty baro altitude, and keep it
        // incoming sporadically.
        if (firstrecovery < 3) {
          // We alse reset these values, just in case we are through a mux
          reset_nmea_info_availability();
          for (auto& dev : DeviceList) {
            dev.nmeaParser.Reset();
          }
          // 120824 Check this situation better> Reset is setting activeGPS true for both devices!
          lastvalidBaro=false;
          GotFirstBaroAltitude=false;
          firstrecovery++;
        }
      }
    }
  });

  //
  // Following is for diagnostics only
  //

  // Nothing has changed? No need to give new alerts. We might have no active gps at all, also.
  // In this case, active and lastactive are 0, nothing we can do about it.
  if (active != last_active) {

    // we need to reset all data availabilty flags, otherwise some of them 
    // can leave set to "true'" even if new active device don't provide data
    reset_nmea_info_availability();

    if (active)
      StartupStore(_T(". GPS NMEA source changed to device %c  %s"), devLetter(active.value()), WhatTimeIsIt());
    else
      StartupStore(_T("... GPS NMEA source PROBLEM, no active GPS!  %s"), WhatTimeIsIt());


    if (PortMonitorMessages<15) { // do not overload pilot with messages!
      // do not say anything if we never got the first port, on startup essentially
      if (last_active && active_dev.nmeaParser.gpsValid) {
        TCHAR vbuf[100];

        lk::snprintf(vbuf,_T("%s %d\n< %s >"), MsgToken<277>(), active.value_or(0), active_dev.Name); // FALLBACK USING GPS ON PORT ..
        DoStatusMessage(vbuf);
        PortMonitorMessages++;
      } 
    } else {
      if (PortMonitorMessages==15) { 
        StartupStore(_T("... GOING SILENT on too many Com reportings.  %s" NEWLINE),WhatTimeIsIt());
        DoStatusMessage(MsgToken<317>()); // GOING SILENT ON COM REPORTING
        PortMonitorMessages++;	// we go to 16, and never be back here
      }
    }
  }
  return active_dev.nmeaParser.connected;      
}

} // namespace

// Running at 0.2hz every 5 seconds
int ConnectionProcessTimer(int itimeout) {

  // TODO: PRINT THIS INFORMATION IN THE IGC LOG FILE, ABSOLUTELY!
  static double oldoffset=0;
  if (GPSAltitudeOffset!=oldoffset) {
    StartupStore(_T(". GPS ALTITUDE OFFSET CHANGED FROM: %f TO: %f%s"),oldoffset,GPSAltitudeOffset,NEWLINE);
    oldoffset=GPSAltitudeOffset;
  }

  devHeartBeat();  // Send Heartbeats every 5s to driver e.g. KeepAlive functions if needed

  bool gpsconnect = UpdateMonitor();


//      1) we have valid fix on active device ?
//      2) what port is alive ?
//      3) restart only dead port


  // dont warn on startup
  static bool s_firstcom=true;

  static bool s_lastGpsConnect = false;
  static bool s_connectWait = false;
  static bool s_lockWait = false;

  if (gpsconnect) {
    extGPSCONNECT = TRUE;
  }

  bool navwarning = WithLock(CritSec_FlightData, [&]() {
    if (!extGPSCONNECT) {
      if (!GPS_INFO.NAVWarning) {
        // If gps is not connected, set NAVWarning to true so
        // calculations flight timers don't get updated
        GPS_INFO.NAVWarning = true;
      }
    }
    return GPS_INFO.NAVWarning;
  });

  bool DoTriggerUpdate = false;

  if((gpsconnect == false) && (s_lastGpsConnect == false)) {
    // re-draw screen every five seconds even if no GPS
    DoTriggerUpdate = true;

    devLinkTimeout(); // no device connected, call LinkTimeout on all device.

    if(s_lockWait == true) {
      // gps was waiting for fix, now waiting for connection
      s_lockWait = false;
    }
    if(!s_connectWait) {
      // gps is waiting for connection first time
      extGPSCONNECT = FALSE;

      s_connectWait = true;

      if (!s_firstcom) {
        LKSound(TEXT("LK_GPSNOCOM.WAV"));
      }
      FullScreen();
    } else {
      // restart comm ports on timeouts, but not during managed special communications with devices
      // that will not provide NMEA stream, for example during a binary conversation for task declaration
      // or during a restart. Very careful, it shall be set to zero by the same function who
      // set it to true.
      // V6: do not reset comports while inside configuration
      if ((itimeout % 90 == 0) && !LKDoNotResetComms && !MenuActive) {
        // no activity for 90/2 seconds (running at 2Hz), then reset.
        // This is needed only for virtual com ports..
        if (!devIsDisabled()) {
          extGPSCONNECT = FALSE;
          StartupStore(_T(". ComPort RESET ordered" NEWLINE));
          if (MapSpaceMode != MSM_WELCOME) {
            InputEvents::processGlideComputer(GCE_COMMPORT_RESTART);
          }
          RestartCommPorts();
        }

        itimeout = 0;
      }
    }
  }

  // Force RESET of comm ports on demand
  if (LKForceComPortReset) {
    StartupStore(_T(". ComPort RESET ordered" NEWLINE));
    LKForceComPortReset=false;
    LKDoNotResetComms=false;
    if (MapSpaceMode != MSM_WELCOME) {
      InputEvents::processGlideComputer(GCE_COMMPORT_RESTART);
    }

    RestartCommPorts();
  }

  if((gpsconnect == true) && (s_lastGpsConnect == false)) {
    itimeout = 0; // reset timeout
    s_firstcom=false;

    if(s_connectWait) {
      DoTriggerUpdate = true;
      s_connectWait = false;
    }
  }

  if((gpsconnect == true) && (s_lastGpsConnect == true)) {
    if((navwarning == true) && (s_lockWait == false)) {
      DoTriggerUpdate = true;

      s_lockWait = true;
      LKSound(TEXT("LK_GPSNOFIX.WAV"));
      FullScreen();
    } else {
      if((navwarning == false) && (s_lockWait == true)) {
        DoTriggerUpdate = true;
        s_lockWait = false;
      }
    }
  }

  if(DoTriggerUpdate) {
    TriggerGPSUpdate();
  }

  s_lastGpsConnect = gpsconnect;
  return itimeout;
}
