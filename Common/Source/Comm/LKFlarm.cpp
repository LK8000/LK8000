/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"
#include "FlarmRadar.h"

#include "FlarmCalculations.h"
FlarmCalculations flarmCalculations;

//
// This should be renamed to something clear to understand, being global.
//
LastPositions asRingBuf[NO_TRACE_PTS];
int iLastPtr=0;
bool bBuffFull;


void CheckBackTarget(int flarmslot);

//#define DEBUG_LKT	1



void FLARM_RefreshSlots(NMEA_INFO *GPS_INFO) {
  int i;
  double passed;
  if (GPS_INFO->FLARM_Available) {
	static int iTraceSpaceCnt = 0;
	if(iTraceSpaceCnt++ > GC_TRACE_TIME_SKIP)
		iTraceSpaceCnt =0;
	#ifdef DEBUG_LKT
	StartupStore(_T("... [CALC thread] RefreshSlots\n"));
	#endif

	for (i=0; i<FLARM_MAX_TRAFFIC; i++) {
		if (GPS_INFO->FLARM_Traffic[i].ID>0) {

			if ( GPS_INFO->Time< GPS_INFO->FLARM_Traffic[i].Time_Fix) {
				// time gone back to to Replay mode?
				#ifdef DEBUG_LKT
				StartupStore(_T("...... Refresh Back in time! Removing:%s"),NEWLINE);
				FLARM_DumpSlot(GPS_INFO,i);
				#endif
				if (GPS_INFO->FLARM_Traffic[i].Locked) {
					#ifdef DEBUG_LKT
					StartupStore(_T("...... (it was a LOCKED target, unlocking)%s"),NEWLINE);
					#endif
					LKTargetIndex=-1;
					LKTargetType=LKT_TYPE_NONE;
					// reset Virtual waypoint in any case
					WayPointList[RESWP_FLARMTARGET].Latitude   = RESWP_INVALIDNUMBER;
					WayPointList[RESWP_FLARMTARGET].Longitude  = RESWP_INVALIDNUMBER;
					WayPointList[RESWP_FLARMTARGET].Altitude   = RESWP_INVALIDNUMBER;
				}
				FLARM_EmptySlot(GPS_INFO,i);
				continue;
			}

			passed= GPS_INFO->Time-GPS_INFO->FLARM_Traffic[i].Time_Fix;

			// if time has passed > zombie, then we remove it
			if (passed > LKTime_Zombie) {
				if (GPS_INFO->FLARM_Traffic[i].Locked) {
					#ifdef DEBUG_LKT
					StartupStore(_T("...... Zombie overtime index=%d is LOCKED, no remove\n"),i);
					#endif
					continue;
				}
				#ifdef DEBUG_LKT
				StartupStore(_T("... Refresh Removing old zombie (passed=%f Fix=%f Now=%f):%s"),
					passed,
					GPS_INFO->FLARM_Traffic[i].Time_Fix,
					GPS_INFO->Time,
					NEWLINE);
				FLARM_DumpSlot(GPS_INFO,i);
				#endif
				FLARM_EmptySlot(GPS_INFO,i);
				continue;
			}

			// if time has passed > ghost, then it is a zombie
			// Ghosts are not visible on map and radar, only in infopages
			if (passed > LKTime_Ghost) {
				if (GPS_INFO->FLARM_Traffic[i].Status == LKT_ZOMBIE) continue;
				#ifdef DEBUG_LKT
				StartupStore(_T("... Refresh Change to zombie:%s"),NEWLINE);
				FLARM_DumpSlot(GPS_INFO,i);
				#endif
				GPS_INFO->FLARM_Traffic[i].Status = LKT_ZOMBIE;
				continue;
			}

			// if time has passed > real, than it is a ghost
			// Shadows are shown on map as reals.
			if (passed > LKTime_Real) {
				if (GPS_INFO->FLARM_Traffic[i].Status == LKT_GHOST) continue;
				#ifdef DEBUG_LKT
				StartupStore(_T("... Refresh Change to ghost:%s"),NEWLINE);
				FLARM_DumpSlot(GPS_INFO,i);
				#endif
				GPS_INFO->FLARM_Traffic[i].Status = LKT_GHOST;
				continue;
			}

			// Then it is real traffic
			GPS_INFO->FLARM_Traffic[i].Status = LKT_REAL; // 100325 BUGFIX missing
            if(iTraceSpaceCnt == 0)
            {

		      asRingBuf[iLastPtr].fLat = GPS_INFO->FLARM_Traffic[i].Latitude;
		      asRingBuf[iLastPtr].fLon = GPS_INFO->FLARM_Traffic[i].Longitude;

		      double Vario = GPS_INFO->FLARM_Traffic[i].Average30s;
			  int iColorIdx = (int)(2*Vario  -0.5)+NO_VARIO_COLORS/2;
			  iColorIdx = max( iColorIdx, 0);
			  iColorIdx = min( iColorIdx, NO_VARIO_COLORS-1);


		      asRingBuf[iLastPtr].iColorIdx = iColorIdx;
		      iLastPtr++;
		      if(iLastPtr >= NO_TRACE_PTS)
		      {
		        iLastPtr=0;
		        bBuffFull = true;
		      }
            }
			/*
			} else {
				if (GPS_INFO->FLARM_Traffic[i].AlarmLevel>0) {
					GaugeFLARM::Suppress = false; // NO USE
				}
			}
			*/
		}
	}
  }
}


