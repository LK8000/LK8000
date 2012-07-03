/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: InputEvents.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef INPUTEVENTS_H
#define INPUTEVENTS_H

#include "lk8000.h"
#include "externs.h"

typedef void (*pt2Event)(const TCHAR *);

HINSTANCE _loadDLL(TCHAR *name);

class InputEvents {
 public:
  static void readFile();
  static void UnloadString();

  static int mode2int(const TCHAR *mode, bool create);
  static void setMode(const TCHAR *mode);
  static TCHAR* getMode();
  static int getModeID();
  static int findKey(const TCHAR *data);
  static int findGCE(const TCHAR *data);
  static int findNE(const TCHAR *data);
  static pt2Event findEvent(const TCHAR *);
  static bool processKey(int key);
  static bool processNmea(int key);
  static bool processButton(int bindex);
  static bool processGlideComputer(int);
  static void DoQueuedEvents(void);
  static void processGo(int event_id);
  static int  makeEvent(void (*event)(const TCHAR *), const TCHAR *misc, int next = 0);
  static void makeLabel(int mode_id, const TCHAR *label, int location, int event_id);

  static void drawButtons(int Mode);


  // -------

  static void eventAbortTask(const TCHAR *misc);
  static void eventAdjustForecastTemperature(const TCHAR *misc);
  static void eventAdjustWaypoint(const TCHAR *misc);
  static void eventAnalysis(const TCHAR *misc);
  static void eventArmAdvance(const TCHAR *misc);
  static void eventBallast(const TCHAR *misc);
  static void eventBugs(const TCHAR *misc);
  static void eventCalculator(const TCHAR *misc);
  static void eventChecklist(const TCHAR *misc);
  static void eventDLLExecute(const TCHAR *misc);
  static void eventFlightMode(const TCHAR *misc);
  static void eventLogger(const TCHAR *misc);
  static void eventMacCready(const TCHAR *misc);
  static void eventMarkLocation(const TCHAR *misc);
  static void eventMode(const TCHAR *misc);
  static void eventNearestAirspaceDetails(const TCHAR *misc);
  static void eventNearestWaypointDetails(const TCHAR *misc);
  static void eventNull(const TCHAR *misc); 
  static void eventPan(const TCHAR *misc);
  static void eventPlaySound(const TCHAR *misc);
  static void eventProfileLoad(const TCHAR *misc);
  static void eventProfileSave(const TCHAR *misc);
  static void eventRepeatStatusMessage(const TCHAR *misc);
  static void eventRun(const TCHAR *misc);
  static void eventScreenModes(const TCHAR *misc);
  static void eventSendNMEA(const TCHAR *misc);
  static void eventSendNMEAPort1(const TCHAR *misc);
  static void eventSendNMEAPort2(const TCHAR *misc);
  static void eventSetup(const TCHAR *misc);
  static void eventSnailTrail(const TCHAR *misc);
  static void eventVisualGlide(const TCHAR *misc); // VENTA3
  static void eventAirSpace(const TCHAR *misc); // VENTA3
  static void eventSounds(const TCHAR *misc);
  static void eventStatus(const TCHAR *misc);
  static void eventStatusMessage(const TCHAR *misc);
  static void eventTaskLoad(const TCHAR *misc);
  static void eventTaskSave(const TCHAR *misc);
  static void eventTerrainTopology(const TCHAR *misc);
  static void eventWaypointDetails(const TCHAR *misc);
  static void eventWind(const TCHAR *misc);
  static void eventZoom(const TCHAR *misc);
  static void eventDeclutterLabels(const TCHAR *misc);
  static void eventExit(const TCHAR *misc);
  static void eventBeep(const TCHAR *misc);
  static void eventUserDisplayModeForce(const TCHAR *misc);
  static void eventAirspaceDisplayMode(const TCHAR *misc);
  static void eventAutoLogger(const TCHAR *misc);
  static void eventMyMenu(const TCHAR *misc);
  static void eventAddWaypoint(const TCHAR *misc);
  static void eventOrientation(const TCHAR *misc);
  static void eventCalcWind(const TCHAR *misc);
  static void eventInvertColor(const TCHAR *misc);
  static void eventChangeBack(const TCHAR *misc);
  static void eventResetTask(const TCHAR *misc);
  static void eventResetQFE(const TCHAR *misc);
  static void eventRestartCommPorts(const TCHAR *misc);
  static void eventMoveGlider(const TCHAR *misc);
  static void eventActiveMap(const TCHAR *misc);
  static void eventChangeWindCalcSpeed(const TCHAR *misc);
  static void eventTimeGates(const TCHAR *misc);
  static void eventChangeMultitarget(const TCHAR *misc);
  static void eventBaroAltitude(const TCHAR *misc);
  static void eventChangeHGPS(const TCHAR *misc);
  static void eventChangeGS(const TCHAR *misc);
  static void eventChangeTurn(const TCHAR *misc);
  static void eventService(const TCHAR *misc);
  // -------

