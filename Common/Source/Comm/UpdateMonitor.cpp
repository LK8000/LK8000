/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"
#include "Sound/Sound.h"
#include "InputEvents.h"

extern bool GotFirstBaroAltitude; // used by UpdateBaroSource
extern unsigned LastRMZHB;	 // common to both devA and devB, updated in Parser

double trackbearingminspeed=0; // minimal speed to use gps bearing

#ifndef NDEBUG
//#define DEBUGBARO	1	// also needed in UpdateBaroSource
//#define DEBUGNPM	1
#endif
//
// Run every 5 seconds, approx.
// This is the hearth of LK. Questions? Ask Paolo..
// THIS IS RUNNING WITH LockComm  from ConnectionProcessTimer .
//

static
bool  UpdateMonitor(void)
{

  static int lastactive=0;
  static bool  lastvalidBaro=false;
  static bool wasSilent[array_size(DeviceList)] = { false };

  int active = -1;
  // find first valid GPS
  for(const auto& dev : DeviceList) {
    if(dev.nmeaParser.gpsValid) {
      active = dev.PortNumber;
      break; // we got first, no need to continue;
    }
  }
  
  if(active == -1) {
    // No valid fix on any port. We use the first port with at least some data going through!
    // This will keep probably at least the time updated since the gps may still be receiving a 
    // valid time, good for us.
    for(auto& dev : DeviceList) {
      if(devIsGPSSource(&dev) && ((LKHearthBeats-dev.HB)<10)) {
        active = dev.PortNumber;
        break; // we got first, no need to contine;
      }
    }    
  }
  if(active == -1) {
    // if no activity on any port, let first connected port active.
    for(const auto& dev : DeviceList) {
      if(dev.nmeaParser.connected) {
        active = dev.PortNumber;
        break; // we got first, no need to contine;
      }
    }
  }
  if(active == -1) {
    // if no connected port, let first port active.
    active = 0;
  }

  // activate "active" port and desactivate all others
  for(auto& dev : DeviceList) {
      dev.nmeaParser.activeGPS = (dev.PortNumber == active);
  }

  PDeviceDescriptor_t active_dev = devX(active);

  // wait for 10 seconds before monitoring, after startup
  if (LKHearthBeats<20) {
    return active_dev->nmeaParser.connected;
  }

  /* check if Flarm disappeared after 30 seconds no activity */
  if (GPS_INFO.FLARM_Available && ((GPS_INFO.Time -LastFlarmCommandTime)> (LKTime_Real*2)) ) {
    static unsigned short MessageCnt =0;
    if(MessageCnt < 10) {
      MessageCnt++;
      StartupStore(_T(". FLARM lost! Disable FLARM functions !%s"),NEWLINE);
      DoStatusMessage(MsgToken(947)); // _@M947_ "FLARM SIGNAL LOST"
    }
    GPS_INFO.FLARM_Available = false;
    GPS_INFO.FLARM_HW_Version =0.0;
    GPS_INFO.FLARM_SW_Version =0.0;
  }

  short invalidGps = 0;
  short invalidBaro = 0;
  short validBaro = 0;
  // Check each Port with no serial activity in last seconds
  for(auto& dev : DeviceList) {
    // ignore disabled port
    if(dev.Disabled) {
      continue;
    }

    LKASSERT((unsigned)dev.PortNumber < array_size(wasSilent));
    if ((LKHearthBeats-dev.HB)>10 ) {
#ifdef DEBUGNPM
      StartupStore(_T("... GPS Port %d : no activity LKHB=%u CBHB=%u" NEWLINE), dev.PortNumber, LKHearthBeats, dev.HB);
#endif
      // if this is active and supposed to have a valid fix.., but no HB..
      if ( (active==dev.PortNumber) && (dev.nmeaParser.gpsValid) ) {
        StartupStore(_T("... GPS Port %d no hearthbeats, but still gpsValid: forced invalid  %s" NEWLINE), dev.PortNumber, WhatTimeIsIt());
      }
      dev.nmeaParser.gpsValid=false;
      invalidGps++;
      // We want to be sure that if this device is silent, and it was providing Baro altitude,
      // now it is set to off.
      if (GPS_INFO.BaroAltitudeAvailable==TRUE) {
        if ( &dev == pDevPrimaryBaroSource || dev.nmeaParser.IsValidBaroSource() ) {
          invalidBaro++;
        }
      }

      bool dev_active = dev.nmeaParser.activeGPS;
      bool dev_expire = dev.nmeaParser.expire;
      dev.nmeaParser._Reset();
      dev.nmeaParser.expire = dev_expire; // restore expire status (Reset is setting it to true)

      if(!dev.nmeaParser.expire) {
        // if device never expire, is still connected & don't change activeGPS status.
        dev.nmeaParser.connected = true;
        dev.nmeaParser.activeGPS = dev_active;
      } else {
        dev.nmeaParser.activeGPS=false; // because Reset is setting it to true
      }

      // We reset some flags globally only once in case of device gone silent 
      if (!dev.Disabled && !wasSilent[dev.PortNumber]) {
        GPS_INFO.AirspeedAvailable=false;
        GPS_INFO.VarioAvailable=false;
        GPS_INFO.NettoVarioAvailable=false;
        GPS_INFO.AccelerationAvailable = false;
        EnableExternalTriggerCruise = false;
        wasSilent[dev.PortNumber]=true;
      }
    } else {
      wasSilent[dev.PortNumber]=false;
      // We have hearth beats, is baro available?
      if ( devIsBaroSource(&dev) || dev.nmeaParser.IsValidBaroSource() ) // 100411
        validBaro++;
    }
  }

#ifdef DEBUGNPM
  if (invalidGps==array_size(DeviceList)) {
    StartupStore(_T("... GPS no gpsValid available on any port, active=%d @%s" NEWLINE),active,WhatTimeIsIt());
  }
  if (invalidBaro>0) {
    StartupStore(_T("... Baro altitude just lost, current status=%d @%s" NEWLINE),GPS_INFO.BaroAltitudeAvailable,WhatTimeIsIt());
  }
#endif

  // do we really still have a baro altitude available?
  // If some baro source disappeared, let's reset it for safety. Parser will re-enable them immediately if available.
  // Assuming here that if no Baro is available, no airdata is available also
  //
  if (validBaro==0) {
    if ( GPS_INFO.BaroAltitudeAvailable ) {
      StartupStore(_T("... GPS no active baro source, and still BaroAltitudeAvailable, forced off  %s%s"),WhatTimeIsIt(),NEWLINE);
      if (EnableNavBaroAltitude && active) {
        // LKTOKEN  _@M122_ = "BARO ALTITUDE NOT AVAILABLE, USING GPS ALTITUDE" 
        DoStatusMessage(MsgToken(122));
        PortMonitorMessages++;
      } else {
        // LKTOKEN  _@M121_ = "BARO ALTITUDE NOT AVAILABLE" 
        DoStatusMessage(MsgToken(121));
      }
      GPS_INFO.BaroAltitudeAvailable=FALSE;
      // We alse reset these values, just in case we are through a mux
      GPS_INFO.AirspeedAvailable=false;
      GPS_INFO.VarioAvailable=false;
      GPS_INFO.NettoVarioAvailable=false;
      GPS_INFO.AccelerationAvailable = false;
      EnableExternalTriggerCruise = false;
      for(auto& dev : DeviceList) {
        dev.nmeaParser._Reset();
      }
      // 120824 Check this situation better> Reset is setting activeGPS true for both devices!
      lastvalidBaro=false;
    }
  } else {
    if ( lastvalidBaro==false) {
#if DEBUGBARO
      const TCHAR* devname = (pDevPrimaryBaroSource) ? pDevPrimaryBaroSource->Name : _T("unknown");
      StartupStore(_T("... GPS baro source back available from <%s>" NEWLINE),devname);
#endif

      if (GotFirstBaroAltitude) {
        if (EnableNavBaroAltitude) {
          DoStatusMessage(MsgToken(1796)); // USING BARO ALTITUDE
        } else {
          DoStatusMessage(MsgToken(1795)); // BARO ALTITUDE IS AVAILABLE
        }
        StartupStore(_T("... GPS baro source back available %s" NEWLINE),WhatTimeIsIt());
        lastvalidBaro=true;
      } else {
        static bool said=false;
        if (!said) {
          StartupStore(_T("... GPS BARO SOURCE PROBLEM, umnanaged port activity. Wrong device? %s" NEWLINE),WhatTimeIsIt());
          said=true;
        }
      }
    } 
    else {
      // last baro was Ok, currently we still have a validbaro, but no HBs...
      // Probably it is a special case when no gps fix was found on the secondary baro source.
      if (invalidBaro||!GotFirstBaroAltitude) {
        GPS_INFO.BaroAltitudeAvailable=FALSE;
        #ifdef DEBUGNPM
        StartupStore(_T(".... We still have valid baro, resetting BaroAltitude OFF %s" NEWLINE),WhatTimeIsIt());
        #endif
      }
    }
  }

  // Very important check for multiplexers: if RMZ did not get through in the past seconds, we want to
  // be very sure that there still is one incoming, otherwise we shall be UpdatingBaroSource using the old one,
  // never really updated!! This is because GGA and RMC are using RMZAvailable to UpdateBaroSource, no matter if
  // there was a real RMZ in the NMEA stream lately.
  // Normally RMZAvailable, RMCAvailable, GGA etc.etc. are reset to false when the com port is silent.
  // But RMZ is special, because it can be sent through the multiplexer from a flarm box.
  if ((LastRMZHB > 0) && LKHearthBeats > (LastRMZHB+5)) {
    #if DEBUGBARO
    StartupStore(_T(".... RMZ not updated recently, resetting HB" NEWLINE));
    #endif
    LastRMZHB = 0;
    for(auto& dev : DeviceList) {
      dev.nmeaParser.ResetRMZ();
    }
  }

  // Check baro altitude problems. This can happen for several reasons: mixed input on baro on same port,
  // faulty device, etc. The important thing is that we shall not be using baro altitude for navigation in such cases.
  // So we do this check only for the case we are actually using baro altitude.
  // A typical case is: mixed devices on same port, baro altitude disappearing because of mechanical switch,
  // but other traffic still incoming, so hearthbeats are ok. 
  static double	lastBaroAltitude=-1, lastGPSAltitude=-1;
  static unsigned int	counterSameBaro=0, counterSameHGPS=0;
  static unsigned short firstrecovery=0;
  if (GPS_INFO.BaroAltitudeAvailable && EnableNavBaroAltitude && !GPS_INFO.NAVWarning) {
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
    unsigned short timethreshold=15; // first three times,  timeout at about 1 minute
    if (firstrecovery>=3) timethreshold=40; // then about every 3 minutes
		
    if ( ((counterSameBaro > timethreshold) && (counterSameHGPS<2)) && (fabs(GPS_INFO.Altitude-GPS_INFO.BaroAltitude)>100.0) && !CALCULATED_INFO.OnGround ) {
        DoStatusMessage(MsgToken(122)); // Baro not available, Using GPS ALTITUDE
        EnableNavBaroAltitude=false;
        StartupStore(_T("... WARNING, NavBaroAltitude DISABLED due to possible fault: baro steady at %f, HGPS=%f @%s%s"), GPS_INFO.BaroAltitude, GPS_INFO.Altitude,WhatTimeIsIt(),NEWLINE);
        lastBaroAltitude=-1;
        lastGPSAltitude=-1;
        counterSameBaro=0;
        counterSameHGPS=0;
        // We do only ONE attempt to recover a faulty device, to avoid flipflopping.
        // In case of big problems, we shall have disabled the use of the faulty baro altitude, and keep it
        // incoming sporadically.
        if (firstrecovery<3) {
          GPS_INFO.BaroAltitudeAvailable=FALSE;
          // We alse reset these values, just in case we are through a mux
          GPS_INFO.AirspeedAvailable=false;
          GPS_INFO.VarioAvailable=false;
          GPS_INFO.NettoVarioAvailable=false;
          GPS_INFO.AccelerationAvailable = false;
          EnableExternalTriggerCruise = false;
          for(auto& dev : DeviceList) {
            dev.nmeaParser._Reset();
          }
          // 120824 Check this situation better> Reset is setting activeGPS true for both devices!
          lastvalidBaro=false;
          GotFirstBaroAltitude=false;
          firstrecovery++;
        }
    }
  }


    // Set some fine tuning parameters here, depending on device/situation/mode
    if (ISCAR)
      trackbearingminspeed=0; // trekking mode/car mode, min speed >0
    else
      trackbearingminspeed=1; // flymode,  min speed >1 knot

  //
  // Following is for diagnostics only
  //

  // Nothing has changed? No need to give new alerts. We might have no active gps at all, also.
  // In this case, active and lastactive are 0, nothing we can do about it.
  if (active != lastactive) {

    if (active!=0)
      StartupStore(_T(". GPS NMEA source changed to port %d  %s" NEWLINE),active,WhatTimeIsIt());
    else
      StartupStore(_T("... GPS NMEA source PROBLEM, no active GPS!  %s" NEWLINE),WhatTimeIsIt());


    if (PortMonitorMessages<15) { // do not overload pilot with messages!
      // do not say anything if we never got the first port, on startup essentially
      if ( (lastactive!=0) && active_dev &&  active_dev->nmeaParser.gpsValid ) {
        TCHAR vbuf[100];
        _stprintf(vbuf,_T("%s %d"), MsgToken(277),active); // FALLBACK USING GPS ON PORT ..
        DoStatusMessage(vbuf);
        PortMonitorMessages++;
      } 
    } else {
      if (PortMonitorMessages==15) { 
        StartupStore(_T("... GOING SILENT on too many Com reportings.  %s" NEWLINE),WhatTimeIsIt());
        DoStatusMessage(MsgToken(317)); // GOING SILENT ON COM REPORTING
        PortMonitorMessages++;	// we go to 16, and never be back here
      }
    }

    lastactive=active;
  }
  return active_dev->nmeaParser.connected;      
}