// Reset a flarm slot
void FLARM_EmptySlot(NMEA_INFO *GPS_INFO,int i) {

#ifdef DEBUG_LKT
  StartupStore(_T("... --- EmptySlot %d : ID=%0x <%s> Cn=<%s>\n"),  i, 
	GPS_INFO->FLARM_Traffic[i].ID, 
	GPS_INFO->FLARM_Traffic[i].Name,
	GPS_INFO->FLARM_Traffic[i].Cn);
#endif

  if (i<0 || i>=FLARM_MAX_TRAFFIC) return;
  GPS_INFO->FLARM_Traffic[i].ID= 0;
  GPS_INFO->FLARM_Traffic[i].Name[0] = 0;
  GPS_INFO->FLARM_Traffic[i].Cn[0] = 0;
  GPS_INFO->FLARM_Traffic[i].Speed=0;
  GPS_INFO->FLARM_Traffic[i].Altitude=0;
  GPS_INFO->FLARM_Traffic[i].Status = LKT_EMPTY;
  GPS_INFO->FLARM_Traffic[i].AlarmLevel=0;
  GPS_INFO->FLARM_Traffic[i].RelativeNorth=0;
  GPS_INFO->FLARM_Traffic[i].RelativeEast=0;
  GPS_INFO->FLARM_Traffic[i].RelativeAltitude=0;
  GPS_INFO->FLARM_Traffic[i].IDType=0;
  GPS_INFO->FLARM_Traffic[i].TrackBearing=0;
  GPS_INFO->FLARM_Traffic[i].TurnRate=0;
  GPS_INFO->FLARM_Traffic[i].ClimbRate=0;
  GPS_INFO->FLARM_Traffic[i].Type=0;
  GPS_INFO->FLARM_Traffic[i].Time_Fix=0;
  GPS_INFO->FLARM_Traffic[i].Average30s=0;
  GPS_INFO->FLARM_Traffic[i].Locked = false;
  GPS_INFO->FLARM_Traffic[i].UpdateNameFlag = false;

}


#ifdef DEBUG_LKT
void FLARM_DumpSlot(NMEA_INFO *GPS_INFO,int i) {
  TCHAR dump[256];
  _stprintf(dump, _T("... DumpSlot (%d) status=%d id=<%lx> Name=<%s> Cn=<%s> Speed=%.0f rAlt=%.0f  %s"),
	i,
	GPS_INFO->FLARM_Traffic[i].Status,
	GPS_INFO->FLARM_Traffic[i].ID,
	GPS_INFO->FLARM_Traffic[i].Name,
	GPS_INFO->FLARM_Traffic[i].Cn,
	GPS_INFO->FLARM_Traffic[i].Speed,
	GPS_INFO->FLARM_Traffic[i].Altitude, 
	NEWLINE);
  StartupStore(dump);
}
#endif	


