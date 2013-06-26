#if !defined(AFX_DIALOGS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_DIALOGS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "dlgTools.h"

void LKReadLanguageFile(void);
void LKUnloadMessage();

void SetWindowText_gettext(HWND hDlg, int entry);
void ClearStatusMessages(void);

void StartupScreen();

HWND CreateProgressDialog(TCHAR *text);
void CloseProgressDialog();
void StartHourglassCursor();
void StopHourglassCursor();

#define IM_AIRSPACE 1
#define IM_WAYPOINT 2
#define IM_FLARM    3

typedef	struct{
  char type;
  long* ptr;

  int iIdx;
  double Dist;

} ListElement;


bool dlgAirspaceWarningShowDlg(bool force);
// int dlgWayPointSelect(void);
int dlgWayPointSelect(double lon=0.0, double lat=90.0, int type=-1, int FilterNear=0);
int dlgAirspaceColoursShowModal(void);
ListElement* dlgMultiSelectListShowModal(void);
void dlgAddMultiSelectListItem(long* pNew ,int Idx, char type, double Distance);
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
void dlgTaskWaypointShowModal(int itemindex, int type, bool addonly=false);
void dlgTaskOverviewShowModal(void);
void dlgVoiceShowModal(void);
void dlgWayPointDetailsShowModal(short mypage);
short dlgWayQuickShowModal(void);
void dlgTextEntryShowModal(TCHAR *text, int width=0, bool WPKeyRed= false);
void dlgTeamCodeShowModal(void);
void dlgStartPointShowModal(void);
void dlgWaypointEditShowModal(WAYPOINT *wpt);
void dlgAirspaceSelect(void);
void dlgTarget(void);
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

#if (WINDOWSPC>0)
#ifdef DEBUG
#pragma warning( disable : 4786 ) 
#endif
#endif

void WriteMissingTranslations(void);
void dlgTextEntryKeyboardShowModal(TCHAR *text, int width=0);

#endif