  static void showErrors();

#ifdef LXMINIMAP
  static int getSelectedButtonIndex();
  static void eventChangeSorting(const TCHAR *misc);
  static bool isSelectMode();
#endif
  // This must be public in any case
  static void eventMinimapKey(const TCHAR *misc);

 private:
  static bool processGlideComputer_real(int);
  static bool processNmea_real(int key);
};


// GCE = Glide Computer Event
enum {
  GCE_AIRSPACE_ENTER,
  GCE_AIRSPACE_LEAVE,
  GCE_COMMPORT_RESTART,
  GCE_FLARM_NOTRAFFIC,
  GCE_FLARM_TRAFFIC,
  GCE_FLIGHTMODE_CLIMB,
  GCE_FLIGHTMODE_CRUISE,
  GCE_FLIGHTMODE_FINALGLIDE,
  GCE_FLIGHTMODE_FINALGLIDE_TERRAIN,
  GCE_FLIGHTMODE_FINALGLIDE_ABOVE,	
  GCE_FLIGHTMODE_FINALGLIDE_BELOW,
  GCE_GPS_CONNECTION_WAIT,
  GCE_GPS_FIX_WAIT,
  GCE_HEIGHT_MAX,
  GCE_LANDING,
  GCE_STARTUP_REAL,
  GCE_STARTUP_SIMULATOR,
  GCE_TAKEOFF,
  GCE_TASK_NEXTWAYPOINT,
  GCE_TASK_START,
  GCE_TASK_FINISH,
  GCE_TEAM_POS_REACHED,
  GCE_ARM_READY,
  GCE_TASK_CONFIRMSTART,
  GCE_COUNT			// How many we have for arrays etc
};

// NE = NMEA Events (hard coded triggered events from the NMEA processor)
enum {
  NE_DOWN_IN_FLAP_POS=                  0,
  NE_DOWN_IN_FLAP_ZERO=                 1,
  NE_DOWN_IN_FLAP_NEG=                  2,
  NE_DOWN_IN_SC=                        3,
  NE_DOWN_IN_GEAR_RETRACTED=            4,
  NE_DOWN_IN_GEAR_EXTENDED=             5,
  NE_DOWN_IN_AIRBRAKENOTLOCKED=         6,
  NE_DOWN_IN_AUX=                       7,
  NE_DOWN_IN_ACK=                       8,
  NE_DOWN_IN_REP=                       9,
  NE_DOWN_IN_CIRCLING_PDA=              10,
  NE_DOWN_IN_CIRCLING_FLARM=            11,
  NE_DOWN_IN_FLYING=                    12,
  NE_DOWN_IN_NOTFLYING=                 13,
  NE_DOWN_IN_PDA_CONNECTED=             14,
  NE_DOWN_IN_VELOCITY_MANOEUVERING=     15,
  NE_DOWN_IN_VELOCITY_FLAP=             16,
  NE_DOWN_IN_VELOCITY_AIRBRAKE=         17,
  NE_DOWN_IN_VELOCITY_TERRAIN=          18,
  NE_DOWN_IN_VELOCITY_NEVEREXCEED=      19,
  NE_DOWN_IN_STALL=                     20,
  NE_DOWN_IN_AIRBRAKELOCKED=            21,
  NE_DOWN_IN_TAKINGOFF=                 22,
  NE_DOWN_IN_USERSWUP=                  23,
  NE_DOWN_IN_USERSWMIDDLE=              24,
  NE_DOWN_IN_USERSWDOWN=                25,

  NE_UNUSED_0 =                      26,
  NE_UNUSED_1 =                      27,
  NE_UNUSED_2 =                      28,
  NE_UNUSED_3 =                      29,
  NE_UNUSED_4 =                      30,
  NE_UNUSED_5 =                      31,

  NE_DOWN_OUT_CIRCLING=                 32,
  NE_DOWN_OUT_VELOCITY_MANOEUVERING=    33,
  NE_DOWN_OUT_VELOCITY_FLAP=            34,
  NE_DOWN_OUT_VELOCITY_AIRBRAKE=        35,
  NE_DOWN_OUT_VELOCITY_TERRAIN=         36,
  NE_DOWN_OUT_VELOCITY_NEVEREXCEED=     37,
  NE_DOWN_OUT_GEAR_LANDING=             38,
  NE_DOWN_OUT_FLAP_LANDING=             39,
  NE_DOWN_OUT_AIRBRAKE_TAKEOFF=         40,
  NE_DOWN_OUT_STALL=                    41,

