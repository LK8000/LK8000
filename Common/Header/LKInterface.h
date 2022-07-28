/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: LKInterface.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/
#if !defined(LK8000_LKINTERFACE_H)
#define LK8000_LKINTERFACE_H

class WndForm;

void    InitLKFonts();
void    DeInitLKFonts();

void    InitLKScreen();
void    InitLK8000();
void    InitModeTable();

bool	CustomKeyHandler(unsigned key);
const TCHAR* CustomKeyLabel(unsigned key);
void AddCustomKeyList(WndForm* pForm, const TCHAR* WndName, unsigned value);
void GetCustomKey(WndForm* pForm, const TCHAR* WndName, unsigned short& value);
void GetCustomKey(WndForm* pForm, const TCHAR* WndName, int& value);

void	BottomBarChange(bool advance);
void	InfoPageChange(bool advance);
void	SetModeType(short modeindex, short modetype);
bool    IsActiveModeType(short modeindex, short modetype);
void	NextModeType();
void	PreviousModeType();
void	NextModeIndex();
void	PreviousModeIndex();
void	SetModeIndex(short i);
void	SoundModeIndex();
void	SelectMapSpace(short i);
void	UnselectMapSpace(short i);
int	GetInfoboxType(int i);
int	GetInfoboxIndex(int i, MapWindow::Mode::TModeFly dmMode);

int GetOvertargetIndex();
void GetOvertargetName(TCHAR *overtargetname);
extern TCHAR * GetOvertargetHeader(void);
extern void RotateOvertarget(void);
void ExternalDeviceSendTarget();
void ResetMultitargetSync();

int ProcessVirtualKey(int x, int y, long keytime, short vkmode);

void BottomSounds();


typedef enum{
	ckDisabled=0,
	ckMenu,
	ckBackMode,
	ckToggleMap,
	ckToggleMapLandable,
	ckLandables,
	ckToggleMapCommons,
	ckCommons,
	ckToggleMapTraffic,
	ckTraffic,
	ckInvertColors,
	ckTrueWind,
	ckToggleOverlays,
	ckAutoZoom,
	ckActiveMap,
	ckMarkLocation,
	ckTimeGates,
	ckBooster,
	ckGoHome,
	ckPanorama,
	ckMultitargetRotate,
	ckMultitargetMenu,
	ckTeamCode,
	ckBaroToggle,
	ckBasicSetup,
	ckSimMenu,
	ckAirspaceAnalysis,
	ckToggleMapAirspace,
	ckZoomIn,
	ckZoomOut,
	ckZoomInMore,
	ckZoomOutMore,
	ckOptimizeRoute,
	ckLockScreen,
	ckWhereAmI,
	ckUseTotalEnergy,
	ckNotepad,
	ckTerrainColors,
	ckNearestAirspace,
	ckOlcAnalysis,
	ckTerrainColorsBack,
	ckForceFreeFlightRestart,
	ckCustomMenu1,
	ckTaskCalc,
	ckTaskTarget,
	ckArmAdvance,
	ckMessageRepeat,
	ckWaypointLookup,
	ckPan,
	ckWindRose,
	ckFlarmRadar,
	ckDeviceA,
	ckDeviceB,
	ckResetOdometer,
	ckForceLanding,
	ckResetTripComputer,
	ckSonarToggle,
	ckResetView,
	ckMapOrient,
	ckResetComm,
	ckDspMode,
	ckDrawXCToggle,
	ckAirspaceLookup,
    ckDeviceC,
    ckDeviceD,
    ckDeviceE,
    ckDeviceF,
    ckRadioDlg,
	ckMCSetting,
	ckTOP
} CustomKeyMode_t;


#endif // LK8000_LKINTERFACE_H