// Running at 0.2hz every 5 seconds
int ConnectionProcessTimer(int itimeout) {

  // TODO: PRINT THIS INFORMATION IN THE IGC LOG FILE, ABSOLUTELY!
  static double oldoffset=0;
  if (GPSAltitudeOffset!=oldoffset) {
    StartupStore(_T(". GPS ALTITUDE OFFSET CHANGED FROM: %f TO: %f%s"),oldoffset,GPSAltitudeOffset,NEWLINE);
    oldoffset=GPSAltitudeOffset;
  }
  devHeartBeat(devAll());  // Send Heartbeats every 5s to driver e.g. KeepAlive functions if needed

  LockComm();
  bool gpsconnect = UpdateMonitor();
  UnlockComm();


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

  if ((extGPSCONNECT == FALSE) && (GPS_INFO.NAVWarning!=true)) {
    // If gps is not connected, set navwarning to true so
    // calculations flight timers don't get updated
    LockFlightData();
    GPS_INFO.NAVWarning = true;
    UnlockFlightData();
  }

  bool DoTriggerUpdate = false;

  bool navwarning = GPS_INFO.NAVWarning;

  if((gpsconnect == false) && (s_lastGpsConnect == false)) {
    // re-draw screen every five seconds even if no GPS
    DoTriggerUpdate = true;

    devLinkTimeout(devAll());

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
        if (!(devIsDisabled(0) && devIsDisabled(1))) {

          if(devA()->nmeaParser.expire) {
            extGPSCONNECT = FALSE;
            StartupStore(_T(". ComPort RESET ordered" NEWLINE));
            if (MapSpaceMode != MSM_WELCOME) {
              InputEvents::processGlideComputer(GCE_COMMPORT_RESTART);
            }
            RestartCommPorts();
          }
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
