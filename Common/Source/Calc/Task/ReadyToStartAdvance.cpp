/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "InputEvents.h"
#include "Calc/Task/TimeGates.h"

#define AUTOADVANCE_MANUAL   0
#define AUTOADVANCE_AUTO     1
#define AUTOADVANCE_ARM      2
#define AUTOADVANCE_ARMSTART 3
#define AUTOADVANCE_ARMTPS   4

bool ReadyToStart(DERIVED_INFO *Calculated) {
  if (!Calculated->Flying) {
    return false;
  }
  if (!Calculated->FreeFlying && (ISGLIDER||ISPARAGLIDER)) {
    return false;
  }
  
  if (!ValidGate()) return false; // 100509
  if (AutoAdvance== AUTOADVANCE_AUTO || AutoAdvance== AUTOADVANCE_ARMTPS) {  
    return true;
  }
  if ((AutoAdvance== AUTOADVANCE_ARM) || (AutoAdvance==AUTOADVANCE_ARMSTART)) {
    if (AdvanceArmed) {
      return true;
    }
  }
  return false;
}


bool ReadyToAdvance(DERIVED_INFO *Calculated, bool reset=true, bool restart=false) {
  static int lastReady = -1;
  static int lastActive = -1;
  bool say_ready = false;

  // 0: Manual
  // 1: Auto
  // 2: Arm
  // 3: Arm start

  if (!Calculated->Flying) {
    lastReady = -1;
    lastActive = -1;
    return false;
  }
  if (!Calculated->FreeFlying && (ISGLIDER||ISPARAGLIDER)) {
    lastReady = -1;
    lastActive = -1;
    return false;
  }

  if (AutoAdvance== AUTOADVANCE_AUTO) {  
    if (reset) AdvanceArmed = false;
    return true;
  }
  if (AutoAdvance== AUTOADVANCE_ARM) {
    if (AdvanceArmed) {
      if (reset) AdvanceArmed = false;
      return true;
    } else {
      say_ready = true;
    }
  }
  if (AutoAdvance== AUTOADVANCE_ARMSTART) { 
    if ((ActiveTaskPoint == 0) || restart) {
      if (!AdvanceArmed) {
        say_ready = true;
      } else if (reset) { 
        AdvanceArmed = false; 
        return true;
      }
    } else {
      // JMW fixed 20070528
      if (ActiveTaskPoint>0) {
        if (reset) AdvanceArmed = false;
        return true;
      }
    }
  }
  if (AutoAdvance== AUTOADVANCE_ARMTPS) { 
    if ((ActiveTaskPoint == 0) || restart) {
      if (reset)
        AdvanceArmed = false;
      return true;
    }
    else {
      if (!AdvanceArmed) {
        say_ready = true;
      } else if (reset) { 
        AdvanceArmed = false; 
        return true;
      }
    }
  }

  // see if we've gone back a waypoint (e.g. restart)
  if (ActiveTaskPoint < lastActive) {
    lastReady = -1;
  }
  lastActive = ActiveTaskPoint;

  if (say_ready) {
    if (ActiveTaskPoint != lastReady) {
      InputEvents::processGlideComputer(GCE_ARM_READY);
      lastReady = ActiveTaskPoint;
    }
  }
  return false;
}
