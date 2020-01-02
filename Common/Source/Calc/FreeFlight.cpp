/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: FreeFlight.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "Logger.h"
#include "LKProcess.h"
#include "DoInits.h"
#include "Sound/Sound.h"
#include "NavFunctions.h"
#include "Waypointparser.h"
#include "Library/TimeFunctions.h"

/* 
 * Detect start of free flight (FF) for both towing and winching.
 * Start of FF may be marked 3 seconds later, max. Small error.
 * This combined approach is much more accurate than seeyou!
 * WARNING> the internal IGC interpolator is buggy and will cause problems
 * while replaying IGC file logged at more than 2s interval.
 * During testing, all 1s interval logs (which are equivalent to real flight)
 * worked fine, while we got false positives using >2s interval logs.
 * We dont care much, since we need real flight FF detection, and LK
 * is logging always at 1s. So this might actually happen only replaying 
 * an external log file on LK. 
 * 
 * FREEFLIGHT DETECTION IS USING GPS ALTITUDE ONLY
 *
 * Called by Calculations thread, each second.
 *
 * No message will be given to the user concerning start of free flight.
 * We should not disturb pilots during critical phases of flight.
 */
#define FF_TOWING_ALTLOSS	25		// meters loss for towing
#define FF_WINCHIN_ALTLOSS	10		// meters loss for winching, careful about GPS H errors.
#define FF_MAXTOWTIME	1200			// 20 minutes
#define MAX_NO_GEAR_WARN 10   			// max. numbers of Gerwarnings before disabled



