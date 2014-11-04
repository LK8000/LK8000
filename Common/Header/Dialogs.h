#if !defined(AFX_DIALOGS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_DIALOGS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "options.h"
#include "dlgTools.h"


void ClearStatusMessages(void);

void StartupScreen();

HWND CreateProgressDialog(const TCHAR *text);
void CloseProgressDialog();
void StartHourglassCursor();
void StopHourglassCursor();


#define GC_SUB_STRING_THRESHOLD 1

#define IM_AIRSPACE   1
#define IM_WAYPOINT   2
#define IM_FLARM      3
#define IM_THERMAL_PT 4
#define IM_TOPO_PT    5
#define IM_TASK_PT    6
#define IM_AIRFIELD   7
#define IM_OUTLAND    8
#define IM_OWN_POS    9
#define IM_ORACLE     10
#define IM_TEAM       11


typedef	struct{
  char type;
  void* ptr;

  int iIdx;
  double Dist;
  char Subtype;

} ListElement;


bool dlgAirspaceWarningShowDlg(bool force);
// int dlgWayPointSelect(void);
int dlgWayPointSelect(double lon=0.0, double lat=90.0, int type=-1, int FilterNear=0);
int dlgAirspaceColoursShowModal(void);
ListElement* dlgMultiSelectListShowModal(void);
void dlgAddMultiSelectListItem(long* pNew ,int Idx, char type, double Distance);
int dlgGetNoElements(void);
int dlgAirspacePatternsShowModal(void);
bool dlgAirspaceShowModal(bool colored);
void dlgBasicSettingsShowModal(void);
void dlgBrightnessShowModal(void);
void dlgHelpShowModal(const TCHAR* Caption, const TCHAR* HelpText);
void dlgChecklistShowModal(short checklistmode);
void dlgConfigurationShowModal(short mode);
void dlgVegaDemoShowModal(void);
bool dlgConfigurationVarioShowModal(void);
void dlgLoggerReplayShowModal(void);
short dlgStartupShowModal(void);
void dlgTaskCalculatorShowModal(void);
void dlgWindSettingsShowModal(void);
void dlgStartTaskShowModal(bool *validStart, double Time, double Speed, double Altitude);
void dlgAnalysisShowModal(int inpage);
void dlgStatusShowModal(int page);
void dlgSwitchesShowModal(void);
void dlgTaskWaypointShowModal(int itemindex, int type, bool addonly=false, bool Moveallowed=false);
void dlgTaskOverviewShowModal(int Idx=-1);
void dlgVoiceShowModal(void);
void dlgWayPointDetailsShowModal(short mypage);
short dlgWayQuickShowModal(void);
int  dlgTextEntryShowModal(TCHAR *text, int width=0, bool WPKeyRed= false);
void dlgNumEntryShowModal(TCHAR *text, int width, bool );
void dlgTeamCodeShowModal(void);
void dlgStartPointShowModal(void);
void dlgWaypointEditShowModal(WAYPOINT *wpt);
void dlgAirspaceSelect(void);
void dlgTarget(int TaskPoint = -1);
bool dlgTaskRules(void);
void dlgAirspaceDetails(CAirspace *airspace);
bool dlgAirspaceWarningVisible(void);
void dlgLKTrafficDetails(int indexid);
void dlgThermalDetails(int indexid);
void dlgTimeGatesShowModal(void);
void dlgTopologyShowModal(void);
void dlgCustomKeysShowModal(void);
void dlgBottomBarShowModal(void);
void dlgInfoPagesShowModal(void);
void dlgProfilesShowModal(short mode);
void dlgAirspaceWarningParamsShowModal(void);
void dlgMultimapsShowModal(void);
void dlgIgcFileShowModal(void);

#if (WINDOWSPC>0)
#ifdef DEBUG
#pragma warning( disable : 4786 ) 
#endif
#endif

void WriteMissingTranslations(void);

void dlgTextEntryKeyboardShowModal(TCHAR *text, int width=0);
void dlgNumEntryShowModal(TCHAR *text, int width,bool WPKeyRed);

#ifndef NO_BLUETOOTH
namespace DlgBluetooth {
    void Show();
};
#endif

#endif