#include "InputEvents.h"

double FLARM_NorthingToLatitude = 0.0;
double FLARM_EastingToLongitude = 0.0;


BOOL NMEAParser::PFLAU(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *GPS_INFO)
{
  static int old_flarm_rx = 0;
  static bool sayflarmavailable=true; // 100325

  GPS_INFO->FLARM_Available = true;
  isFlarm = true;
  if ( sayflarmavailable ) {
	// LKTOKEN  _@M279_ = "FLARM DETECTED" 
	DoStatusMessage(gettext(TEXT("_@M279_")));
	sayflarmavailable=false;
  }

  // calculate relative east and north projection to lat/lon

  double delta_lat = 0.01;
  double delta_lon = 0.01;

  double dlat;
  DistanceBearing(GPS_INFO->Latitude, GPS_INFO->Longitude,
                  GPS_INFO->Latitude+delta_lat, GPS_INFO->Longitude,
                  &dlat, NULL);
  double dlon;
  DistanceBearing(GPS_INFO->Latitude, GPS_INFO->Longitude,
                  GPS_INFO->Latitude, GPS_INFO->Longitude+delta_lon,
                  &dlon, NULL);

  if ((fabs(dlat)>0.0)&&(fabs(dlon)>0.0)) {
    FLARM_NorthingToLatitude = delta_lat / dlat;
    FLARM_EastingToLongitude = delta_lon / dlon;
  } else {
    FLARM_NorthingToLatitude=0.0;
    FLARM_EastingToLongitude=0.0;
  }

  swscanf(String,
	  TEXT("%hu,%hu,%hu,%hu"),
	  &GPS_INFO->FLARM_RX, // number of received FLARM devices
	  &GPS_INFO->FLARM_TX, // Transmit status
	  &GPS_INFO->FLARM_GPS, // GPS status
	  &GPS_INFO->FLARM_AlarmLevel); // Alarm level of FLARM (0-3)

  // process flarm updates

  if ((GPS_INFO->FLARM_RX) && (old_flarm_rx==0)) {
    // traffic has appeared..
    InputEvents::processGlideComputer(GCE_FLARM_TRAFFIC);
  }
  if (GPS_INFO->FLARM_RX > old_flarm_rx) {
    // re-set suppression of gauge, as new traffic has arrived
    //    GaugeFLARM::Suppress = false;
  }
  if ((GPS_INFO->FLARM_RX==0) && (old_flarm_rx)) {
    // traffic has disappeared..
    InputEvents::processGlideComputer(GCE_FLARM_NOTRAFFIC);
  }
  // TODO feature: add another event for new traffic.

  old_flarm_rx = GPS_INFO->FLARM_RX;

  return FALSE;
}



