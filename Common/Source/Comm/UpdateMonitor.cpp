/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"

extern bool GotFirstBaroAltitude; // used by Parser
extern double LastRMZHB;	 // common to both devA and devB, updated in Parser
extern NMEAParser nmeaParser1;
extern NMEAParser nmeaParser2;

double trackbearingminspeed=0; // minimal speed to use gps bearing

//#define DEBUGBARO	1	// also in Parser to be updated
//#define DEBUGNPM	1

//
// Run every 5 seconds, approx.
//
void NMEAParser::UpdateMonitor(void) 
{
  short active; // active port number for gps
  static short lastactive=0;
  static bool  lastvalidBaro=false;
  short invalidGps=0;
  short invalidBaro=0;
  short validBaro=0; 

  // does anyone have GPS?
  if (nmeaParser1.gpsValid || nmeaParser2.gpsValid) {
	if (nmeaParser1.gpsValid && nmeaParser2.gpsValid) {
		// both valid, just use first
		nmeaParser2.activeGPS = false;
		nmeaParser1.activeGPS = true;
		active=1;
	} else {
		nmeaParser1.activeGPS = nmeaParser1.gpsValid;
		nmeaParser2.activeGPS = nmeaParser2.gpsValid;
		active= nmeaParser1.activeGPS ? 1 : 2;
	}
  } else {
	// No valid fix on any port. We use the first port with at least some data going through!
	// This will keep probably at least the time updated since the gps may still be receiving a 
	// valid time, good for us.
	if ( (LKHearthBeats-ComPortHB[0])<10 ) active=1;
	else 
		if ( (LKHearthBeats-ComPortHB[1])<10 ) active=2;
		else
			active=1;	// lets keep waiting for the first port

	if (active==1) {
		nmeaParser1.activeGPS = true;
		nmeaParser2.activeGPS = false;
	} else {
		nmeaParser1.activeGPS = false;
		nmeaParser2.activeGPS = true;
	}
  }
  if (nmeaParser2.activeGPS==true && active==1) {
	StartupStore(_T(".... GPS Update error: port 1 and 2 are active!%s"),NEWLINE);
	nmeaParser2.activeGPS=false; // force it off
	active=1; 
  }

  // wait for some seconds before monitoring, after startup
  if (LKHearthBeats<20) return;
  // Check Port 1 with no serial activity in last seconds
  if ( (LKHearthBeats-ComPortHB[0])>10 ) {
	#ifdef DEBUGNPM
	StartupStore(_T("... GPS Port 1 : no activity LKHB=%.0f CBHB=%.0f %s"),LKHearthBeats, ComPortHB[0],NEWLINE);
	#endif
	// if this is active and supposed to have a valid fix.., but no HB..
	if ( (active==1) && (nmeaParser1.gpsValid) ) {
		StartupStore(_T("... GPS Port 1 no hearthbeats, but still gpsValid: forced invalid%s"),NEWLINE);
	}
	nmeaParser1.gpsValid=false;
	invalidGps=1;
	// We want to be sure that if this device is silent, and it was providing Baro altitude,
	// now it is set to off.
	if (GPS_INFO.BaroAltitudeAvailable==TRUE) {
		if ( devA() == pDevPrimaryBaroSource || nmeaParser1.RMZAvailable 
		  || nmeaParser1.RMAAvailable || nmeaParser1.TASAvailable ) {
			invalidBaro=1;
		}
	}
	GPS_INFO.AirspeedAvailable=false;
	GPS_INFO.VarioAvailable=false;
	GPS_INFO.NettoVarioAvailable=false;
	GPS_INFO.AccelerationAvailable = false;
	EnableExternalTriggerCruise = false;
	nmeaParser1._Reset();
	nmeaParser1.activeGPS=false;
  } else {
	// We have hearth beats, is baro available?
	if ( devIsBaroSource(devA()) || nmeaParser1.RMZAvailable || nmeaParser1.RMAAvailable || nmeaParser1.TASAvailable ) // 100411
		validBaro++;
  }
  // now check also port 2
  if ( (LKHearthBeats-ComPortHB[1])>10 ) {
	#ifdef DEBUGNPM
	StartupStore(_T("... GPS Port 2 : no activity LKHB=%.0f CBHB=%.0f %s"),LKHearthBeats, ComPortHB[1],NEWLINE);
	#endif
	if ( (active==2) && (nmeaParser2.gpsValid) ) {
		StartupStore(_T("... GPS port 2 no hearthbeats, but still gpsValid: forced invalid%s"),NEWLINE);
	}
	nmeaParser2.gpsValid=false;
	invalidGps++;
	if (GPS_INFO.BaroAltitudeAvailable==TRUE) {
		if ( devB() == pDevPrimaryBaroSource || nmeaParser2.RMZAvailable 
		  || nmeaParser2.RMAAvailable || nmeaParser2.TASAvailable ) {
			invalidBaro++;
		}
	}
	GPS_INFO.AirspeedAvailable=false;
	GPS_INFO.VarioAvailable=false;
	GPS_INFO.NettoVarioAvailable=false;
	GPS_INFO.AccelerationAvailable = false;
	EnableExternalTriggerCruise = false;
	nmeaParser2._Reset();
	nmeaParser2.activeGPS=false;
  } else {
	// We have hearth beats, is baro available?
	if ( devIsBaroSource(devB()) || nmeaParser2.RMZAvailable || nmeaParser2.RMAAvailable || nmeaParser2.TASAvailable   )  // 100411
		validBaro++;
  }

  #ifdef DEBUGNPM
  if (invalidGps==2) {
	StartupStore(_T("... GPS no gpsValid available on port 1 and 2, active=%d%s"),active,NEWLINE);
  }
  if (invalidBaro>0) {
	StartupStore(_T("... Baro altitude just lost, current status=%d%s"),GPS_INFO.BaroAltitudeAvailable,NEWLINE);
  }
  #endif


  // do we really still have a baro altitude available?
  // If some baro source disappeared, let's reset it for safety. Parser will re-enable them immediately if available.
  // Assuming here that if no Baro is available, no airdata is available also
  //
  if (validBaro==0) {
	if ( GPS_INFO.BaroAltitudeAvailable ) {
		StartupStore(_T("... GPS no active baro source, and still BaroAltitudeAvailable, forced off%s"),NEWLINE);
		if (EnableNavBaroAltitude) {
			// LKTOKEN  _@M122_ = "BARO ALTITUDE NOT AVAILABLE, USING GPS ALTITUDE" 
			DoStatusMessage(MsgToken(122));
			PortMonitorMessages++;
		} else {
			// LKTOKEN  _@M121_ = "BARO ALTITUDE NOT AVAILABLE" 
			DoStatusMessage(MsgToken(121));
		}
		GPS_INFO.BaroAltitudeAvailable=false;
		// We alse reset these values, just in case we are through a mux
		GPS_INFO.AirspeedAvailable=false;
		GPS_INFO.VarioAvailable=false;
		GPS_INFO.NettoVarioAvailable=false;
		GPS_INFO.AccelerationAvailable = false;
		EnableExternalTriggerCruise = false;
		nmeaParser1._Reset();
		nmeaParser2._Reset();
		// 120824 Check this situation better> Reset is setting activeGPS true for both devices!
		lastvalidBaro=false;
	}
  } else {
	if ( lastvalidBaro==false) {
		#if DEBUGBARO
		TCHAR devname[50];
		if (pDevPrimaryBaroSource) {
			LK_tcsncpy(devname,pDevPrimaryBaroSource->Name,49);
		} else {
			_tcscpy(devname,_T("unknown"));
		}
		StartupStore(_T("... GPS baro source back available from <%s>%s"),devname,NEWLINE);
		#else
		StartupStore(_T("... GPS baro source back available%s"),NEWLINE);
		#endif

		if (GotFirstBaroAltitude) {
			if (EnableNavBaroAltitude) {
				DoStatusMessage(MsgToken(1796)); // USING BARO ALTITUDE
			} else {
				DoStatusMessage(MsgToken(1795)); // BARO ALTITUDE IS AVAILABLE
			}
			lastvalidBaro=true;
		}
	} 
	else {
		// last baro was Ok, currently we still have a validbaro, but no HBs...
		// Probably it is a special case when no gps fix was found on the secondary baro source.
		if (invalidBaro||!GotFirstBaroAltitude) {
			GPS_INFO.BaroAltitudeAvailable=FALSE;
			#ifdef DEBUGNPM
			StartupStore(_T(".... We still have valid baro, resetting BaroAltitude OFF\n"));
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
  if ( (nmeaParser1.RMZAvailable || nmeaParser2.RMZAvailable) && (LKHearthBeats > (LastRMZHB+5))) {
	#if DEBUGBARO
	StartupStore(_T(".... RMZ not updated recently, resetting HB\n"));
	#endif
	nmeaParser1.RMZAvailable = FALSE;
	nmeaParser2.RMZAvailable = FALSE;
  }

  // Set some fine tuning parameters here, dependint on device/situation/mode
  if (ISCAR)
	trackbearingminspeed=0; // trekking mode/car mode, min speed >0
  else
	trackbearingminspeed=1; // flymode,  min speed >1 knot

  // Following diagnostics only
  if (active == lastactive) return;
  if (lastactive==0) {
	lastactive=active;
	StartupStore(_T(". GPS NMEA init delegated to port %d%s"),active,NEWLINE);
	return;
  }

  lastactive=active;
  // in case of no gps at all, port 1 is selected but we dont want to tell unless really working
  if (PortMonitorMessages<10) { // 100221 do not overload pilot with messages!
	StartupStore(_T("... GPS NMEA source changed to port %d %s"),active,NEWLINE);
	if (nmeaParser1.gpsValid || nmeaParser2.gpsValid){
		TCHAR vbuf[100]; _stprintf(vbuf,_T("%s %d"),
	// LKTOKEN  _@M277_ = "FALLBACK USING GPS ON PORT" 
		gettext(TEXT("_@M277_")),active);
		DoStatusMessage(vbuf);
	}
	PortMonitorMessages++;
  } else {
	if (PortMonitorMessages==10) { // 100221
		StartupStore(_T("... GOING SILENT on too many Com reportings.%s"),NEWLINE);
	// LKTOKEN  _@M317_ = "GOING SILENT ON COM REPORTING" 
		DoStatusMessage(gettext(TEXT("_@M317_")));
		PortMonitorMessages++;
	} else
		PortMonitorMessages++;
  }


}