  NE_UNUSED_6 =                      42,
  NE_UNUSED_7 =                      43,
  NE_UNUSED_8 =                      44,
  NE_UNUSED_9 =                      45,
  NE_UNUSED_10 =                      46,
  NE_UNUSED_11 =                      47,
  NE_UNUSED_12 =                      48,
  NE_UNUSED_13 =                      49,
  NE_UNUSED_14 =                      50,
  NE_UNUSED_15 =                      51,
  NE_UNUSED_16 =                      52,
  NE_UNUSED_17 =                      53,
  NE_UNUSED_18 =                      54,
  NE_UNUSED_19 =                      55,
  NE_UNUSED_20 =                      56,
  NE_UNUSED_21 =                      57,
  NE_UNUSED_22 =                      58,
  NE_UNUSED_23 =                      59,
  NE_UNUSED_24 =                      60,
  NE_UNUSED_25 =                      61,
  NE_UNUSED_26 =                      62,
  NE_UNUSED_27 =                      63,

  NE_UP_IN_FLAP_POS=                  64,
  NE_UP_IN_FLAP_ZERO=                 65,
  NE_UP_IN_FLAP_NEG=                  66,
  NE_UP_IN_SC=                        67,
  NE_UP_IN_GEAR_RETRACTED=            68,
  NE_UP_IN_GEAR_EXTENDED=             69,
  NE_UP_IN_AIRBRAKENOTLOCKED=         70,
  NE_UP_IN_AUX=                       71,
  NE_UP_IN_ACK=                       72,
  NE_UP_IN_REP=                       73,
  NE_UP_IN_CIRCLING_PDA=              74,
  NE_UP_IN_CIRCLING_FLARM=            75,
  NE_UP_IN_FLYING=                    76,
  NE_UP_IN_NOTFLYING=                 77,
  NE_UP_IN_PDA_CONNECTED=             78,
  NE_UP_IN_VELOCITY_MANOEUVERING=     79,
  NE_UP_IN_VELOCITY_FLAP=             80,
  NE_UP_IN_VELOCITY_AIRBRAKE=         81,
  NE_UP_IN_VELOCITY_TERRAIN=          82,
  NE_UP_IN_VELOCITY_NEVEREXCEED=      83,
  NE_UP_IN_STALL=                     84,
  NE_UP_IN_AIRBRAKELOCKED=            85,
  NE_UP_IN_TAKINGOFF=                 86,
  NE_UP_IN_USERSWUP=                  87,
  NE_UP_IN_USERSWMIDDLE=              88,
  NE_UP_IN_USERSWDOWN=                89,

  NE_UNUSED_28 =                      90,
  NE_UNUSED_29 =                      91,
  NE_UNUSED_30 =                      92,
  NE_UNUSED_31 =                      93,
  NE_UNUSED_32 =                      94,
  NE_UNUSED_33 =                      95,

  NE_UP_OUT_CIRCLING=                 96,
  NE_UP_OUT_VELOCITY_MANOEUVERING=    97,
  NE_UP_OUT_VELOCITY_FLAP=            98,
  NE_UP_OUT_VELOCITY_AIRBRAKE=        99,
  NE_UP_OUT_VELOCITY_TERRAIN=         100,
  NE_UP_OUT_VELOCITY_NEVEREXCEED=     101,
  NE_UP_OUT_GEAR_LANDING=             102,
  NE_UP_OUT_FLAP_LANDING=             103,
  NE_UP_OUT_AIRBRAKE_TAKEOFF=         104,
  NE_UP_OUT_STALL=                    105,

  NE_UNUSED_34 =                      106,
  NE_UNUSED_35 =                      107,
  NE_UNUSED_36 =                      108,
  NE_UNUSED_37 =                      108,	// XXX Duplicate of above
  NE_UNUSED_38 =                      109,
  NE_UNUSED_39 =                      110,
  NE_UNUSED_40 =                      111,
  NE_UNUSED_41 =                      112,
  NE_UNUSED_42 =                      113,
  NE_UNUSED_43 =                      114,
  NE_UNUSED_44 =                      115,
  NE_UNUSED_45 =                      116,
  NE_UNUSED_46 =                      117,
  NE_UNUSED_47 =                      118,
  NE_UNUSED_48 =                      119,
  NE_UNUSED_49 =                      120,
  NE_UNUSED_50 =                      121,
  NE_UNUSED_51 =                      122,
  NE_UNUSED_52 =                      122,	// XXX Duplicate of above
  NE_UNUSED_53 =                      123,
  NE_UNUSED_54 =                      124,
  NE_UNUSED_55 =                      125,
  NE_UNUSED_56 =                      126,
  NE_UNUSED_57 =                      127,
  NE_COUNT = 132, // How many we have for arrays etc // XXX Increased arbitrarily for duplicates above
};


#endif