int FLARM_FindSlot(NMEA_INFO *GPS_INFO, long Id)
{
  int i;
  for (i=0; i<FLARM_MAX_TRAFFIC; i++) {

	// find position in existing slot
	if (Id==GPS_INFO->FLARM_Traffic[i].ID) {
		//#ifdef DEBUG_LKT
		//StartupStore(_T("... FindSlot ID=%lx found in slot %d\n"),Id,i);
		//#endif
		return i;
	}
	// find old empty slot
  }
  // not found, so try to find an empty slot
  for (i=0; i<FLARM_MAX_TRAFFIC; i++) {
	if (GPS_INFO->FLARM_Traffic[i].ID<=0) { // 100327 <= was ==
		// this is a new target
		#ifdef DEBUG_LKT
		StartupStore(_T("... FLARM ID=%lx assigned NEW SLOT=%d\n"),Id,i);
		#endif
		return i;
	}
  }
  // remove a zombie to make place
  int toremove=-1;
  for (i=0; i<FLARM_MAX_TRAFFIC; i++) {
	if ( (GPS_INFO->FLARM_Traffic[i].ID>0) && (GPS_INFO->FLARM_Traffic[i].Status==LKT_ZOMBIE) &&
		(!GPS_INFO->FLARM_Traffic[i].Locked) ) { 
		// if this is the first zombie, assign it and continue searching
		if (toremove==-1) {
			toremove=i;
		} else {
			// if this zombie is older than previous one
			if ( GPS_INFO->FLARM_Traffic[i].Time_Fix < GPS_INFO->FLARM_Traffic[toremove].Time_Fix ) {
				toremove=i;
			}
		}
	}
  }
  // did we find a zombie to remove?
  if (toremove>=0) {
	#ifdef DEBUG_LKT
	StartupStore(_T("... Removing OLDEST zombie:%s"),NEWLINE);
	FLARM_DumpSlot(GPS_INFO,toremove);
	#endif
	FLARM_EmptySlot(GPS_INFO,toremove);
	return toremove;
  }
  // remove a ghost to make place
  toremove=-1;
  for (i=0; i<FLARM_MAX_TRAFFIC; i++) {
	if ( (GPS_INFO->FLARM_Traffic[i].ID>0) && (GPS_INFO->FLARM_Traffic[i].Status==LKT_GHOST) &&
		(!GPS_INFO->FLARM_Traffic[i].Locked) ) { 
		// if this is the first ghost, assign it and continue searching
		if (toremove==-1) {
			toremove=i;
		} else {
			// if this ghost is older than previous one
			if ( GPS_INFO->FLARM_Traffic[i].Time_Fix < GPS_INFO->FLARM_Traffic[toremove].Time_Fix ) {
				toremove=i;
			}
		}
	}
  }
  // did we find a ghost to remove?
  if (toremove>=0) {
	#ifdef DEBUG_LKT
	StartupStore(_T("... Removing OLDEST ghost:%s"),NEWLINE);
	FLARM_DumpSlot(GPS_INFO,toremove);
	#endif
	FLARM_EmptySlot(GPS_INFO,toremove);
	return toremove;
  }

  #ifdef DEBUG_LKT
  StartupStore(_T("... ID=<%lx> NO SPACE in slots!\n"),Id);
  #endif
  // still not found and no empty slots left, buffer is full
  return -1;
}




