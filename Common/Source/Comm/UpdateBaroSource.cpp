/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"
#include "Baro.h"

bool GotFirstBaroAltitude=false;

// #define DEBUGBARO	1

/*
 * We update only if incoming from PrimaryBaroSource, or if it is without a device (parserid>0).
 * FLARM has the highest priority since it is the most common used Baro device (hopefully)
 *
 * We must consider the case of multiplexers sending mixed devices, and the special case when
 * one of these devices stop sending baro altitude, 
 *
 * UpdateMonitor is already setting baroaltitude off if no data is coming from that nmea port.
 *
 * CAUTION do not use devicedescriptor's pointers to functions without LockComm!!
 */


bool UpdateBaroSource( NMEA_INFO* pGPS, short parserid, const PDeviceDescriptor_t d, double fAlt)
{

  //static double	lastBaroHB=0;		// updated only when baro is assigned as valid
  static bool notifyErr=true;

  static unsigned lastRMZfromFlarmHB=0;
  static bool	havePrimaryBaroSource=false;
  static unsigned lastPrimaryBaroSourceHB=0;


  #if DEBUGBARO
  if (parserid>0 && d) StartupStore(_T(".... Abnormal call of updatebarosource!\n"));

  if (parserid>0) {
	StartupStore(_T("... UpdateBaroSource: from internal parser type <%d>, alt=%.1f\n"),parserid,fAlt);
  } else {
	StartupStore(_T("... UpdateBaroSource: port=%d <%s> primary=%d secondary=%d disabled=%d alt=%f\n"),
	d->PortNumber, d->Name, d==pDevPrimaryBaroSource, d==pDevSecondaryBaroSource, d->Disabled, fAlt);
  }
  #endif
  if (fAlt>30000 || fAlt<-1000) {
	if (notifyErr) {
		StartupStore(_T("... RECEIVING INVALID BARO ALTITUDE: %f\n"),fAlt);
		DoStatusMessage(MsgToken(1530));
		notifyErr=false;
	}
  } else  {
	GotFirstBaroAltitude=true;
  }

  // First we keep memory of what we got so far.
  // RMZ_FLARM must be granted to be real (not a ghost baro), all checks in calling function.
  if ( parserid == BARO__RMZ_FLARM ) {
	#if DEBUGBARO
	StartupStore(_T("... we have RMZ from Flarm\n"));
	#endif
	pGPS->haveRMZfromFlarm=true;
	lastRMZfromFlarmHB=LKHearthBeats;

  }
  if ( (d!=NULL) && (d == pDevPrimaryBaroSource)) {
	#if DEBUGBARO
	StartupStore(_T("... we have PrimaryBaroSource\n"));
	#endif
	havePrimaryBaroSource=true;
	lastPrimaryBaroSourceHB=LKHearthBeats;
  }

  // Then we update for missing sentences from broken comms
  if (pGPS->haveRMZfromFlarm && (LKHearthBeats > (lastRMZfromFlarmHB+9))) { // 9 is 9 HBs = 4.5s
	// For some reason the RMZ is not coming anymore. So we want to use any other baro available.
	StartupStore(_T("... Flarm baro altitude missing since 5 seconds, fallback required\n"));
	pGPS->haveRMZfromFlarm=false;
  }
  if (havePrimaryBaroSource && (LKHearthBeats > (lastPrimaryBaroSourceHB+9))) {
	StartupStore(_T("... Primary baro altitude missing since 5 seconds, fallback required\n"));
	havePrimaryBaroSource=false;
  }

  // If RMZ from flarm, we ony want that!
  // A multiplexer can filter flarm RMZ, this is why we ensure that we have one available.
  if (pGPS->haveRMZfromFlarm) {
	if ( parserid == BARO__RMZ_FLARM) {
		#if DEBUGBARO
		StartupStore(_T("....> Using RMZ from Flarm:  %.1f\n"),fAlt);
		#endif
		pGPS->BaroAltitudeAvailable = true;
		pGPS->BaroAltitude = fAlt;
		//lastBaroHB=LKHearthBeats;
		return true;
	}

	// When Flarm is available, we only use its RMZ. Nothing else.
	// We must also consider the case of dual RMZ from dual port. Only one if from flarm.
	#if DEBUGBARO
	StartupStore(_T("....> Discarding baro altitude  %.1f, not from Flarm\n"),fAlt);
	#endif
	return false;
  }

  // Special devices that have internal custom driver as RMZ
  // They are supposed to be lone and unmixed
  if ( (parserid>=BARO__CUSTOMFROM) && (parserid<=BARO__CUSTOMTO)) {
	#if DEBUGBARO
	StartupStore(_T("....> Using custom device <%d>, alt=%.1f\n"),parserid,fAlt);
	#endif
	pGPS->BaroAltitudeAvailable = true;
	pGPS->BaroAltitude = fAlt;
	//lastBaroHB=LKHearthBeats;
	return true;
  }

  // Otherwise we go for primary device
  if (havePrimaryBaroSource ) {
	if ((d!=NULL) && (d == pDevPrimaryBaroSource)) {
		#if DEBUGBARO
		StartupStore(_T("....> Using Primary alt=%.1f\n"),fAlt);
		#endif
		pGPS->BaroAltitudeAvailable = true;
		pGPS->BaroAltitude = fAlt;
		//lastBaroHB=LKHearthBeats;
		return true;
  	}
	#if DEBUGBARO
	StartupStore(_T("....> Discarding not-primary baro alt=%.1f\n"),fAlt);
	#endif
	return false;
  }

  // .. else we use the baro, probably from SecondaryBaroSource
  #if DEBUGBARO
  StartupStore(_T("....> Using fallback baro alt=%.1f\n"),fAlt);
  #endif
  pGPS->BaroAltitudeAvailable = true;
  pGPS->BaroAltitude = fAlt;
  //lastBaroHB=LKHearthBeats;
  return true;


}

