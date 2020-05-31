/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: InputEvents.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef INPUTEVENTS_H
#define INPUTEVENTS_H


typedef void (*pt2Event)(const TCHAR *);

class InputEvents {
private:
  static double getIncStep(const TCHAR *misc, double step);
  static double getIncStep(double step, int *count, int *otherCount);

public:

  typedef enum {
    PopupWaypoint,
    PopupThermal,
    PopupTraffic,
    PopupOracle,
    PopupTeam,
    PopupBasic,
    PopupWeatherSt,
  } PopupType;


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
  static bool processButton(unsigned MenuId);
  static bool processGlideComputer(int);
  static void processPopupDetails(PopupType type, int index);
  static void DoQueuedEvents(void);
  static void processGo(int event_id);
  static int  makeEvent(void (*event)(const TCHAR *), const TCHAR *misc, int next = 0);
  static void makeLabel(int mode_id, const TCHAR *label, unsigned MenuId, int event_id);

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
  static void eventSendNMEAPort3(const TCHAR *misc);
  static void eventSendNMEAPort4(const TCHAR *misc);
  static void eventSendNMEAPort5(const TCHAR *misc);
  static void eventSendNMEAPort6(const TCHAR *misc);
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
  static void eventChangeNettoVario(const TCHAR *misc);
  static void eventService(const TCHAR *misc);
  static void eventWifi(const TCHAR *misc);

  static void eventInfoStripe(const TCHAR *misc);
  static void eventInfoPage(const TCHAR *misc);
  static void eventModeType(const TCHAR *misc);

  static void eventShowMultiselect(const TCHAR *misc);
  // -------

  static void showErrors();

  static unsigned getSelectedButtonId();
  static void selectNextButton();
  static void selectPrevButton();
  static void triggerSelectedButton();

#ifdef LXMINIMAP
  static void eventChangeSorting(const TCHAR *misc);
  static bool isSelectMode();
#endif
  // This must be public in any case
  static void eventMinimapKey(const TCHAR *misc);

 private:
  static bool processGlideComputer_real(int);
  static bool processNmea_real(int key);
  static void processPopupDetails_real();
};


// GCE = Glide Computer Event
enum {
  GCE_COMMPORT_RESTART,
  GCE_FLARM_NOTRAFFIC,
  GCE_FLARM_TRAFFIC,
  GCE_FLIGHTMODE_CLIMB,
  GCE_FLIGHTMODE_CRUISE,
  GCE_FLIGHTMODE_FINALGLIDE,
  GCE_FLIGHTMODE_FINALGLIDE_TERRAIN,
  GCE_FLIGHTMODE_FINALGLIDE_ABOVE,
  GCE_FLIGHTMODE_FINALGLIDE_BELOW,
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
  GCE_POPUP_MULTISELECT,
  GCE_WAYPOINT_DETAILS_SCREEN,
  GCE_COUNT			// How many we have for arrays etc
};

// NE = NMEA Events (hard coded triggered events from the NMEA processor)
// NMEA Events are Not Used, leave in code only for allow reuse in futur.
enum {
  NE_COUNT // How many we have for arrays etc
};


#endif