BOOL NMEAParser::PFLAA(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *GPS_INFO)
{
  int flarm_slot = 0;

  isFlarm = true;

  // 5 id, 6 digit hex
  long ID;
  swscanf(params[5],TEXT("%lx"), &ID);
//  unsigned long uID = ID;

  flarm_slot = FLARM_FindSlot(GPS_INFO, ID);
  if (flarm_slot<0) {
    // no more slots available,
	#ifdef DEBUG_LKT
	StartupStore(_T("... NO SLOTS for Flarm traffic, too many ids!%s"),NEWLINE);
	#endif
	return FALSE;
  }

  // before changing timefix, see if it was an old target back locked in!
  CheckBackTarget(flarm_slot);
  // and then set time of fix to current time
  GPS_INFO->FLARM_Traffic[flarm_slot].Time_Fix = GPS_INFO->Time;

  TCHAR nString[MAX_NMEA_LEN+1];
  unsigned int i, j;
  for (i=0, j=0; i<_tcslen(String); i++) {
	// if not a comma, copy and proceed
	if (String[i] != _T(',')) {
		nString[j++]=String[i];
		continue;
	}
	// there was a comma, but the next one is not a comma, so ok..
	if (String[i+1] != _T(',') ) {
		nString[j++]=String[i];
		continue;
	}
	// We have a bad ,, case that scanf cannot bear with, so we add a 0
	nString[j++] = String[i];
	nString[j++] = _T('0');
  }
  nString[j]=_T('\0');

  //#ifdef DEBUG_LKT
  //StartupStore(_T("PFLAA: %s%s"),nString,NEWLINE);
  //#endif

  _stscanf(nString,
	  TEXT("%hu,%lf,%lf,%lf,%hu,%lx,%lf,%lf,%lf,%lf,%hu"),
	  &GPS_INFO->FLARM_Traffic[flarm_slot].AlarmLevel, // unsigned short 0
	  &GPS_INFO->FLARM_Traffic[flarm_slot].RelativeNorth, //  1	
	  &GPS_INFO->FLARM_Traffic[flarm_slot].RelativeEast, //   2
	  &GPS_INFO->FLARM_Traffic[flarm_slot].RelativeAltitude, //  3
	  &GPS_INFO->FLARM_Traffic[flarm_slot].IDType, // unsigned short     4
	  &GPS_INFO->FLARM_Traffic[flarm_slot].ID, // 6 char hex
	  &GPS_INFO->FLARM_Traffic[flarm_slot].TrackBearing, // double       6
	  &GPS_INFO->FLARM_Traffic[flarm_slot].TurnRate, // double           7
	  &GPS_INFO->FLARM_Traffic[flarm_slot].Speed, // double              8 m/s
	  &GPS_INFO->FLARM_Traffic[flarm_slot].ClimbRate, // double          9 m/s
	  &GPS_INFO->FLARM_Traffic[flarm_slot].Type); // unsigned short     10
  // 1 relativenorth, meters  
  GPS_INFO->FLARM_Traffic[flarm_slot].Latitude = 
    GPS_INFO->FLARM_Traffic[flarm_slot].RelativeNorth *FLARM_NorthingToLatitude + GPS_INFO->Latitude;
  // 2 relativeeast, meters
  GPS_INFO->FLARM_Traffic[flarm_slot].Longitude = 
    GPS_INFO->FLARM_Traffic[flarm_slot].RelativeEast *FLARM_EastingToLongitude + GPS_INFO->Longitude;

  // we need to compare with BARO altitude FLARM relative Alt difference!
  if (GPS_INFO->BaroAltitude>0) // just to be sure
	GPS_INFO->FLARM_Traffic[flarm_slot].Altitude = GPS_INFO->FLARM_Traffic[flarm_slot].RelativeAltitude + GPS_INFO->BaroAltitude;
  else
	GPS_INFO->FLARM_Traffic[flarm_slot].Altitude = GPS_INFO->FLARM_Traffic[flarm_slot].RelativeAltitude + GPS_INFO->Altitude;



  GPS_INFO->FLARM_Traffic[flarm_slot].Average30s = flarmCalculations.Average30s(
	  GPS_INFO->FLARM_Traffic[flarm_slot].ID,
	  GPS_INFO->Time,
	  GPS_INFO->FLARM_Traffic[flarm_slot].Altitude);

  TCHAR *name = GPS_INFO->FLARM_Traffic[flarm_slot].Name;
  //TCHAR *cn = GPS_INFO->FLARM_Traffic[flarm_slot].Cn;
  // If there is no name yet, or if we have a pending update event..
  if (!_tcslen(name) || GPS_INFO->FLARM_Traffic[flarm_slot].UpdateNameFlag ) {

	#ifdef DEBUG_LKT
	if (GPS_INFO->FLARM_Traffic[flarm_slot].UpdateNameFlag ) {
		StartupStore(_T("... UpdateNameFlag for slot %d\n"),flarm_slot);
	} else {
		StartupStore(_T("... First lookup name for slot %d\n"),flarm_slot);
	}
	#endif

	GPS_INFO->FLARM_Traffic[flarm_slot].UpdateNameFlag=false; // clear flag first
	TCHAR *fname = LookupFLARMDetails(GPS_INFO->FLARM_Traffic[flarm_slot].ID);
	if (fname) {
		LK_tcsncpy(name,fname,MAXFLARMNAME);

		//  Now we have the name, so lookup also for the Cn
		// This will return either real Cn or Name, again
		TCHAR *cname = LookupFLARMCn(GPS_INFO->FLARM_Traffic[flarm_slot].ID);
		if (cname) {
			int cnamelen=_tcslen(cname);
			if (cnamelen<=MAXFLARMCN) {
				_tcscpy( GPS_INFO->FLARM_Traffic[flarm_slot].Cn, cname);
			} else {
				// else probably it is the Name again, and we create a fake Cn
				GPS_INFO->FLARM_Traffic[flarm_slot].Cn[0]=cname[0];
				GPS_INFO->FLARM_Traffic[flarm_slot].Cn[1]=cname[cnamelen-2];
				GPS_INFO->FLARM_Traffic[flarm_slot].Cn[2]=cname[cnamelen-1];
				GPS_INFO->FLARM_Traffic[flarm_slot].Cn[3]=_T('\0');
			}
		} else {
			_tcscpy( GPS_INFO->FLARM_Traffic[flarm_slot].Cn, _T("Err"));
		}

		#ifdef DEBUG_LKT
		StartupStore(_T("... PFLAA Name to FlarmSlot=%d ID=%lx Name=<%s> Cn=<%s>\n"),
			flarm_slot,
	  		GPS_INFO->FLARM_Traffic[flarm_slot].ID,
			GPS_INFO->FLARM_Traffic[flarm_slot].Name,
			GPS_INFO->FLARM_Traffic[flarm_slot].Cn);
		#endif
	} else {
		// Else we NEED to set a name, otherwise it will constantly search for it over and over..
		name[0]=_T('?');
		name[1]=_T('\0');
		GPS_INFO->FLARM_Traffic[flarm_slot].Cn[0]=_T('?');
		GPS_INFO->FLARM_Traffic[flarm_slot].Cn[1]=_T('\0');
		
		#ifdef DEBUG_LKT
		StartupStore(_T("... New FlarmSlot=%d ID=%lx with no name, assigned a \"?\"\n"),
			flarm_slot,
	  		GPS_INFO->FLARM_Traffic[flarm_slot].ID);
		#endif
	}
  }

  #ifdef DEBUG_LKT
  StartupStore(_T("... PFLAA GPS_INFO slot=%d ID=%lx name=<%s> cn=<%s> rAlt=%.0f Track=%.0f Speed=%.0f Climb=%.1f Baro=%f FlAlt=%f\n"),
	flarm_slot,
	GPS_INFO->FLARM_Traffic[flarm_slot].ID,
	GPS_INFO->FLARM_Traffic[flarm_slot].Name,
	GPS_INFO->FLARM_Traffic[flarm_slot].Cn,
	GPS_INFO->FLARM_Traffic[flarm_slot].RelativeAltitude,
	GPS_INFO->FLARM_Traffic[flarm_slot].TrackBearing,
	GPS_INFO->FLARM_Traffic[flarm_slot].Speed,
	GPS_INFO->FLARM_Traffic[flarm_slot].ClimbRate,
	GPS_INFO->BaroAltitude,
	GPS_INFO->FLARM_Traffic[flarm_slot].Altitude);
  #endif

  //  update Virtual Waypoint for target FLARM
  if (flarm_slot == LKTargetIndex) {
	WayPointList[RESWP_FLARMTARGET].Latitude   = GPS_INFO->FLARM_Traffic[LKTargetIndex].Latitude;
	WayPointList[RESWP_FLARMTARGET].Longitude  = GPS_INFO->FLARM_Traffic[LKTargetIndex].Longitude;
	WayPointList[RESWP_FLARMTARGET].Altitude   = GPS_INFO->FLARM_Traffic[LKTargetIndex].Altitude;
  }


  return FALSE;
}



// Warn about an old locked zombie back visible
void CheckBackTarget(int flarmslot) {
  if ( !GPS_INFO.FLARM_Traffic[flarmslot].Locked ) return;
  if ( GPS_INFO.FLARM_Traffic[flarmslot].Status != LKT_ZOMBIE ) return;

  // if more than 15 minutes ago, warn pilot with full message and sound
  if ( (GPS_INFO.Time - GPS_INFO.FLARM_Traffic[flarmslot].Time_Fix) >=900) {
	// LKTOKEN  _@M674_ = "TARGET BACK VISIBLE" 
	DoStatusMessage(gettext(TEXT("_@M674_")));
	#ifndef DISABLEAUDIO
	if (EnableSoundModes) LKSound(_T("TARGVISIBLE.WAV"));
	#endif
  } else {
	// otherwise a simple sound
	#ifndef DISABLEAUDIO
	if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_DRIP"));
	#endif
  }
}


