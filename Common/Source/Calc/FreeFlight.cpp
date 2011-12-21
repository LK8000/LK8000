/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: FreeFlight.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "Logger.h"
#include "DoInits.h"

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
 * Called by Calculations thread, each second.
 *
 * No message will be given to the user concerning start of free flight.
 * We should not disturb pilots during critical phases of flight.
 */
#define FF_TOWING_ALTLOSS	25		// meters loss for towing
#define FF_WINCHIN_ALTLOSS	10		// meters loss for winching, careful about GPS H errors.
#define FF_MAXTOWTIME	1200			// 20 minutes


bool DetectFreeFlying(DERIVED_INFO *Calculated) {

  static bool   ffDetected=false;
  static int    lastMaxAltitude=-1000;
  static double gndAltitude=0;
  static double vario[8];
  static bool   winchdetected=false;
  static short  wlaunch=0;
  static int    altLoss=0;

  if (DoInit[MDI_DETECTFREEFLYING]) {
    for (int i=0; i<8; i++) vario[i]=0;
    gndAltitude=0;
    winchdetected=false;
    wlaunch=0;
    altLoss=0;
    ffDetected=false;
    lastMaxAltitude=-1000;
    DoInit[MDI_DETECTFREEFLYING]=false;
  }

  // reset on ground
  if (Calculated->Flying == FALSE) {
    Calculated->FreeFlying=false;
    ffDetected=false;
    lastMaxAltitude=-1000;
    gndAltitude=GPS_INFO.Altitude;
    winchdetected=false;
    wlaunch=0;
    altLoss=FF_TOWING_ALTLOSS;
    return false;
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
  if ((int)GPS_INFO.Time > ( Calculated->TakeOffTime + FF_MAXTOWTIME)) {
    #if DEBUG_DFF
    DoStatusMessage(_T("DFF:  TIMEOUT"));
    #endif
    goto backtrue;	// unconditionally force start FF
  }

 
  // A loss of altitude will trigger FF 
  lastMaxAltitude = std::max(lastMaxAltitude, (int)GPS_INFO.Altitude);
  if ((int)GPS_INFO.Altitude <= ( lastMaxAltitude - altLoss)) {
    #if DEBUG_DFF
    if ( (winchdetected) || ((GPS_INFO.Altitude - gndAltitude)>=400) 
      && ((GPS_INFO.Time - Calculated->TakeOffTime) >= 150))
      DoStatusMessage(_T("DFF:  ALTITUDE LOSS"));
    #endif
    goto lastcheck;
  }

  // If circling we assume that we are in free flight already, using the start of circling time and position.
  // But we must be sure that we are not circling.. while towed. A 12 deg/sec turn rate will make it quite sure.
  // Of course nobody can circle while winchlaunched, so in this case FF is immediately detected.
  if (Calculated->Circling && ( winchdetected || ( fabs(Calculated->TurnRate) >=12 ) ) ) {

    if (UseContestEngine()) {
      CContestMgr::Instance().Add(new CPointGPS(static_cast<unsigned>(Calculated->ClimbStartTime),
	Calculated->ClimbStartLat, Calculated->ClimbStartLong,
	static_cast<unsigned>(Calculated->ClimbStartAlt)));
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
    if ( (winchdetected) || ((GPS_INFO.Altitude - gndAltitude)>=400) 
      && ((GPS_INFO.Time - Calculated->TakeOffTime) >= 150))
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
    if ( (winchdetected) || ((GPS_INFO.Altitude - gndAltitude)>=400) 
      && ((GPS_INFO.Time - Calculated->TakeOffTime) >= 150))
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
    if ( (GPS_INFO.Altitude - gndAltitude)<400)
      return false;
    if ( (GPS_INFO.Time - Calculated->TakeOffTime) < 150)
      return false;
  }

  backtrue:
  // In real flight on a fly device (assuming a windowsPC is NOT a fly device)
  // give no messages during critical phases of flight
  #if ( !defined(WINDOWSPC) || WINDOWSPC==0 )
  if (SIMMODE)
  #endif
  DoStatusMessage(gettext(TEXT("_@M1452_")));  // LKTOKEN  _@M1452_ = "Free flight detected"

  StartupStore(_T(". Free Flight started %s%s"), WhatTimeIsIt(),NEWLINE);

  ffDetected=true;
  Calculated->FreeFlying=true;
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

  for (i=0; i<200; i++) {
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

  // MinAltitude is handled separately already taking care of FF


}

