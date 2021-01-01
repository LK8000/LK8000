#if !defined(AFX_DIALOGS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_DIALOGS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#include "options.h"
#include "Enums.h"
class CAirspace;

MsgReturn_t MessageBoxX(LPCTSTR lpText, LPCTSTR lpCaption, MsgType_t uType, bool wfullscreen=false);

void StartupScreen();

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
#define IM_WEATHERST  12


typedef	struct{
  char type;
  void* ptr;

  int iIdx;
  double Dist;
  char Subtype;

} ListElement;


int dlgSelectWaypoint(int type=-1, int FilterNear=0);
void dlgSelectAirspace();

int dlgAirspaceColoursShowModal(void);
ListElement* dlgMultiSelectListShowModal(void);
void dlgAddMultiSelectListItem(long* pNew ,int Idx, char type, double Distance);
int dlgGetNoElements(void);
#ifdef HAVE_HATCHED_BRUSH
int dlgAirspacePatternsShowModal(void);
#endif
bool dlgAirspaceShowModal(bool colored);
void dlgBasicSettingsShowModal(void);
void dlgHelpShowModal(const TCHAR* Caption, const TCHAR* HelpText, bool bHelpCapt = true);
void dlgChecklistShowModal(short checklistmode);
void dlgConfigurationShowModal(short mode);
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
void dlgWayPointDetailsShowModal(short mypage);
short dlgWayQuickShowModal(void);

struct key_filter_interface {
  virtual const TCHAR* GetLabel() const = 0;
  virtual bool isHiddenKey(TCHAR c) const = 0;
  
  virtual unsigned GetMatchCount() const = 0;
  virtual const TCHAR* GetMatchText() const = 0;

  virtual void Update(const TCHAR* text) = 0;
};

void dlgTextEntryShowModal(TCHAR *text, int width, key_filter_interface* filter = nullptr);
void dlgNumEntryShowModal(TCHAR *text, int width);

void dlgTeamCodeShowModal(void);
void dlgStartPointShowModal(void);
void dlgWaypointEditShowModal(WAYPOINT *wpt);
void dlgTarget(int TaskPoint = -1);
bool dlgTaskRules(void);

void dlgLKTrafficDetails(int indexid);
void dlgThermalDetails(int indexid);
void dlgTimeGatesShowModal(void);
void dlgTopologyShowModal(void);
void dlgCustomKeysShowModal(void);
void dlgBottomBarShowModal(void);
void dlgInfoPagesShowModal(void);
void dlgProfilesShowModal(short mode);
void dlgAirspaceWarningParamsShowModal(void);
void dlgAirspaceFilesShowModal(void);
void dlgWaypointFilesShowModal(void);
void dlgMultimapsShowModal(void);
void dlgIgcFileShowModal(void);
void dlgWeatherStDetails(int indexid);

void dlgRadioSettingsShowModal(void);
void dlgOracleShowModal();
void dlgTerminal(int portnum);
void dlgOverlaysShowModal(void);
void dlgCustomMenuShowModal(void);

#ifndef NO_BLUETOOTH
namespace DlgBluetooth {
    void Show();
};
#endif

#endif
