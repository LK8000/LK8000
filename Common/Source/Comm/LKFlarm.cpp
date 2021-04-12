/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"
#include "FlarmRadar.h"
#include "Sound/Sound.h"
#include "FlarmCalculations.h"
#include "NavFunctions.h"

FlarmCalculations flarmCalculations;


void CheckBackTarget(NMEA_INFO *pGPS, int flarmslot);

//#define DEBUG_LKT	1



void FLARM_RefreshSlots(NMEA_INFO *pGPS) {
  int i;
  double passed;
//  if (pGPS->FLARM_Available)
  {
	static unsigned short iTraceSpaceCnt = 0;
	if(iTraceSpaceCnt++ > GC_TRACE_TIME_SKIP) iTraceSpaceCnt =0;
	#ifdef DEBUG_LKT
	StartupStore(_T("... [CALC thread] RefreshSlots\n"));
	#endif

	for (i=0; i<FLARM_MAX_TRAFFIC; i++) {
		if (pGPS->FLARM_Traffic[i].ID>0) {

			if ( pGPS->Time< pGPS->FLARM_Traffic[i].Time_Fix) {
				// time gone back to to Replay mode?
				#ifdef DEBUG_LKT
				StartupStore(_T("...... Refresh Back in time! Removing:%s"),NEWLINE);
				FLARM_DumpSlot(pGPS,i);
				#endif
				if (pGPS->FLARM_Traffic[i].Locked) {
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
				FLARM_EmptySlot(pGPS,i);
				continue;
			}

			passed= pGPS->Time-pGPS->FLARM_Traffic[i].Time_Fix;

			// if time has passed > zombie, then we remove it
			if (passed > LKTime_Zombie) {
				if (pGPS->FLARM_Traffic[i].Locked) {
					#ifdef DEBUG_LKT
					StartupStore(_T("...... Zombie overtime index=%d is LOCKED, no remove\n"),i);
					#endif
					continue;
				}
				#ifdef DEBUG_LKT
				StartupStore(_T("... Refresh Removing old zombie (passed=%f Fix=%f Now=%f):%s"),
					passed,
					pGPS->FLARM_Traffic[i].Time_Fix,
					pGPS->Time,
					NEWLINE);
				FLARM_DumpSlot(pGPS,i);
				#endif
				FLARM_EmptySlot(pGPS,i);
				continue;
			}

			// if time has passed > ghost, then it is a zombie
			// Ghosts are not visible on map and radar, only in infopages
			if (passed > LKTime_Ghost) {
				if (pGPS->FLARM_Traffic[i].Status == LKT_ZOMBIE) continue;
				#ifdef DEBUG_LKT
				StartupStore(_T("... Refresh Change to zombie:%s"),NEWLINE);
				FLARM_DumpSlot(pGPS,i);
				#endif
				pGPS->FLARM_Traffic[i].Status = LKT_ZOMBIE;
				continue;
			}

			// if time has passed > real, than it is a ghost
			// Shadows are shown on map as reals.
			if (passed > LKTime_Real) {
				if (pGPS->FLARM_Traffic[i].Status == LKT_GHOST) continue;
				#ifdef DEBUG_LKT
				StartupStore(_T("... Refresh Change to ghost:%s"),NEWLINE);
				FLARM_DumpSlot(pGPS,i);
				#endif
				pGPS->FLARM_Traffic[i].Status = LKT_GHOST;
				continue;
			}

			// Then it is real traffic
			pGPS->FLARM_Traffic[i].Status = LKT_REAL; // 100325 BUGFIX missing

			if(iTraceSpaceCnt == 0) {

				LKASSERT(pGPS->FLARMTRACE_iLastPtr>=0 && pGPS->FLARMTRACE_iLastPtr<MAX_FLARM_TRACES);
				pGPS->FLARM_RingBuf[pGPS->FLARMTRACE_iLastPtr].fLat = pGPS->FLARM_Traffic[i].Latitude;
				pGPS->FLARM_RingBuf[pGPS->FLARMTRACE_iLastPtr].fLon = pGPS->FLARM_Traffic[i].Longitude;

				double Vario = pGPS->FLARM_Traffic[i].Average30s;
				int iColorIdx = (int)(2*Vario  -0.5)+NO_VARIO_COLORS/2;
				iColorIdx = max( iColorIdx, 0);
				iColorIdx = min( iColorIdx, NO_VARIO_COLORS-1);

				pGPS->FLARM_RingBuf[pGPS->FLARMTRACE_iLastPtr].iColorIdx = iColorIdx;
				pGPS->FLARMTRACE_iLastPtr++;

				if(pGPS->FLARMTRACE_iLastPtr >= MAX_FLARM_TRACES) {
					pGPS->FLARMTRACE_iLastPtr=0;
					pGPS->FLARMTRACE_bBuffFull = true;
				}
			}
		} // ID >0
	} // for all traffic

#ifdef OWN_FLARM_TRACES
	double Vario=0 ;

	if(iTraceSpaceCnt == 0) {

		LKASSERT(pGPS->FLARMTRACE_iLastPtr>=0 && pGPS->FLARMTRACE_iLastPtr<MAX_FLARM_TRACES);
		pGPS->FLARM_RingBuf[pGPS->FLARMTRACE_iLastPtr].fLat = pGPS->Latitude;
		pGPS->FLARM_RingBuf[pGPS->FLARMTRACE_iLastPtr].fLon = pGPS->Longitude;

        if(pGPS->NettoVarioAvailable)
		  Vario = pGPS->NettoVario;
        else
        {
          if(pGPS->VarioAvailable)
    		Vario = pGPS->Vario;
          else
          {
        	Vario =  CALCULATED_INFO.Vario;
          }
        }
		int iColorIdx = (int)(2*Vario  -0.5)+NO_VARIO_COLORS/2;
		iColorIdx = max( iColorIdx, 0);
		iColorIdx = min( iColorIdx, NO_VARIO_COLORS-1);

		pGPS->FLARM_RingBuf[pGPS->FLARMTRACE_iLastPtr].iColorIdx = iColorIdx;
		pGPS->FLARMTRACE_iLastPtr++;

		if(pGPS->FLARMTRACE_iLastPtr >= MAX_FLARM_TRACES) {
			pGPS->FLARMTRACE_iLastPtr=0;
			pGPS->FLARMTRACE_bBuffFull = true;
		}
	}
#endif
  } // if flarm available
}


// Reset a flarm slot
void FLARM_EmptySlot(NMEA_INFO *pGPS,int i) {

#ifdef DEBUG_LKT
  StartupStore(_T("... --- EmptySlot %d : ID=%0x <%s> Cn=<%s>\n"),  i, 
	pGPS->FLARM_Traffic[i].ID, 
	pGPS->FLARM_Traffic[i].Name,
	pGPS->FLARM_Traffic[i].Cn);
#endif

  if (i<0 || i>=FLARM_MAX_TRAFFIC) return;
  pGPS->FLARM_Traffic[i].ID= 0;
  pGPS->FLARM_Traffic[i].Name[0] = 0;
  pGPS->FLARM_Traffic[i].Cn[0] = 0;
  pGPS->FLARM_Traffic[i].Speed=0;
  pGPS->FLARM_Traffic[i].Altitude=0;
  pGPS->FLARM_Traffic[i].Status = LKT_EMPTY;
  pGPS->FLARM_Traffic[i].AlarmLevel=0;
  pGPS->FLARM_Traffic[i].IDType=0;
  pGPS->FLARM_Traffic[i].TrackBearing=0;
  pGPS->FLARM_Traffic[i].TurnRate=0;
  pGPS->FLARM_Traffic[i].ClimbRate=0;
  pGPS->FLARM_Traffic[i].Type=0;
  pGPS->FLARM_Traffic[i].Time_Fix=0;
  pGPS->FLARM_Traffic[i].Average30s=0;
  pGPS->FLARM_Traffic[i].Locked = false;
  pGPS->FLARM_Traffic[i].UpdateNameFlag = false;

}


#ifdef DEBUG_LKT
void FLARM_DumpSlot(NMEA_INFO *pGPS,int i) {
  TCHAR dump[256];
  _stprintf(dump, _T("... DumpSlot (%d) status=%d id=<%lx> Name=<%s> Cn=<%s> Speed=%.0f rAlt=%.0f  %s"),
	i,
	pGPS->FLARM_Traffic[i].Status,
	pGPS->FLARM_Traffic[i].ID,
	pGPS->FLARM_Traffic[i].Name,
	pGPS->FLARM_Traffic[i].Cn,
	pGPS->FLARM_Traffic[i].Speed,
	pGPS->FLARM_Traffic[i].Altitude, 
	NEWLINE);
  StartupStore(dump);
}
#endif	


#include "InputEvents.h"

double FLARM_NorthingToLatitude = 0.0;
double FLARM_EastingToLongitude = 0.0;

BOOL NMEAParser::PFLAV(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *pGPS)
{
TCHAR Temp[80];

//$PFLAV,A,3.00,5.04,alps20110919_*5C
//StartupStore(_T("FLARM PFLAV received: %s"), String);
//ParToDouble(String, 1, &pGPS->FLARM_HW_Version);
//ParToDouble(String, 2, &pGPS->FLARM_SW_Version);

ExtractParameter(String, Temp,1); _stscanf(Temp,TEXT("%lf"), &pGPS->FLARM_HW_Version);
ExtractParameter(String, Temp,2); _stscanf(Temp,TEXT("%lf"), &pGPS->FLARM_SW_Version);
ExtractParameter(String, Temp,3);
StartupStore(_T("FLARM  found SW:%4.2f  HW:%4.2f  OBS:%s%s"),pGPS->FLARM_SW_Version,pGPS->FLARM_HW_Version,Temp, NEWLINE);

return true;
}


BOOL NMEAParser::PFLAU(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *pGPS)
{
  static int old_flarm_rx = 0;
  static bool sayflarmavailable=true; // 100325
  static bool conflict=false;

  // It can happen that both port auto/exclude themselves, or one will succeed to survive.
  // In either cases, there is a bad problem going on. Recovery should not be a choice.
  if (conflict) return FALSE;

  //
  // We want to be sure that we are not going to elect as Flarm two simultaneous ports.
  // We let it happen once, and give warning. Then only one of the two will remain.
  // It is a real borderline situation, due to conflict on comm ports, normally virtual com ports.
  unsigned flarm_count = 0;
  for(const auto& dev : DeviceList) {
    if(dev.nmeaParser.gpsValid && dev.nmeaParser.isFlarm) {
      ++flarm_count;
    }
  }
  if (flarm_count > 1) {
    DoStatusMessage(MsgToken(1529));
    StartupStore(_T("......... WARNING! FLARM DETECTED ON TWO COM PORTS! %s\n"), WhatTimeIsIt());
    pGPS->FLARM_Available = false;
    isFlarm = false;
    conflict=true;
    return FALSE;
  }

  if(!pGPS->FLARM_Available)
	sayflarmavailable = true;

  pGPS->FLARM_Available = true;
  LastFlarmCommandTime = pGPS->Time;
  isFlarm = true;

  if ( sayflarmavailable ) {
	pGPS->FLARM_SW_Version =0.0;
	pGPS->FLARM_HW_Version =0.0;
	static int MessageCnt =0;
	if(MessageCnt < 10)
	{
	  MessageCnt++;
	  DoStatusMessage(MsgToken(279)); // FLARM DETECTED
	}
	sayflarmavailable=false;
#if FLARMDEADLOCK
  for(const auto& dev : DeviceList) {
    if(dev.nmeaParser.isFlarm) {
      devRequestFlarmVersion(&dev);
      break; // we have got first available Flarm device, ingore next device.
    }
  }
#endif
  }


  // calculate relative east and north projection to lat/lon

  double delta_lat = 0.01;
  double delta_lon = 0.01;

  double dlat;
  DistanceBearing(pGPS->Latitude, pGPS->Longitude,
                  pGPS->Latitude+delta_lat, pGPS->Longitude,
                  &dlat, NULL);
  double dlon;
  DistanceBearing(pGPS->Latitude, pGPS->Longitude,
                  pGPS->Latitude, pGPS->Longitude+delta_lon,
                  &dlon, NULL);

  if ((fabs(dlat)>0.0)&&(fabs(dlon)>0.0)) {
    FLARM_NorthingToLatitude = delta_lat / dlat;
    FLARM_EastingToLongitude = delta_lon / dlon;
  } else {
    FLARM_NorthingToLatitude=0.0;
    FLARM_EastingToLongitude=0.0;
  }

  unsigned short power;
  _stscanf(String,
	  TEXT("%hu,%hu,%hu,%hu,%hu"),
	  &pGPS->FLARM_RX, // number of received FLARM devices
	  &pGPS->FLARM_TX, // Transmit status
	  &pGPS->FLARM_GPS, // GPS status
	  &power, // <Power> : unused
	  &pGPS->FLARM_AlarmLevel); // Alarm level of FLARM (0-3)

  // process flarm updates

  if ((pGPS->FLARM_RX) && (old_flarm_rx==0)) {
    // traffic has appeared..
    InputEvents::processGlideComputer(GCE_FLARM_TRAFFIC);
  }
  if (pGPS->FLARM_RX > old_flarm_rx) {
    // re-set suppression of gauge, as new traffic has arrived
    //    GaugeFLARM::Suppress = false;
  }
  if ((pGPS->FLARM_RX==0) && (old_flarm_rx)) {
    // traffic has disappeared..
    InputEvents::processGlideComputer(GCE_FLARM_NOTRAFFIC);
  }
  // TODO feature: add another event for new traffic.

  old_flarm_rx = pGPS->FLARM_RX;

  return FALSE;
}



int FLARM_FindSlot(NMEA_INFO *pGPS, long Id)
{
  int i;
  for (i=0; i<FLARM_MAX_TRAFFIC; i++) {

	// find position in existing slot
	if (Id==pGPS->FLARM_Traffic[i].ID) {
		//#ifdef DEBUG_LKT
		//StartupStore(_T("... FindSlot ID=%lx found in slot %d\n"),Id,i);
		//#endif
		return i;
	}
	// find old empty slot
  }
  // not found, so try to find an empty slot
  for (i=0; i<FLARM_MAX_TRAFFIC; i++) {
	if (pGPS->FLARM_Traffic[i].ID<=0) { // 100327 <= was ==
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
	if ( (pGPS->FLARM_Traffic[i].ID>0) && (pGPS->FLARM_Traffic[i].Status==LKT_ZOMBIE) &&
		(!pGPS->FLARM_Traffic[i].Locked) ) { 
		// if this is the first zombie, assign it and continue searching
		if (toremove==-1) {
			toremove=i;
		} else {
			// if this zombie is older than previous one
			if ( pGPS->FLARM_Traffic[i].Time_Fix < pGPS->FLARM_Traffic[toremove].Time_Fix ) {
				toremove=i;
			}
		}
	}
  }
  // did we find a zombie to remove?
  if (toremove>=0) {
	#ifdef DEBUG_LKT
	StartupStore(_T("... Removing OLDEST zombie:%s"),NEWLINE);
	FLARM_DumpSlot(pGPS,toremove);
	#endif
	FLARM_EmptySlot(pGPS,toremove);
	return toremove;
  }
  // remove a ghost to make place
  toremove=-1;
  for (i=0; i<FLARM_MAX_TRAFFIC; i++) {
	if ( (pGPS->FLARM_Traffic[i].ID>0) && (pGPS->FLARM_Traffic[i].Status==LKT_GHOST) &&
		(!pGPS->FLARM_Traffic[i].Locked) ) { 
		// if this is the first ghost, assign it and continue searching
		if (toremove==-1) {
			toremove=i;
		} else {
			// if this ghost is older than previous one
			if ( pGPS->FLARM_Traffic[i].Time_Fix < pGPS->FLARM_Traffic[toremove].Time_Fix ) {
				toremove=i;
			}
		}
	}
  }
  // did we find a ghost to remove?
  if (toremove>=0) {
	#ifdef DEBUG_LKT
	StartupStore(_T("... Removing OLDEST ghost:%s"),NEWLINE);
	FLARM_DumpSlot(pGPS,toremove);
	#endif
	FLARM_EmptySlot(pGPS,toremove);
	return toremove;
  }

  #ifdef DEBUG_LKT
  StartupStore(_T("... ID=<%lx> NO SPACE in slots!\n"),Id);
  #endif
  // still not found and no empty slots left, buffer is full
  return -1;
}




BOOL NMEAParser::PFLAA(TCHAR *String, TCHAR **params, size_t nparams, NMEA_INFO *pGPS)
{
  if(nparams < 6) {
    TESTBENCH_DO_ONLY(10,StartupStore(_T(". NMEAParser invalid PFLAA sentence, nparams=%u%s"),(unsigned)nparams,NEWLINE));
    // max index used is 5...
    return FALSE;
  }
  
  int flarm_slot = 0;

  isFlarm = true;

  // 5 id, 6 digit hex
  long ID;
  _stscanf(params[5],TEXT("%lx"), &ID);
//  unsigned long uID = ID;

  flarm_slot = FLARM_FindSlot(pGPS, ID);
  if (flarm_slot<0) {
    // no more slots available,
	#ifdef DEBUG_LKT
	StartupStore(_T("... NO SLOTS for Flarm traffic, too many ids!%s"),NEWLINE);
	#endif
	return FALSE;
  }

  // before changing timefix, see if it was an old target back locked in!
  CheckBackTarget(pGPS, flarm_slot);
  // and then set time of fix to current time
  pGPS->FLARM_Traffic[flarm_slot].Time_Fix = pGPS->Time;

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

  double RelativeNorth, RelativeEast, RelativeAltitude;

  _stscanf(nString,
	  TEXT("%hu,%lf,%lf,%lf,%hu,%x,%lf,%lf,%lf,%lf,%hu"),
	  &pGPS->FLARM_Traffic[flarm_slot].AlarmLevel, // unsigned short 0
	  &RelativeNorth, //  1
	  &RelativeEast, //   2
	  &RelativeAltitude, //  3
	  &pGPS->FLARM_Traffic[flarm_slot].IDType, // unsigned short     4
	  &pGPS->FLARM_Traffic[flarm_slot].ID, // 6 char hex
	  &pGPS->FLARM_Traffic[flarm_slot].TrackBearing, // double       6
	  &pGPS->FLARM_Traffic[flarm_slot].TurnRate, // double           7
	  &pGPS->FLARM_Traffic[flarm_slot].Speed, // double              8 m/s
	  &pGPS->FLARM_Traffic[flarm_slot].ClimbRate, // double          9 m/s
	  &pGPS->FLARM_Traffic[flarm_slot].Type); // unsigned short     10
  // 1 relativenorth, meters  
  pGPS->FLARM_Traffic[flarm_slot].Latitude = RelativeNorth *FLARM_NorthingToLatitude + pGPS->Latitude;
  // 2 relativeeast, meters
  pGPS->FLARM_Traffic[flarm_slot].Longitude = RelativeEast *FLARM_EastingToLongitude + pGPS->Longitude;

  // we need to compare with BARO altitude FLARM relative Alt difference!
  if (pGPS->BaroAltitude>0) // just to be sure
	pGPS->FLARM_Traffic[flarm_slot].Altitude = RelativeAltitude + pGPS->BaroAltitude;
  else
	pGPS->FLARM_Traffic[flarm_slot].Altitude = RelativeAltitude + pGPS->Altitude;



  pGPS->FLARM_Traffic[flarm_slot].Average30s = flarmCalculations.Average30s(
	  pGPS->FLARM_Traffic[flarm_slot].ID,
	  pGPS->Time,
	  pGPS->FLARM_Traffic[flarm_slot].Altitude);

  TCHAR *name = pGPS->FLARM_Traffic[flarm_slot].Name;
  //TCHAR *cn = pGPS->FLARM_Traffic[flarm_slot].Cn;
  // If there is no name yet, or if we have a pending update event..
  if (!_tcslen(name) || pGPS->FLARM_Traffic[flarm_slot].UpdateNameFlag ) {

	#ifdef DEBUG_LKT
	if (pGPS->FLARM_Traffic[flarm_slot].UpdateNameFlag ) {
		StartupStore(_T("... UpdateNameFlag for slot %d\n"),flarm_slot);
	} else {
		StartupStore(_T("... First lookup name for slot %d\n"),flarm_slot);
	}
	#endif

	pGPS->FLARM_Traffic[flarm_slot].UpdateNameFlag=false; // clear flag first
	TCHAR *fname = LookupFLARMDetails(pGPS->FLARM_Traffic[flarm_slot].ID);
	if (fname) {
		LK_tcsncpy(name,fname,MAXFLARMNAME);

		//  Now we have the name, so lookup also for the Cn
		// This will return either real Cn or Name, again
		TCHAR *cname = LookupFLARMCn(pGPS->FLARM_Traffic[flarm_slot].ID);
		if (cname) {
			int cnamelen=_tcslen(cname);
			if (cnamelen<=MAXFLARMCN) {
				_tcscpy( pGPS->FLARM_Traffic[flarm_slot].Cn, cname);
			} else {
				// else probably it is the Name again, and we create a fake Cn
				pGPS->FLARM_Traffic[flarm_slot].Cn[0]=cname[0];
				pGPS->FLARM_Traffic[flarm_slot].Cn[1]=cname[cnamelen-2];
				pGPS->FLARM_Traffic[flarm_slot].Cn[2]=cname[cnamelen-1];
				pGPS->FLARM_Traffic[flarm_slot].Cn[3]=_T('\0');
			}
		} else {
			_tcscpy( pGPS->FLARM_Traffic[flarm_slot].Cn, _T("Err"));
		}

		#ifdef DEBUG_LKT
		StartupStore(_T("... PFLAA Name to FlarmSlot=%d ID=%lx Name=<%s> Cn=<%s>\n"),
			flarm_slot,
	  		pGPS->FLARM_Traffic[flarm_slot].ID,
			pGPS->FLARM_Traffic[flarm_slot].Name,
			pGPS->FLARM_Traffic[flarm_slot].Cn);
		#endif
	} else {
		// Else we NEED to set a name, otherwise it will constantly search for it over and over..
		name[0]=_T('?');
		name[1]=_T('\0');
		pGPS->FLARM_Traffic[flarm_slot].Cn[0]=_T('?');
		pGPS->FLARM_Traffic[flarm_slot].Cn[1]=_T('\0');
		
		#ifdef DEBUG_LKT
		StartupStore(_T("... New FlarmSlot=%d ID=%lx with no name, assigned a \"?\"\n"),
			flarm_slot,
	  		pGPS->FLARM_Traffic[flarm_slot].ID);
		#endif
	}
  }

  #ifdef DEBUG_LKT
  StartupStore(_T("... PFLAA pGPS slot=%d ID=%lx name=<%s> cn=<%s> rAlt=%.0f Track=%.0f Speed=%.0f Climb=%.1f Baro=%f FlAlt=%f\n"),
	flarm_slot,
	pGPS->FLARM_Traffic[flarm_slot].ID,
	pGPS->FLARM_Traffic[flarm_slot].Name,
	pGPS->FLARM_Traffic[flarm_slot].Cn,
	pGPS->FLARM_Traffic[flarm_slot].RelativeAltitude,
	pGPS->FLARM_Traffic[flarm_slot].TrackBearing,
	pGPS->FLARM_Traffic[flarm_slot].Speed,
	pGPS->FLARM_Traffic[flarm_slot].ClimbRate,
	pGPS->BaroAltitude,
	pGPS->FLARM_Traffic[flarm_slot].Altitude);
  #endif

  //  update Virtual Waypoint for target FLARM
  if (flarm_slot == LKTargetIndex) {
	WayPointList[RESWP_FLARMTARGET].Latitude   = pGPS->FLARM_Traffic[LKTargetIndex].Latitude;
	WayPointList[RESWP_FLARMTARGET].Longitude  = pGPS->FLARM_Traffic[LKTargetIndex].Longitude;
	WayPointList[RESWP_FLARMTARGET].Altitude   = pGPS->FLARM_Traffic[LKTargetIndex].Altitude;
  }


  return FALSE;
}



// Warn about an old locked zombie back visible
// Attention, do not use sounds from Rx Thread.. deadlocks pending. So NO ext sounds here.
void CheckBackTarget(NMEA_INFO *pGPS, int flarmslot) {
  if ( pGPS->FLARM_Traffic[flarmslot].Locked ) return;
  if ( pGPS->FLARM_Traffic[flarmslot].Status != LKT_ZOMBIE ) return;

  // if more than 15 minutes ago, warn pilot with full message and sound
  if ( (pGPS->Time - pGPS->FLARM_Traffic[flarmslot].Time_Fix) >=900) {
	// LKTOKEN  _@M674_ = "TARGET BACK VISIBLE" 
	DoStatusMessage(MsgToken(674));
        if (!UseExtSound1 && !UseExtSound2) LKSound(_T("TARGVISIBLE.WAV"));
  } else {
	// otherwise a simple sound
	if (!UseExtSound1 && !UseExtSound2) PlayResource(TEXT("IDR_WAV_DRIP"));
  }
}


