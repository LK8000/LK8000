/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"

extern bool GotFirstBaroAltitude; // used by UpdateBaroSource
extern double LastRMZHB;	 // common to both devA and devB, updated in Parser
extern NMEAParser nmeaParser1;
extern NMEAParser nmeaParser2;

double trackbearingminspeed=0; // minimal speed to use gps bearing

//#define DEBUGBARO	1	// also needed in UpdateBaroSource
//#define DEBUGNPM	1

//
// Run every 5 seconds, approx.
// This is the hearth of LK. Questions? Ask Paolo..
// THIS IS RUNNING WITH LockComm  from ConnectionProcessTimer .
//
void NMEAParser::UpdateMonitor(void) 
{
  short active=0; // active port number for gps
  static short lastactive=0;
  static bool  lastvalidBaro=false;
  static bool wasSilent[2]={false,false};
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
		// only one valid, pick it up
		nmeaParser1.activeGPS = nmeaParser1.gpsValid;
		nmeaParser2.activeGPS = nmeaParser2.gpsValid;
		active= nmeaParser1.activeGPS ? 1 : 2;
	}
  } else {
	// No valid fix on any port. We use the first port with at least some data going through!
	// This will keep probably at least the time updated since the gps may still be receiving a 
	// valid time, good for us.
	if ( (LKHearthBeats-ComPortHB[0])<10 ) {
		// It is not granted that devA is really a GPS source.
		// Very unlikely, but possible..
		if (devIsGPSSource(devA())) active=1;
	} else {
		if ( (LKHearthBeats-ComPortHB[1])<10 ) {
			// portB is really active, although there is no valid fix on it.
			// Before electing it to gps, lets be sure it really has one!
			// LKEXT1 and other instruments do not provide GPS source in fact.
			if (devIsGPSSource(devB())) active=2;
		} else {
			// nothing coming in from any port, recently.
			if (devIsGPSSource(devA())) active=1;	// lets keep waiting for the first port
		}
	}

	switch(active) {
		case 0:
			nmeaParser1.activeGPS = false;
			nmeaParser2.activeGPS = false;
			break;
		case 1:
			nmeaParser1.activeGPS = true;
			nmeaParser2.activeGPS = false;
			break;
		case 2:
			nmeaParser1.activeGPS = false;
			nmeaParser2.activeGPS = true;
			break;
		default:
			nmeaParser1.activeGPS = false;
			nmeaParser2.activeGPS = false;
			LKASSERT(0);
			break;
	}
  }


  if (nmeaParser2.activeGPS==true && active==1) {
	StartupStore(_T(".... GPS Update error: port 1 and 2 are active! %s%s"),WhatTimeIsIt(),NEWLINE);
	nmeaParser2.activeGPS=false; // force it off
	active=1; 
  }

  // wait for some seconds before monitoring, after startup
  if (LKHearthBeats<20) return;

  /* check if Flarm disappeared after 30 seconds no activity */
  if (GPS_INFO.FLARM_Available && ((GPS_INFO.Time -LastFlarmCommandTime)> 30) )
  {
	static unsigned short MessageCnt =0;
	if(MessageCnt <10)
	{
		MessageCnt++;
		StartupStore(_T(". FLARM lost! Disable FLARM functions !%s"),NEWLINE);
		DoStatusMessage(gettext(TEXT("_@M947_"))); // _@M947_ "FLARM SIGNAL LOST"
	}
	GPS_INFO.FLARM_Available = false;
	GPS_INFO.FLARM_HW_Version =0.0;
	GPS_INFO.FLARM_SW_Version =0.0;
  }

  // Check Port 1 with no serial activity in last seconds
  if ( (LKHearthBeats-ComPortHB[0])>10 ) {
	#ifdef DEBUGNPM
	StartupStore(_T("... GPS Port 1 : no activity LKHB=%.0f CBHB=%.0f %s"),LKHearthBeats, ComPortHB[0],NEWLINE);
	#endif
	// if this is active and supposed to have a valid fix.., but no HB..
	if ( (active==1) && (nmeaParser1.gpsValid) ) {
		StartupStore(_T("... GPS Port 1 no hearthbeats, but still gpsValid: forced invalid  %s%s"),WhatTimeIsIt(),NEWLINE);
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
	nmeaParser1._Reset();
	nmeaParser1.activeGPS=false; // because Reset is setting it to true
	// We reset some flags globally only once in case of device gone silent 
	if (!devIsDisabled(0) && !wasSilent[0]) {
		GPS_INFO.AirspeedAvailable=false;
		GPS_INFO.VarioAvailable=false;
		GPS_INFO.NettoVarioAvailable=false;
		GPS_INFO.AccelerationAvailable = false;
		EnableExternalTriggerCruise = false;
		wasSilent[0]=true;
	}
  } else {
	wasSilent[0]=false;
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
		StartupStore(_T("... GPS port 2 no hearthbeats, but still gpsValid: forced invalid  %s%s"),WhatTimeIsIt(),NEWLINE);
	}
	nmeaParser2.gpsValid=false;
	invalidGps++;
	if (GPS_INFO.BaroAltitudeAvailable==TRUE) {
		if ( devB() == pDevPrimaryBaroSource || nmeaParser2.RMZAvailable 
		  || nmeaParser2.RMAAvailable || nmeaParser2.TASAvailable ) {
			invalidBaro++;
		}
	}
	nmeaParser2._Reset();
	nmeaParser2.activeGPS=false; // because Reset is setting it to true
	if (!devIsDisabled(1) && !wasSilent[1]) {
		GPS_INFO.AirspeedAvailable=false;
		GPS_INFO.VarioAvailable=false;
		GPS_INFO.NettoVarioAvailable=false;
		GPS_INFO.AccelerationAvailable = false;
		EnableExternalTriggerCruise = false;
		wasSilent[1]=true;
	}
  } else {
	wasSilent[1]=false;
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
		#endif

		if (GotFirstBaroAltitude) {
			if (EnableNavBaroAltitude) {
				DoStatusMessage(MsgToken(1796)); // USING BARO ALTITUDE
			} else {
				DoStatusMessage(MsgToken(1795)); // BARO ALTITUDE IS AVAILABLE
			}
			StartupStore(_T("... GPS baro source back available %s%s"),WhatTimeIsIt(),NEWLINE);
			lastvalidBaro=true;
		} else {
			static bool said=false;
			if (!said) {
				StartupStore(_T("... GPS BARO SOURCE PROBLEM, umnanaged port activity. Wrong device? %s%s"),WhatTimeIsIt(),NEWLINE);
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
		
	if ( (counterSameBaro > timethreshold) && (counterSameHGPS<2) ) {
			DoStatusMessage(MsgToken(122)); // Baro not available, Using GPS ALTITUDE
			EnableNavBaroAltitude=false;
			StartupStore(_T("... WARNING, NavBaroAltitude DISABLED due to possible fault: baro steady at %f, HGPS=%f%s"),
			GPS_INFO.BaroAltitude, GPS_INFO.Altitude,NEWLINE);
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
				nmeaParser1._Reset();
				nmeaParser2._Reset();
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
  if (active == lastactive) return;

  if (active!=0)
	StartupStore(_T(". GPS NMEA source changed to port %d  %s%s"),active,WhatTimeIsIt(),NEWLINE);
  else
	StartupStore(_T("... GPS NMEA source PROBLEM, no active GPS!  %s%s"),WhatTimeIsIt(),NEWLINE);


  if (PortMonitorMessages<15) { // do not overload pilot with messages!
	// do not say anything if we never got the first port, on startup essentially
	if ((lastactive!=0) && (nmeaParser1.gpsValid || nmeaParser2.gpsValid)){
		TCHAR vbuf[100];
		_stprintf(vbuf,_T("%s %d"), MsgToken(277),active); // FALLBACK USING GPS ON PORT ..
		DoStatusMessage(vbuf);
		PortMonitorMessages++;
	} 
  } else {
	if (PortMonitorMessages==15) { 
		StartupStore(_T("... GOING SILENT on too many Com reportings.  %s%s"),WhatTimeIsIt(),NEWLINE);
		DoStatusMessage(MsgToken(317)); // GOING SILENT ON COM REPORTING
		PortMonitorMessages++;	// we go to 16, and never be back here
	}
  }

  lastactive=active;

}