bool DetectFreeFlying(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  static bool   ffDetected=false;
  static int    lastMaxAltitude=-1000;
  static double gndAltitude=0;
  static double vario[8];
  static bool   winchdetected=false;
  static short  wlaunch=0;
  static int    altLoss=0;
  static bool   safeTakeoffDetected=false;
  static bool   nowGearWarning = true; // init with true to prevent gear warning just after free flight detect
  static unsigned short noMessages =0;

  bool forcereset=LKSW_ForceFreeFlightRestart;

  if (DoInit[MDI_DETECTFREEFLYING]) {
    for (int i=0; i<8; i++) vario[i]=0;
    gndAltitude=0;
    winchdetected=false;
    wlaunch=0;
    altLoss=0;
    ffDetected=false;
    lastMaxAltitude=-1000;
    safeTakeoffDetected=false;
    nowGearWarning=true; // we are here before freeflight!
    noMessages=0;	// after a new takeoff we can give warnings again!
    DoInit[MDI_DETECTFREEFLYING]=false;
  }

  // reset on ground
  if (Calculated->Flying == false) {
    Calculated->FreeFlying=false;
    ffDetected=false;
    lastMaxAltitude=-1000;
    // For winch launches and also for quick taekoffs do not update gndAltitude when the plane
    // is already moving, probably towed or winched already. Threshold is at 4m/s, = 14kmh
    if (Basic->Speed<=4.0) gndAltitude=Basic->Altitude;
    winchdetected=false;
    wlaunch=0;
    altLoss=FF_TOWING_ALTLOSS;
    safeTakeoffDetected=false;
    LKSW_ForceFreeFlightRestart=false;
    return false;
  }

  if (forcereset) {
	LKSW_ForceFreeFlightRestart=false;
	#if TESTBENCH
	StartupStore(_T("... Forced FreeFlight Restart!\n"));
	#endif
	DoStatusMessage(MsgToken<1452>(),NULL,false);  // LKTOKEN  _@M1452_ = "Free flight detected"
	goto confirmbacktrue;
  }

  //  If we have a takeoff alarm, and  it is still pending!
  //  The AlarmTakeoffSafety is saved multiplied by 1000, so conversion between feet and meters will always be
  //  accurate and possible with no safety concerns about loosing accuracy on this issue!
  if ( (AlarmTakeoffSafety>0) && !safeTakeoffDetected ) {
	// Only if not in SIMMODE, or in SIM but replaying a flight
	if ( !(SIMMODE && !ReplayLogger::IsEnabled()) ) {
		if ( (Basic->Altitude - gndAltitude)>=(AlarmTakeoffSafety/1000)) {
			LKSound(_T("LK_SAFETAKEOFF.WAV"));
			safeTakeoffDetected=true;
		}
	}
  }

  if ( (GearWarningMode>0) && ffDetected) {
      double AltitudeAGL = Calculated->AltitudeAGL;
      double dist = 0;

      if(GearWarningMode ==1) {
          if (ValidWayPoint(BestAlternate)) {
	      AltitudeAGL = Basic->Altitude - WayPointList[BestAlternate].Altitude; // AGL = height above landable

	      if( AltitudeAGL <= (GearWarningAltitude/1000)) {
	          DistanceBearing(Basic->Latitude, Basic->Longitude, WayPointList[BestAlternate].Latitude, 
                     WayPointList[BestAlternate].Longitude, &dist, NULL);
              }

	  } else {
	      dist = 9999; // set to far away if best alternate  not valid
	  }

      }

      if (( AltitudeAGL  <=(GearWarningAltitude/1000)) && (noMessages < MAX_NO_GEAR_WARN)) {
          if(!nowGearWarning) {
              if(dist < 3700) { // show gear warning if 2Nautical Miles close to landable
	          LKSound(_T("LK_GEARWARNING.WAV"));
		  DoStatusMessage(MsgToken<1834>(),NULL,false);  // LKTOKEN _@M1834_ "Prepare for landing !"
		  nowGearWarning=true;
		  noMessages++;

#if TESTBENCH
		  if(GearWarningMode ==2)
                      StartupStore(_T("... %i. Gear warning at %im = %im [%im] AGL%s"),
                         noMessages,(int)Basic->Altitude,(int)AltitudeAGL,(int)GearWarningAltitude/1000,NEWLINE);
		  else {
                      if (ValidWayPoint(BestAlternate)) 
	              StartupStore(_T("...%i. Gear warning at %im = %im [%im] over landable %s (%im)%s"),
                         noMessages,(int)Basic->Altitude,(int)AltitudeAGL,(int)GearWarningAltitude/1000,
                         WayPointList[BestAlternate].Name,(int)WayPointList[BestAlternate].Altitude,NEWLINE);
                  }
#endif

	      }

              if (noMessages==MAX_NO_GEAR_WARN) {
                  StartupStore(_T("... GOING SILENT on too many Gear warnings.  %s%s"),WhatTimeIsIt(),NEWLINE);

	          DoStatusMessage(MsgToken<2304>()); // GOING SILENT ON GEAR REPORTING
	          noMessages++;	// we go to 11, and never be back here
	      }
          }
      } else {
          // re-enable warning if higher that 100m above Gear altitude
          if(( AltitudeAGL)> ((GearWarningAltitude/1000)+100)) { 
              if( nowGearWarning ) {
#if TESTBENCH
                  if(GearWarningMode ==2)
		      StartupStore(_T("...rearmed %i. Gear warning at %im = %im AGL %s"),
                          noMessages,(int)Basic->Altitude,(int)AltitudeAGL,NEWLINE);
                  else
		      if (ValidWayPoint(BestAlternate)) {
		          StartupStore(_T("..rearmed %i. Gear warning at %im = %im over landable %s (%im)%s"),
                            noMessages,(int)Basic->Altitude,(int)AltitudeAGL,WayPointList[BestAlternate].Name,
                            (int)WayPointList[BestAlternate].Altitude,NEWLINE);
                      }
#endif
	  	  nowGearWarning = false;
              }
	  }
      }
  }

  if (ISPARAGLIDER) {
    Calculated->FreeFlying=true;
    return true; 
  }
  if (ISGAAIRCRAFT||ISCAR) {
    return false; 
  }

  // do something special for other situations
  if (SIMMODE && !ReplayLogger::IsEnabled()) {
    Calculated->FreeFlying=true;
    ffDetected=true;
    return true; 
  }

  // If flying, and start was already detected, assume valid freeflight.
  // Put here in the future the Engine Noise Detection for motorplanes
  if (ffDetected)
    return true;

  // Here we are flying, and the start of free flight must still be detected

  // In any case, after this time, force a start. This is to avoid that for any reason, no FF is ever detected.
  if ((int)Basic->Time > ( Calculated->TakeOffTime + FF_MAXTOWTIME)) {
    #if DEBUG_DFF
    DoStatusMessage(_T("DFF:  TIMEOUT"));
    #endif
    goto backtrue;	// unconditionally force start FF
  }

 
  // A loss of altitude will trigger FF 
  lastMaxAltitude = std::max(lastMaxAltitude, (int)Basic->Altitude);
  if ((int)Basic->Altitude <= ( lastMaxAltitude - altLoss)) {
    #if DEBUG_DFF
    if ( (winchdetected) || ((Basic->Altitude - gndAltitude)>=400) 
      && ((Basic->Time - Calculated->TakeOffTime) >= 150))
      DoStatusMessage(_T("DFF:  ALTITUDE LOSS"));
    #endif
    goto lastcheck;
  }

#ifdef TOW_CRUISE
  // If circling, we assume that we're in free flight and use the
  // start-of-circling time and position.  Turning.cpp makes sure
  // we aren't circling while on tow by imposing a 12 deg/sec turn
  // rate limit for on-tow turning.
#else
  // If circling we assume that we are in free flight already, using the start of circling time and position.
  // But we must be sure that we are not circling.. while towed. A 12 deg/sec turn rate will make it quite sure.
  // Of course nobody can circle while winchlaunched, so in this case FF is immediately detected.
#endif

#ifdef TOW_CRUISE
  // The 12 deg/sec check is now done in Turning.cpp, so there's no
  // need to do it here, too.  Doing it here, too, could allow the
  // possibility of climb mode engaging without FF detection, if the
  // climb rate goes above 12 deg/sec for just a split second.
  //if (Calculated->Circling && (winchdetected ||
  //   (fabs(Calculated->TurnRate) >= 12))) {

  if (Calculated->Circling) {
#else
  if (Calculated->Circling && ( winchdetected || ( fabs(Calculated->TurnRate) >=12 ) ) ) {
#endif

    if (UseContestEngine()) {
      CContestMgr::Instance().Add(static_cast<unsigned>(Calculated->ClimbStartTime),
                                  Calculated->ClimbStartLat, Calculated->ClimbStartLong,
                                  static_cast<unsigned>(Calculated->ClimbStartAlt));
    }

    #if DEBUG_DFF
    DoStatusMessage(_T("DFF:  THERMALLING"));
    #endif
    goto backtrue;
  }

  vario[7]=vario[6];
  vario[6]=vario[5];
  vario[5]=vario[4];
  vario[4]=vario[3];
  vario[3]=vario[2];
  vario[2]=vario[1];
  vario[1]=Calculated->Vario;

  double newavervario;
  double oldavervario;

  newavervario=(vario[1]+vario[2]+vario[3])/3;

  if (newavervario>=10) {
    wlaunch++;
  } else {
    wlaunch=0;
  }
  // After (6+2) 8 seconds of consecutive fast climbing (AFTER TAKEOFF DETECTED!), winch launch is detected
  if (wlaunch==6) {
    #if DEBUG_DFF
    DoStatusMessage(_T("DFF:  WINCH LAUNCH"));
    #endif
    altLoss=FF_WINCHIN_ALTLOSS;
    winchdetected=true;
  }
    
  if (newavervario>0.3)
    return false;

  if (newavervario<=0) {
    #if DEBUG_DFF
    if ( (winchdetected) || ((Basic->Altitude - gndAltitude)>=400) 
      && ((Basic->Time - Calculated->TakeOffTime) >= 150))
      DoStatusMessage(_T("DFF:  VARIO SINK"));
    #endif
    goto lastcheck;
  }

  oldavervario=(vario[4]+vario[5]+vario[6]+vario[7])/4;
  // current avervario between 0.0 and 0.3: uncertain situation,  we check what we had in the previous time
  // Windy situations during towing may lead to false positives if delta is too low. This is the most
  // difficult part which could lead to false positives or negatives.
  if ( oldavervario >=4.5 ) {
    #if DEBUG_DFF
    StartupStore(_T("..... oldavervario=%.1f newavervario=%.1f current=%.1f\n"),oldavervario,newavervario,Calculated->Vario);
    if ( (winchdetected) || ((Basic->Altitude - gndAltitude)>=400) 
      && ((Basic->Time - Calculated->TakeOffTime) >= 150))
      DoStatusMessage(_T("DFF:  DELTA VARIO"));
    #endif
    goto lastcheck;
  }

  // No free flight detected
  return false;

  lastcheck: 

  // Unless under a winch launch, we shall not consider anything below 400m gain, 
  // and before 2.5minutes have passed since takeoff
  // Anybody releasing tow before 3 minutes will be below 500m QFE, and won't go much around
  // until a thermal is found. So no problems. 

  if (!winchdetected) {
    if ( (Basic->Altitude - gndAltitude)<400)
      return false;
    if ( (Basic->Time - Calculated->TakeOffTime) < 150)
      return false;
  }

  backtrue:
  // In real flight on a fly device (assuming a windowsPC is NOT a fly device)
  // give no messages during critical phases of flight
  #if ( WINDOWSPC==0 )
  if (SIMMODE)
  #endif
  DoStatusMessage(MsgToken<1452>(),NULL,false);  // LKTOKEN  _@M1452_ = "Free flight detected"

  confirmbacktrue:
  // Always sound
  LKSound(_T("LK_FREEFLIGHT.WAV"));

  StartupStore(_T(". Free Flight started %s%s"), WhatTimeIsIt(),NEWLINE);

  ffDetected=true;
  Calculated->FreeFlying=true;
  Calculated->FreeFlightStartTime=Basic->Time;
  Calculated->FreeFlightStartQNH=Basic->Altitude;
  Calculated->FreeFlightStartQFE=gndAltitude;

  WithLock(CritSec_TaskData, [&]() {
    // all "WayPointList" change need to be protected by "LockTaskData"
        WayPointList[RESWP_FREEFLY].Latitude = Basic->Latitude;
        WayPointList[RESWP_FREEFLY].Longitude = Basic->Longitude;
        WayPointList[RESWP_FREEFLY].Altitude = Basic->Altitude;
        if (WayPointList[RESWP_FREEFLY].Altitude == 0) WayPointList[RESWP_FREEFLY].Altitude = 0.001;
        WayPointList[RESWP_FREEFLY].Reachable = TRUE;
        WayPointList[RESWP_FREEFLY].Visible = TRUE;
        WayPointList[RESWP_FREEFLY].Format = LKW_VIRTUAL;

        BUGSTOP_LKASSERT(WayPointList[RESWP_FREEFLY].Comment != NULL);

        if (WayPointList[RESWP_FREEFLY].Comment) {
            TCHAR Temp[30];
            Units::TimeToTextS(Temp, LocalTime(Calculated->FreeFlightStartTime));          
            TCHAR Comment[100];

            _sntprintf(Comment, 99, _T("%s: %s  @%.0f%s QNH"),
                    MsgToken<1754>(), // Free flight start
                    Temp,
                    ALTITUDEMODIFY * Calculated->FreeFlightStartQNH,
                    Units::GetAltitudeName());
            Comment[99] = _T('\0'); // for safety
            SetWaypointComment(WayPointList[RESWP_FREEFLY], Comment);
        }
  });

  ResetFreeFlightStats(Calculated);
  return true;

}




//
// Upon FF detection, we reset some calculations, ONLY SOME.
// Most things are pertinent to flying, not to freeflying.
//
// Notice 1:  we cannot do oldstyle reset, because it would reset also takeoff time.
//            For FF, we need new stuff doing new things.
// Notice 2:  GA and CAR mode will not ever use FF stuff
//
// Notice 3:  since FF is not (still) affecting tasks, we shall not reset task values now
//
void ResetFreeFlightStats(DERIVED_INFO *Calculated) {

  int i;

  CContestMgr::Instance().Reset(Handicap);
  flightstats.Reset();

  Calculated->timeCruising = 0;
  Calculated->timeCircling = 0;

  // Calculated->TotalHeightClimb = 0;
  // Calculated->CruiseStartTime = -1;
  // 
  // ClimbStartTime CANNOT be reset here: it is a loop! We use themal start to detect freeflight!
  // We have a conflict here!
  // Calculated->ClimbStartTime = -1;  
  // Calculated->AverageThermal = 0;

  for (i=0; i<MAXAVERAGECLIMBRATESIZE; i++) {
     Calculated->AverageClimbRate[i]= 0;
     Calculated->AverageClimbRateN[i]= 0;
  }
  Calculated->MaxThermalHeight = 0;
  for (i=0; i<NUMTHERMALBUCKETS; i++) {
    Calculated->ThermalProfileN[i]=0;
    Calculated->ThermalProfileW[i]=0;
  }

  // clear thermal sources for first time.
  for (i=0; i<MAX_THERMAL_SOURCES; i++) {
    Calculated->ThermalSources[i].LiftRate= -1.0;
  }

  // The MaxHeightGain function wait for FF in flight and will update
  // considering 0 as a no-altitude-set-yet .
  Calculated->MinAltitude = 0;
  Calculated->MaxAltitude = 0;
  Calculated->MaxHeightGain = 0;


  Calculated->Odometer = 0;

}

