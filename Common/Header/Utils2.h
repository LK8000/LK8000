/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils2.h,v 8.3 2010/12/15 12:40:49 root Exp root $
*/
#if !defined(LK8000_UTILS2_H_)
#define LK8000_UTILS2_H_

#if !defined(AFX_CALCULATIONS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#include "Calculations.h"
#endif

#include "MapWindow.h"

// These are used by LK functions, there is no overflow check so caution
// generic text buffer
#define LKSIZETEXT		50
// Generic Tchar buffer
#define LKSIZEBUFFER		50
// Tchar buffer used for messages
#define LKSIZEBUFFERLARGE	100

#define LKSIZEBUFFERTITLE	30
#define LKSIZEBUFFERUNIT	15
#define LKSIZEBUFFERVALUE	30

// size of a path, complete with final file name
#define LKSIZEBUFFERPATH	150

// size of nmea string max for logging
#define LKSIZENMEA		300

#if 0
int	FilterFast(ifilter_s *buf, int minvalue, int maxvalue);
bool	InitFilterBuffer(ifilter_s *buf, short bsize);
int	FilterRotary(ifilter_s *buf, int minvalue, int maxvalue);
void	InsertRotaryBuffer(ifilter_s *buf, int value);
#endif

bool	InitLDRotary(ldrotary_s *buf);
void	InsertLDRotary(ldrotary_s *buf, int distance, int altitude);
double	CalculateLDRotary(ldrotary_s *buf, DERIVED_INFO *Calculated);

// TrueWind functions
void	InitWindRotary(windrotary_s *wbuf);
void	InsertWindRotary(windrotary_s *wbuf, double speed, double track, double altitude);
int	CalculateWindRotary(windrotary_s *wbuf, double iaspeed, double *wfrom, double *wspeed, int windcalctime, int wmode);

void	SetOverColorRef();
bool	CustomKeyHandler(const int key);
void	SetMapScales(void);
bool	LoadModelFromProfile(void);
TCHAR*  GetSizeSuffix(void);
void	LKRunStartEnd(bool);

void	InitNewMap();
#ifndef MAP_ZOOM
void	InitAircraftCategory();
#endif /* ! MAP_ZOOM */
void	InitScreenSize();
void	InitLK8000();
void	LockMap();
void	UnlockMap();
int   GetFontRenderer();
bool	LockMode(short lmode);
int	roundupdivision(int a, int b);
void	Cpustats(int *acc, FILETIME *a, FILETIME *b, FILETIME *c, FILETIME *d);
#ifdef FIVV
void	CidContest();
void	CidInit();
#endif
void	InitModeTable();
void	SetModeType(short modeindex, short modetype);
void	NextModeType();
void	PreviousModeType();
void	NextModeIndex();
void	PreviousModeIndex();
void	SetModeIndex();
void	SoundModeIndex();
void	SelectMapSpace(short i);
void	UnselectMapSpace(short i);
int	GetInfoboxType(int i);
#ifndef MAP_ZOOM
int	GetInfoboxIndex(int i, short dmMode);
#else /* MAP_ZOOM */
int	GetInfoboxIndex(int i, MapWindow::Mode::TModeFly dmMode);
#endif /* MAP_ZOOM */
double	GetMacCready(int wpindex, short wpmode);
void	unicodetoascii(TCHAR *text, int tsize, char *atext);

int ProcessVirtualKey(int x, int y, long keytime, short vkmode);

extern int GetOvertargetIndex(void);
extern void GetOvertargetName(TCHAR *overtargetname);
extern TCHAR * GetOvertargetHeader(void);
extern void RotateOvertarget(void);
extern void ToggleOverlays(void);
extern bool CheckClubVersion(void);
extern void ClubForbiddenMsg(void);

#if ORBITER
extern void CalculateOrbiter(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
#endif

extern HFONT                                   LK8UnitFont;
extern HFONT					LK8TitleFont;
extern HFONT					LK8TitleNavboxFont;
extern HFONT					LK8MapFont;
extern HFONT                                   LK8ValueFont;
extern HFONT                                   LK8TargetFont;
extern HFONT                                   LK8BigFont;
extern HFONT                                   LK8SmallFont;
extern HFONT                                   LK8MediumFont;
extern HFONT                                   LK8SymbolFont;
extern HFONT                                   LK8InfoBigFont;
extern HFONT                                   LK8InfoBigItalicFont;
extern HFONT                                   LK8InfoNormalFont;
extern HFONT					LK8InfoSmallFont;
extern HFONT					LK8PanelBigFont;
extern HFONT					LK8PanelMediumFont;
extern HFONT					LK8PanelSmallFont;
extern HFONT					LK8PanelUnitFont;

// BottomModes aka navboxes
// order is important, and thermal that MUST stay in position 0. Changing order will change sequence.
// Warning, these names have been renamed in MapWindow3, for example BM_AUX appears on screen as NAV2
// Do not confuse these indicators with screen overlay near map zoom
#if 1
#define BM_TRM		0	// TRM
#define	BM_CRU		1	// NAV
#define BM_HGH		2	// ALT
#define BM_AUX		3	// STA
#define BM_TSK		4	// TSK
#define BM_ALT		5	// ATN
#define BM_SYS		6	// SYS
#define BM_CUS2		7	// CRU
#define BM_CUS3		8	// FIN
#define BM_CUS		9	// AUX
#else
#define BM_TRM		0	// TRM
#define	BM_CRU		1	// NAV
#define BM_AUX		2	// STA
#define BM_TSK		3	// TSK
#define BM_SYS		4	// SYS
#define BM_ALT		5	// ATN
#define BM_CUS		6	// AUX
#define BM_CUS2		7	// CRU
#define BM_CUS3		8	// FIN
#define BM_HGH		9	// ALT
#endif
// these are limiters
#define BM_FIRST	1
#define BM_LAST		9

// Global MapSpaceModes : order is not important and you can also have unused modes
//
#define MSM_WELCOME		0
#define MSM_MAP			1
#define MSM_LANDABLE		2
#define MSM_AIRPORTS		3
#define MSM_COMMON		4
#define MSM_RECENT		5
#define MSM_INFO_THERMAL	6
#define MSM_INFO_CRUISE		7
#define MSM_INFO_TASK		8
#define MSM_INFO_AUX		9
#define MSM_INFO_TRI		10
#define MSM_NEARTPS		11
#define MSM_TRAFFIC		12
// infopage for traffic
#define MSM_INFO_TRF		13
// target page for traffic with graphics
#define MSM_INFO_TARGET		14
#define MSM_INFO_CONTEST	15
// turnaround point is TOP (equal to last TRI define), 
// remember that arrays must count from zero, so MSM_TOP+1
#define MSM_TOP			15
//
// THIS CONFIGURATION GIVES THE ORDER OF MENUs. ALL ITEMS MUST ALSO BE ADDED INSIDE INITMODETABLE()
// in Utils2.cpp WHERE each mode is paired with an MSM_xxx item.
// 
// Modes
#define LKMODE_MAP		0
#define LKMODE_INFOMODE		1
#define LKMODE_WP		2
#define LKMODE_NAV		3
#define LKMODE_TRF		4	// TRF page must be the last one
#define LKMODE_TOP		4
//
// Map mode
//
#define MP_WELCOME		0
#define MP_MOVING		1
#define MP_TOP			1
//
// WP mode
//
#define WP_AIRPORTS		0
#define WP_LANDABLE		1
#define WP_NEARTPS		2
#define WP_TOP			2
//
// InfoMode 
//
#define IM_CRUISE		0
#define IM_THERMAL		1
#define IM_TASK			2
#define IM_AUX			3
#define IM_CONTEST		4
#define IM_TRI			5
#define IM_TOP			5
//
// Navigation mode
//
#define NV_COMMONS		0
#define NV_HISTORY		1
#define NV_TOP			1
// 
// Traffic mode
// 
#define TF_LIST			0
#define IM_TRF			1
#define IM_TARGET		2
#define TF_TOP			2

//
extern short ModeTable[LKMODE_TOP+1][MSM_TOP+1];
extern short ModeType[LKMODE_TOP+1];
extern short ModeTableTop[LKMODE_TOP+1];
//
// Events generated by virtual keys, processed in batch mode
//
#define LKEVENT_NONE		0
#define LKEVENT_ENTER		1
#define LKEVENT_PAGEUP		2
#define LKEVENT_PAGEDOWN	3
#define LKEVENT_UP		4
#define LKEVENT_DOWN		5
//
#define LKEVENT_NEWRUN		9
#define LKEVENT_NEWPAGE		10
// Virtual Keys Gestures
// Detected in MapWindow and passed to ProcessVirtualKey in Utils2
#define LKGESTURE_NONE		0
#define LKGESTURE_UP		1
#define LKGESTURE_DOWN		2
#define LKGESTURE_LEFT		3
#define LKGESTURE_RIGHT		4
#define LKGESTURE_DOWNLEFTRIGHT	5
#define LKGESTURE_DOWNRIGHTLEFT	6
#define LKGESTURE_UPLEFTRIGHT	7
#define LKGESTURE_UPRIGHTLEFT	8
//
// LKValues aka InfoBox/Navbox/InfoPage index
#define LK_HNAV			0		// nav altitude
#define LK_HAGL			1		// height above ground level
#define LK_TC_30S		2		// current thermal 30s 
#define LK_BRG			3		// bearing
#define LK_LD_INST		4		// ld instantaneous
#define LK_LD_CRUISE		5		// ld cruise
#define LK_GNDSPEED		6		// ground speed
#define LK_TL_AVG		7		// last thermal average
#define LK_TL_GAIN		8		// last thermal gain 091216
#define LK_TL_TIME		9		// 091216
#define LK_MC			10		//
#define LK_NEXT_DIST		11		//
#define LK_NEXT_ALTDIFF		12		//
#define LK_NEXT_ALTREQ		13		//
#define LK_NEXT_WP		14		// UNSUPPORTED - nothing is returned
#define LK_FIN_ALTDIFF		15		//
#define LK_FIN_ALTREQ		16		//
#define LK_SPEEDTASK_AVG	17		//
#define LK_FIN_DIST		18		//
#define LK_FIN_LD		19		//
#define LK_HGND			20		//
#define LK_TC_AVG		21		// current thermal average
#define LK_TC_GAIN		22		// current thermal gain
#define LK_TRACK		23		//
#define LK_VARIO		24		//
#define LK_WIND_SPEED		25		//
#define LK_WIND_BRG		26		//
#define LK_AA_TIME		27		//
#define LK_AA_DISTMAX		28		//
#define LK_AA_DISTMIN		29		//
#define LK_AA_SPEEDMAX		30		//
#define LK_AA_SPEEDMIN		31		//
#define LK_IAS			32		//
#define LK_HBARO		33		//
#define LK_SPEED_MC		34		//
#define LK_PRCCLIMB		35		//
#define LK_TIMEFLIGHT		36		//
#define LK_GLOAD		37		//
#define LK_NEXT_LD		38		//
#define LK_TIME_LOCAL		39		//
#define LK_TIME_UTC		40		//
#define LK_FIN_ETE		41		//
#define LK_NEXT_ETE		42		//
#define LK_SPEED_DOLPHIN	43		//
#define LK_NETTO		44		//
#define LK_FIN_ETA		45		//
#define LK_NEXT_ETA		46		//
#define LK_BRGDIFF		47		//
#define LK_OAT			48		// 091216
#define LK_RELHUM		49		// UNSUPPORTED
#define LK_MAXTEMP		50		// UNSUPPORTED
#define LK_AA_TARG_DIST		51		//
#define LK_AA_TARG_SPEED	52		//
#define LK_LD_VARIO		53		//
#define LK_TAS			54		//
#define LK_TEAM_CODE		55		// 091216
#define LK_TEAM_BRG		56		// 091216
#define LK_TEAM_BRGDIFF		57		// 091216
#define LK_TEAM_DIST		58		// 091216
#define LK_SPEEDTASK_INST	59		//
#define LK_HOME_DIST		60		//
#define LK_SPEEDTASK_ACH	61		//
#define LK_AA_DELTATIME		62		//
#define LK_TC_ALL		63		// Average of all thermals so far
#define LK_VARIO_DIST		64		//
#define LK_BATTERY		65		//
#define LK_FIN_GR		66		//
#define LK_ALTERNATESGR		67		// not a mistake: let it here before the alternates GR
#define LK_ALTERN1_GR		67		//
#define LK_ALTERN2_GR		68		//
#define LK_BESTALTERN_GR	69		//
#define LK_QFE			70		//
#define LK_LD_AVR		71		//
// new stuff -----------------------------------
#define LK_NEXT_GR		72		//
#define LK_FL			73		//
#define LK_TASK_DISTCOV		74		//
#define LK_ALTERNATESARRIV	75		// not a mistake
#define LK_ALTERN1_ARRIV	75		//
#define LK_ALTERN2_ARRIV	76		//
#define LK_BESTALTERN_ARRIV	77		//
#define LK_HOMERADIAL		78		//
#define LK_AIRSPACEDIST		79		//
#define LK_EXTBATTBANK		80		//
#define LK_EXTBATT1VOLT		81		//
#define LK_EXTBATT2VOLT		82		//
#define LK_ODOMETER		83		//
#define LK_AQNH			84		//
#define LK_AALTAGL		85		//
#define LK_HGPS			86
#define LK_EQMC			87		//

// quick usage values
#define LK_EXP1			88		//	--
#define LK_EXP2			89		//	--

// Non-infobox values - since 2.1 all values are no more available on Infoboxes (IBOX mode)

#define LK_OLC_CLASSIC_DIST		90		//  not for ibox 
#define LK_OLC_FAI_DIST			91		//  not for ibox 
#define LK_OLC_LEAGUE_DIST		92		//  not for ibox 
#define LK_OLC_3TPS_DIST		93		//  not for ibox 
#define LK_OLC_CLASSIC_PREDICTED_DIST	94		//  not for ibox 
#define LK_OLC_FAI_PREDICTED_DIST	95		//  not for ibox 
#define LK_OLC_3TPS_PREDICTED_DIST	96		//  not for ibox 

#define LK_OLC_CLASSIC_SPEED		97		//  not for ibox 
#define LK_OLC_FAI_SPEED		98		//  not for ibox 
#define LK_OLC_LEAGUE_SPEED		99		//  not for ibox 
#define LK_OLC_3TPS_SPEED		100		//  not for ibox 
#define LK_OLC_CLASSIC_PREDICTED_SPEED	101		//  not for ibox 
#define LK_OLC_FAI_PREDICTED_SPEED	102		//  not for ibox 
#define LK_OLC_3TPS_PREDICTED_SPEED	103		//  not for ibox 

#define LK_OLC_CLASSIC_SCORE		104		//  not for ibox 
#define LK_OLC_FAI_SCORE		105		//  not for ibox 
#define LK_OLC_LEAGUE_SCORE		106		//  not for ibox 
#define LK_OLC_3TPS_SCORE		107		//  not for ibox 
#define LK_OLC_CLASSIC_PREDICTED_SCORE	108		//  not for ibox 
#define LK_OLC_FAI_PREDICTED_SCORE	109		//  not for ibox 
#define LK_OLC_3TPS_PREDICTED_SCORE	110		//  not for ibox 

#define LK_OLC_PLUS_SCORE		111		//  not for ibox 
#define LK_OLC_PLUS_PREDICTED_SCORE	112		//  not for ibox 

// The following values are not available for custom configuration

#define LK_WIND			131		//
#define LK_FIN_ALTDIFF0		132		// final (task) altitude difference at MC=0
#define LK_LKFIN_ETE		133		// real ETE 
#define LK_NEXT_ALTDIFF0	134		// 
#define LK_TIME_LOCALSEC	135		// with seconds displayed
// Target infos
#define LK_TARGET_DIST		136		//
#define LK_TARGET_TO		137		//
#define LK_TARGET_BEARING	138		//
#define LK_TARGET_SPEED		139		//
#define LK_TARGET_ALT		140		//
#define LK_TARGET_ALTDIFF	141		//
#define LK_TARGET_VARIO		142		//
#define LK_TARGET_AVGVARIO	143		//
#define LK_TARGET_ALTARRIV	144		//
#define LK_TARGET_GR		145		//
#define LK_TARGET_EIAS		146		//
// Time gates
#define LK_START_DIST		147		//

// overtarget values  UNUSED 
// #define LK_ALT1_DIST		148		//
// #define LK_ALT2_DIST		149		//
// #define LK_BALT_DIST		150		//
// #define LK_ALT1_BRGDIFF		151		//
// #define LK_ALT2_BRGDIFF		152		//
// #define LK_BALT_BRGDIFF		153		//
// #define LK_LASTTHERMAL_DIST	154		//
// #define LK_LASTTHERMAL_BRGDIFF	155		//

// Service values
#define LK_DUMMY		253		//
#define LK_EMPTY		254		//
#define LK_ERROR		255		//

// GetMacCready
#define GMC_DEFAULT	0	// default behaviour

// Wind Calculator result codes
#define WCALC_INVALID_DATA	0
// Invalid data in input: speed and/or track samples not good
// Ground Speed not constant
#define WCALC_INVALID_SPEED	-1
// track speed not constant
#define WCALC_INVALID_TRACK	-2
// both GS and track invalid
#define WCALC_INVALID_ALL	-3
// Heading not within margin
#define WCALC_INVALID_HEADING	-4
// IAS not constant
#define WCALC_INVALID_IAS	-5
// IAS not within range
#define WCALC_INVALID_NOIAS	-6

// Size of really used portion of rotary buffer, truly reducing WCALC_ROTARYSIZE in Defines.h
#define WCALC_MAXSIZE		60
// Skip last x seconds when calculating wind from history
#define WCALC_TIMESKIP  1
// Go back x seconds for GPS GS and Track values from history, default setting
#define WCALC_TIMEBACK  10

// LK TRAFFIC Status flags
#define LKT_EMPTY	0
// anything seen witihin a live_minutes  ago
#define LKT_REAL	1
// last time seen was up to shadow_minutes ago
#define LKT_GHOST	2
// last time seen was up to ghost_minutes ago
#define LKT_ZOMBIE	3

#define LKT_TYPE_NONE	0
#define LKT_TYPE_MASTER	1
#define LKT_TYPE_SLAVE	2

// Waypoint formats. XCW is also DAT
// NEW for new waypoints to be saved on file1 with file1' format
#define LKW_NEW		0
#define LKW_DAT		1
#define LKW_XCW		1
#define LKW_CUP		2
#define LKW_COMPE	3
#define LKW_OZI		4
#define LKW_GPX		5
#define LKW_GARMIN	6
#define LKW_LK8000	7	// mixed format
#define LKW_VIRTUAL	8	// temporary, cannot save them

// ComPort diagnostics: ComPortStatus
#define CPS_UNUSED	0	// init to zero at startup
#define CPS_CLOSED	1	// closed by CloseAll
#define CPS_OPENKO	2 	// port could not be open
#define CPS_OPENOK	3 	// port open ok
#define CPS_EFRAME	4	// frame errors

// Overtargets Modes
#define OVT_TASK	0
#define OVT_BALT	1
#define OVT_ALT1	2
#define OVT_ALT2	3
#define OVT_HOME	4
#define OVT_THER	5	// last thermal
#define OVT_MATE	6	// team mate
#define OVT_FLARM	7	// target flarm in LK mode
#define OVT_MARK	8	// marked location
#define OVT_PASS	9	// closer mountain pass
// this is not the number of modes, because 0 is not accounted, remember
// Also, this is used for dimensioning array. 
#define OVT_MAXMODE	OVT_PASS
// This is used for limiting rotate
#define OVT_ROTATE	OVT_FLARM

// CustomKeys identifiers.. it was time to do it!
#define CKI_BOTTOMCENTER	0
#define CKI_BOTTOMLEFT		1
#define CKI_BOTTOMRIGHT		2
#define CKI_BOTTOMICON		3
#define CKI_TOPLEFT		4
#define CKI_TOPRIGHT		5

// LK8000 run modes, BYTE format 0x00 - 0xff
#define RUN_WELCOME		0x00
#define RUN_PROFILE		0x01
#define RUN_FLY			0x11
#define RUN_FLY_PRIMARY_UNIT	0x11
#define RUN_FLY_REPEATER	0x12
#define RUN_FLY_SECONDARY_UNIT	0x12
#define RUN_SIM			0x21
#define RUN_SIM_PRIMARY_UNIT	0x21
#define RUN_SIM_REPEATER	0x22
#define RUN_SIM_SECONDARY_UNIT	0x22
#define RUN_EXEC		0x04
#define RUN_PASSIVE		0x21
#define RUN_EXIT		0x08



#endif // LK8000_UTILS2_H_
