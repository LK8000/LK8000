/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Defines.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef LK8000_DEFINES_H
#define LK8000_DEFINES_H

#include <math.h>

// The StartupStore line separator, normally CRLF but previously it was only LF
#define SNEWLINE        "\r\n"
#define NEWLINE         _T(SNEWLINE)

/*
 * General defines for LK8000
 */
#define NMEA_REPLAY _T("NMEAReplay")

#define INVALID_GR	999
// max allowed for both signed and unsigned shorts
#define INVALID_VALUE	-1

// an invalid altitude difference identifier
#define INVALID_DIFF	-99999

// an invalid distance
#define INVALID_DIST	999999

/*
 * Over this GR value, alternates and Final GR are not showing numbers anymore and
 * the variable is set to INVALIDVALUE
 */
#define ALTERNATE_MAXVALIDGR MAXEFFICIENCYSHOW

// Below this altitude alt diffs are not shown
#define ALTDIFFLIMIT	(-3000*ALTITUDEMODIFY)


/*
 * Normally GlidePolar::bestld * 0.7 makes an efficiency 40 become 28 as a safety margin
 */
#define SAFELD_FACTOR	0.7
#define ALTERNATE_QUIETMARGIN 200 // meters - Quiet sound mode under safetyaltitude+QUIETMARGIN

/*
 * Additional height over arrival altitude needed for safety searches and green&blue Ok status
 * This is really needed since car GPS or non-baro GPS may be in error of several meters, and
 * we can't accept a +30m over safety if the gps altitude is wrong!!
 */
#define ALTERNATE_OVERSAFETY  100 // meters
#define ALTERNATE_MAXRANGE    100 // km - bestalternate search is within this maximum limit
#define DYNABOXTIME 3.0  // seconds between infobox dynamic flipping

/*
 * This is required because water labels (category 05 and 10 in topology) are special.
 * They are reset differently. This is the default in km zoom.
 */
#define DEFAULT_WATER_LABELS_THRESHOLD	10


/*
 * DOUBLECLICKINTERVAL is a general purpouse timing, used by both VK and synthetic double click
 *
 * No miracles. Couldn't do it any better.
 *
 * Max interval in ms between two clicks for getting a double click
 * Very careful changing this value. It is used by virtual keys also!
 * This is the timing sequence for virtual keys:
 *
 * 0 - VKSHORTCLICK   single click on the waypoint
 * 0 - DCI/2+30		double click detected and VK disabled
 * DCI/2+30 - DCI        airspace click and double click conflict?
 * <DCI			possible double click
 * >DCI 		virtual key
 *
 */

#define DOUBLECLICKINTERVAL 350
#define VKSHORTCLICK 120 // must be < than DCI/2 to have a chance to make airspace click recon!!!
#define VKLONGCLICK 1500  // triggers circling/cruis switch on aircraft icon
#define AIRSPACECLICK CustomKeyTime // interval to look only for airspace and not WP (IF NOT USING VK)
#define FASTDOUBLECLICK DOUBLECLICKINTERVAL/2-30
#define BESTALTERNATEINTERVAL 60.0 // interval in seconds between BA search (slow)


/*
 * Global defines
 */
#ifdef WIN32
#define DIRSEP          "\\"
#else
#define DIRSEP          "/"
#endif

#define LKFORK		"LK8000"
#define LKVERSION	"7"
#define LKRELEASE	"4.3"

#define LKDATADIR	"LK8000"
#define LKPROFILE	"DEFAULT_PROFILE.prf"
#define LKAIRCRAFT	"DEFAULT_AIRCRAFT.acf"
#define LKPILOT		"DEFAULT_PILOT.plt"
#define LKDEVICE	"DEFAULT_DEVICE.dvc"
#define LKSOUNDTABLE    "SOUND_TABLE.TXT"

/*
 * LK8000 directories
 */
#define LKD_HOME	LKDATADIR
#define LKD_LOGS	"_Logger"

#ifdef ANDROID
/*
 * system files are loaded from AssetManager, and '_' prefix are forbiden...
 *  for all file in this directory :
 *....- are inside apk bundle
 *    - must be loaded using zzip from native code or using java code.
 *    - are read only and hidden for users
  */
    #define LKD_SYSTEM	     "assets"
    #define LKD_SOUNDS	     "sounds" // bitmap are loaded from java ressouce
    #define LKD_BITMAPS      "bitmaps" // bitmap are loaded from java using AssetManager
    #define LKD_SYS_LANGUAGE "assets/language"
    #define LKD_SYS_POLAR    "assets/polars"

#else
    #define LKD_SYSTEM       "_System"
    #define LKD_SOUNDS       "_System" DIRSEP "_Sounds"
    #define LKD_BITMAPS      "_System" DIRSEP "_Bitmaps"

    #define LKD_SYS_LANGUAGE "_Language"
    #define LKD_SYS_POLAR    "_Polars"

#endif

#define LKD_DEFAULT_POLAR   "Default.plr"
#define LKD_DEFAULT_LANGUAGE   "en"

#define LKD_CONF	"_Configuration"
#define LKD_TASKS	"_Tasks"
#define LKD_WAYPOINTS	"_Waypoints"
#define LKD_POLARS	"_Polars"
#define LKD_AIRSPACES	"_Airspaces"
#define LKD_MAPS	"_Maps"
#define LKD_LANGUAGE	"_Language"

/*
 * LK8000 suffixes - Do not change them, uncompleted work 091001
 */
#define LKS_TSK		".lkt"
#define LKS_XCTSK   ".xctsk"
#define LKS_LOG		".log"
#define LKS_TXT		".txt"
#define LKS_PRF		".prf"
#define LKS_AIRCRAFT	".acf"
#define LKS_DEVICE	".dvc"
#define LKS_PILOT	".plt"
#define LKS_AIRSPACES	".txt"
#define LKS_POLARS	".plr"
#define LKS_WP_WINPILOT	".dat"
#define LKS_WP_XCSOAR	".xcw"
#define LKS_WP_CUP	".cup"
#define LKS_WP_COMPE	".wpt"
#define LKS_WP_GPX	".gpx"
#define XCS_MAPS	".xcm"
#define LKS_MAPS	".lkm"
#define LKS_TERRAINDAT	".dat"
#define LKS_TERRAINDEM	".dem"
#define LKS_TERRAINJP2	".jp2"
#define LKS_TOPOLOGY	".tpl"
#define LKS_AIRFIELDS	".txt"
#define LKS_LANGUAGE	".LNG"
#define LKS_INPUT	".xci"
#define LKS_IGC		".igc"
#define LKS_OPENAIP ".aip"

/*
 * LK8000 files (keep original suffixes)
 */
#define LKF_RECENTS	"History.txt"
#define LKF_DEFAULTASK	"Default.lkt"
#define LKF_FLARMIDS	"IDFLARM.TXT"
#define LKF_RUNLOG	"RUNTIME.log"
#define LKF_FAILLOG	"FAILURES.log"
#define LKF_AIRFIELDS	"WAYNOTES.TXT"
#define LKF_DEBUG	"DEBUG.log"
#define LKF_PERSIST	"Persist.log"
#define LKF_FLARMNET	"FLARMNET.FLN"
#define LKF_CHECKLIST	"NOTEPAD.TXT"
#define LKF_CREDITS	"CREDITS.TXT"
#define LKF_LOGBOOKTXT	"LOGBOOK.TXT"
#define LKF_LOGBOOKCSV	"LOGBOOK.CSV"
#define LKF_LOGBOOKLST	"LOGBOOK.LST"
#define LKF_WAYPOINTS1	"waypoints1.dat"
#define LKF_WAYPOINTS2	"waypoints2.dat"
#define LKF_AIRSPACES	"airspace.txt"
#define LKF_AIRSPACE_SETTINGS "_Airspaces\\AspConfig.LK"
#define LKF_LASTVERSION "LASTINFO.TXT"


// Rotary buffers and filters, except TrueWind

#define MAXITERFILTER 10 // Max number of iterations during a filter convergence search
			 // just for safety, it should stop at 3-4 automatically

// Size of internal rotary buffer for TrueWind
#define WCALC_ROTARYSIZE        90

#define MAXLDROTARYSIZE 180 // max size of rotary buffer for LD calculation

#define MAXEFFICIENCYSHOW 200  // over this, show INVALID_GR


// Optimization preprocessing for LK8000:  We want ALL landables within dst range.
// Distance is in Km possibly GREATER THAN DSTRANGETURNPOINT but not needed really.
// The RangeWaypoint calculation will reduct the DST values to fit waypoints within range
// inside MAXRANGE array, and it will grow the DST when space is available.
#define MAXRANGELANDABLE        500
#define DSTRANGELANDABLE        150
// Same for nearest turnpoints .
#define MAXRANGETURNPOINT	500
#define DSTRANGETURNPOINT	100

// How many thermals we shall remember
#define MAX_THERMAL_HISTORY	100

// Nearest calculations are made on this list
// if we enlarge, resize also MAXNUMPAGES
#define MAXNEAREST		50
// Commons are both 1 page of commons and HISTORY as well! HISTORY is sized MAXCOMMON!
#define MAXCOMMON		MAXNEAREST
#define MAXTRAFFIC		FLARM_MAX_TRAFFIC
// Max number of airspaces handled by 2.4 nearest airspace page. Basically, the 50 nearest.
#define MAXNEARAIRSPACES	MAXNEAREST
// Max number of thermals in the Nearest Thermals page (out of MAX_THERMAL_HISTORY, normally much larger)
#define MAXTHISTORY		MAXNEAREST

// Max number of pages in mapspacemode.
// Large enough to contain MAXNEAREST/numrows (numraws min 5 on some devices)
// Counting only real pages, 1-x , add 1 for safety
// It only matters dimensional space, so you can enlarge at a memory cost.
// If it is too low, visible waypoints will be limited to this number automatically.
#define MAXNUMPAGES		9	// the nearest
#define MAXCOMMONNUMPAGES	MAXNUMPAGES
#define MAXTRAFFICNUMPAGES	MAXNUMPAGES
#define MAXAIRSPACENUMPAGES	MAXNUMPAGES
#define MAXTHISTORYNUMPAGES	MAXNUMPAGES

// Nearest Update time: wait for some seconds before updating nearest pages with
// new calculations. 5 seconds is far enough, could even be more..
#define NEARESTUPDATETIME	5.0	// seconds, double

// If a user has pressed up or down in nearest/common/etc pages, let this time pass
// before a DoCommon DoNearest etc. are performed again, in order to avoid selection on
// screen changes (for example, if sorted by direction..).
#define PAGINGTIMEOUT		2.0	// seconds, double

// Nearest hold time: when asking for example waypoint details, put the nearest on hold for..
#define	NEARESTONHOLD		60.0	// seconds, double

// Number of background colors available in MapWindow.h for NON-terrain maps
#define LKMAXBACKGROUNDS        10

// Number of color ramp available for draw terrain
#define NUMRAMPS        18

// Task format version
#define LKTASKVERSION	'4'
// How many chars at the beginning of file are reserved
#define LKPREAMBOLSIZE	50

// 101003 SetRxtimeout default, in ms
#define RXTIMEOUT	10


#define SIMMODE	(RUN_MODE==RUN_SIM)

#define CUPSIZE_COUNTRY	10
#define CUPSIZE_CODE	15
#define CUPSIZE_FREQ	15

// Altitude arrival calculation types
#define ALTA_MC		0	// Altitude arrival at current MC
#define ALTA_MC0	1	// Altitude arrival at MC=0
#define ALTA_SMC	2	// Altitude arrival at safety MC
#define ALTA_AVEFF	3	// Altitude arrival at current average efficiency
#define ALTA_TOP	3
#define ALTA_SIZE	4

// Max number of existing LK Alarms
#define MAXLKALARMS	3
#define MAXLKALARMSTRIGGERS	30	// max number of triggered events per alarm
#define LKALARMSINTERVAL	60	// seconds of interval for LK alarms

// Analysys pages can be called directly
#define ANALYSYS_PAGE_DEFAULT	  ANALYSIS_PAGE_BAROGRAPH
#define ANALYSIS_PAGE_BAROGRAPH    0
#define ANALYSIS_PAGE_CLIMB        1
#define ANALYSIS_PAGE_TASK_SPEED   2
#define ANALYSIS_PAGE_WIND         3
#define ANALYSIS_PAGE_POLAR        4
#define ANALYSIS_PAGE_TEMPTRACE    5
#define ANALYSIS_PAGE_TASK         6
#define ANALYSIS_PAGE_CONTEST      7

#define MAXPAGE                    ANALYSIS_PAGE_CONTEST /* 8 */

#define FAI_MIN_DISTANCE_THRESHOLD 5000
#define FAI_BIG_THRESHOLD	       500000  /* FAI distance threshold                   */
#define FAI_NORMAL_PERCENTAGE	   0.28    /* min % of FAI triangle if distance < 750km */
#define FAI_BIG_PERCENTAGE         0.25    /* min % of FAI triangle if distance >= 750km */
#define FAI_BIG_MAX_PERCENTAGE     0.45    /* min % of FAI triangle if distance >= 750km */
#define FAI_SECTOR_COLOR           LKColor(0x50,0x0A,0x8B)

#define D_AUTOWIND_MANUAL	0	// totally manual
#define D_AUTOWIND_CIRCLING	1	// while circling
#define D_AUTOWIND_ZIGZAG	2	// while zigzagging with a TAS available
#define D_AUTOWIND_BOTHCIRCZAG	3	// circling + zigzag
#define D_AUTOWIND_EXTERNAL	4	// from an external instrument only

// Polar weights index
#define WEIGHT_PILOT    0
#define WEIGHT_PLANEDRY 1
#define WEIGHT_WATER    2



// WriteText modalities
#define WTALIGN_LEFT	0
#define WTALIGN_RIGHT	1
#define WTALIGN_CENTER	2
// modes, currently a bool!
#define WTMODE_NORMAL	0
#define WTMODE_OUTLINED	1


// Waypoint TYPE definition
#define WPT_UNKNOWN		0
#define WPT_AIRPORT		1
#define WPT_OUTLANDING		2
#define WPT_TURNPOINT		3

// Number of reserved waypoints at the beginning of WP struct.
// All loops with NumberOfWayPoints should start from NUMRESWP in place of the old 0
// Position 0 is takeoff, etc
// moving waypoints will have assigned fixed slots and positions
#define NUMRESMARKERS		10
//#define NUMRESWP		(10+NUMRESMARKERS)

// Reserved Waypoints positions. Adjust also NUMRESWP!
#define RESWP_INVALIDNUMBER	1.23	// an invalid number for latitude, longitude, altitude etc.
#define RESWP_TAKEOFF		0
#define RESWP_TAKEOFF_NAME		"_@M1316_"			// LKTOKEN _@M1316_ "TAKEOFF"
#define RESWP_LASTTHERMAL	1
#define RESWP_LASTTHERMAL_NAME	"_@M1317_"			// LKTOKEN _@M1317_ "THERMAL"
#define RESWP_TEAMMATE		2
#define RESWP_TEAMMATE_NAME		"_@M1318_"			// LKTOKEN _@M1318_ "TEAMMATE"
#define RESWP_FLARMTARGET	3
#define RESWP_FLARMTARGET_NAME	"_@M1319_"			// LKTOKEN _@M1319_ "TARGET"
#define RESWP_OPTIMIZED		4	// optimized virtual waypoint as target turnpoint for overlays
#define RESWP_FAIOPTIMIZED	5
#define RESWP_FAIOPTIMIZED_NAME "_@M1813_"	// FAI OPTIMIZED
#define RESWP_FREEFLY		6
#define RESWP_FREEFLY_NAME	"_@M1814_"	// FREEFLY
#define RESWP_PANPOS		7
#define RESWP_PANPOS_NAME	"temporary"	// panning center position
#define RESWP_UNUSED		8
#define RESWP_UNUSED_NAME	"_@M1815_"	// unused
#define RESWP_EXT_TARGET	9

#define RESWP_EXT_TARGET_NAME	"_@M2469_"	// Task Waypoint from external device
#define RESWP_FIRST_MARKER	10
#define NUMRESWP		(RESWP_FIRST_MARKER+NUMRESMARKERS)
#define RESWP_LAST_MARKER	(RESWP_FIRST_MARKER+NUMRESMARKERS-1)	// 18
#define RESWP_END		RESWP_LAST_MARKER
// WayPointList .Number int identifier
#define RESWP_ID		9999


// These are used by LK functions, there is no overflow check so caution
// generic text buffer
#define LKSIZETEXT		50
// Generic Tchar buffer
#define LKSIZEBUFFER		50
// Tchar buffer used for messages
#define LKSIZEBUFFERLARGE	100

#define LKSIZEBUFFERTITLE	30
#define LKSIZEBUFFERUNIT	15
#define LKSIZEBUFFERVALUE	50

// size of a path, complete with final file name
#define LKSIZEBUFFERPATH	150

// size of nmea string max for logging
#define LKSIZENMEA		300

// BottomModes aka navboxes
// order is important, and thermal that MUST stay in position 0. Changing order will change sequence.
// Warning, these names have been renamed in MapWindow3, for example BM_AUX appears on screen as NAV2
// Do not confuse these indicators with screen overlay near map zoom
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
// these are limiters
#define BM_FIRST	0
#define BM_LAST		9

// BottomBar Switching mode
#define BBSM_MANUAL           0
#define BBSM_AUTO_THERMALLING 1
#define BBSM_FULL_AUTO        2


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
#define MSM_AIRSPACES		16
#define MSM_THERMALS		17
#define MSM_MAPRADAR		18	// this is multimapped
#define MSM_MAPTRK		19	// this is multimapped
#define MSM_MAPWPT		20	// this is multimapped
#define MSM_MAPASP		21	// this is multimapped
#define MSM_VISUALGLIDE		22	// multimapped, work in progress
#define MSM_MAPTEST		23	// multimapped, for testing purposes
#define MSM_INFO_HSI		24
// turnaround point is TOP
// remember that arrays must count from zero, so MSM_TOP+1
#define MSM_TOP			24
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
#define MP_MAPTRK		2
#define MP_MAPWPT		3
#define MP_MAPASP		4
#define MP_VISUALGLIDE		5
#define MP_RADAR		6
#define MP_TEST			7
#define MP_TOP			7

// M1 M2 M3 M4, not test and radar that are custom.
// All multimaps that need to have shared Display parameters
// Currently they are: MAPTRK, MAPWPT, MAPASP, VISUALGLIDE
#define NUMBER_OF_SHARED_MULTIMAPS      4
// We can disable some of these in LKInit, and skip them in rotation if we want.
#define NUMBER_OF_MULTIMAPS     MP_TOP


//
// WP mode
//
#define WP_AIRPORTS		0
#define WP_LANDABLE		1
#define WP_NEARTPS		2
#define WP_AIRSPACES		3
#define WP_TOP			3
//
// InfoMode
//
#define IM_CRUISE		0
#define IM_THERMAL		1
#define IM_TASK			2
#define IM_AUX			3
#define IM_CONTEST		4
#define IM_TRI			5
#define IM_HSI			6
#define IM_TOP			6	// THIS IS THE ABSOLUTE MAX NUMBER OF PAGES IN ALL MODES, ALSO!
//
// Navigation mode
//
#define NV_COMMONS		0
#define NV_HISTORY		1
#define NV_THERMALS		2
#define NV_TOP			2
//
// Traffic mode
//
#define TF_LIST			0
#define IM_TRF			1
#define IM_TARGET		2
#define TF_TOP			2

//
// How many sort boxes in nearest pages we can have, on the top line, normally 0-4 plus 1 spare
#define MAXSORTBOXES		6

//
// Events generated by virtual keys, processed in batch mode
//
#define LKEVENT_NONE		0
#define LKEVENT_ENTER		1
#define LKEVENT_PAGEUP		2
#define LKEVENT_PAGEDOWN	3
#define LKEVENT_UP		4
#define LKEVENT_DOWN		5
#define LKEVENT_TOPLEFT		6
#define LKEVENT_TOPRIGHT	7
//
#define LKEVENT_NEWRUN		9
#define LKEVENT_NEWPAGE		10	// this is for both up and now, out of multimap
#define LKEVENT_LONGCLICK	11
#define LKEVENT_SHORTCLICK	12	// Only in multimaps, as of 121009

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
#define LK_RESERVED1		19		// AVAILABLE FOR A NEW INFOBOX
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
#define LK_MTG_BRG		38		//
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
#define LK_TEAM_CODE		55		//
#define LK_TEAM_BRG		56		//
#define LK_TEAM_BRGDIFF		57		//
#define LK_TEAM_DIST		58		//
#define LK_SPEEDTASK_INST	59		//
#define LK_HOME_DIST		60		//
#define LK_SPEEDTASK_ACH	61		//
#define LK_AA_DELTATIME		62		//
#define LK_TC_ALL		63		// Average of all thermals so far
#define LK_LOGGER		64		//
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
#define LK_AIRSPACEHDIST	79		//
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
#define LK_FLAPS			113   		// not for ibox
#define LK_AIRSPACEVDIST		114		// not for ibox

#define LK_HOME_ARRIVAL			115

#define LK_ALTERNATESBRG		116		// not a mistake
#define LK_ALTERN1_BRG			116
#define LK_ALTERN2_BRG			117
#define LK_BESTALTERN_BRG		118
#define LK_ALTERNATESDIST		119		// not a mistake
#define LK_ALTERN1_DIST			119
#define LK_ALTERN2_DIST			120
#define LK_BESTALTERN_DIST		121

#define LK_MAXALT			122
#define LK_MAXHGAINED			123
#define LK_HEADWINDSPEED		124


#define LK_OLC_FAI_CLOSE			125
#define LK_OLC_FAI_CLOSE_PERCENT	126
#define LK_BANK_ANGLE                   127
#define LK_ALTERN1_RAD                  128
#define LK_ALTERN2_RAD                  129
#define LK_HEADING                      130
#define LK_ALTERN1_DISTNM		131
#define LK_ALTERN2_DISTNM		132

#define LK_SPEED_ME      133		// Maximum efficiency speed to fly
#define LK_TARGET_RE     134		// Target Req. Efficicency
#define LK_QNE           135		// Barometric QNE Altitude
#define LK_NEXT_DIST_RADIUS   136	  // Distance to Next turnpoint minus turnpoint cylindre radius (center for line and sector turnpoint)


#define LK_XC_FF_DIST       137     // Additional Contest FREE Flight distance
#define LK_XC_FF_SCORE      138     // Additional Contest FREE Flight score
#define LK_XC_FT_DIST       139     // Additional Contest FREE Triangle distance
#define LK_XC_FT_SCORE      140     // Additional Contest FREE Triangle score
#define LK_XC_FAI_DIST      141     // Additional Contest FAI Triangle distance
#define LK_XC_FAI_SCORE     142     // Additional Contest FAI Triangle score
#define LK_XC_DIST          143     // Additional Contest combined distance
#define LK_XC_SCORE         144     // Additional Contest combined score
#define LK_XC_CLOSURE_DIST       145     // Additional Contest combined closure distance
#define LK_XC_CLOSURE_PERC       146     // Additional Contest combined closure %
#define LK_XC_PREDICTED_DIST     147     // Additional Contest combined closure distance
#define LK_XC_MEAN_SPEED     148     // Additional Contest mean speed
#define LK_TIMETASK          149     // elapsed time since task start
#define LK_MTG_QNH_ARRIV   150 // QNH arrival at Multi Target selected
#define LK_MTG_BRG_DIFF             151  // Multitarget Bearing difference
#define LK_ALTERN1_QNH_ARRIV   152 // QNH arrival at Alternate 1
#define LK_HOME_GR             153 // required efficency to home 
#define LK_HOME_DISTNM         154 // distance to home nm 
#define LK_HOME_BRG            155 // bearing to home 
#define LK_XC_TYPE            156 // type of best result in XC 

// The following values are not available for custom configuration

#define LK_WIND			    233		//
#define LK_FIN_ALTDIFF0		234		// final (task) altitude difference at MC=0
#define LK_LKFIN_ETE		235		// real ETE
#define LK_NEXT_ALTDIFF0	236		//
#define LK_TIME_LOCALSEC	237		// with seconds displayed
// Target infos
#define LK_TARGET_DIST		238		//
#define LK_TARGET_TO		239		//
#define LK_TARGET_BEARING	240		//
#define LK_TARGET_SPEED		241		//
#define LK_TARGET_ALT		242		//
#define LK_TARGET_ALTDIFF	243		//
#define LK_TARGET_VARIO		244		//
#define LK_TARGET_AVGVARIO	245		//
#define LK_TARGET_ALTARRIV	246		//
#define LK_TARGET_GR		247		//
#define LK_TARGET_EIAS		248		//
// Time gates
#define LK_START_DIST		249		//

#define LK_NEXT_CENTER_ALTDIFF 250  // Same As  LK_NEXT_ALTDIFF but always with Waypoint center
#define LK_NEXT_CENTER_GR	251	// Same as LK_NEXT_GR but always with Waypoint center
#define LK_START_SPEED		252	// Requiered speed for reach Start of task at Gate Time !
#define LK_START_ALT        253 // 
#define LK_SAT              254
#define LK_HBAR_AVAILABLE   255
#define LK_CPU              256
#define LK_TRIP_SPEED_AVG   257
#define LK_TRIP_STEADY      258
#define LK_TRIP_TOTAL       259
#define LK_SPEED_AVG        260
#define LK_DIST_AVG         261

// Service values
#define LK_DUMMY		262		//
#define LK_EMPTY		263		//
#define LK_ERROR		264		//

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
#define LKW_OPENAIP 9   // OpenAIP format

// ComPort diagnostics: ComPortStatus
#define CPS_UNUSED	0	// init to zero at startup
#define CPS_CLOSED	1	// closed by CloseAll
#define CPS_OPENKO	2 	// port could not be open
#define CPS_OPENOK	3 	// port open ok
#define CPS_EFRAME	4	// frame errors
#define CPS_OPENWAIT    5 	// port open ok but wait for connected device

// Overtargets Modes
#define OVT_TASK        0
#define OVT_TASKCENTER  1   // Active Waypoint Center for PG optimized Task
#define OVT_BALT        2
#define OVT_ALT1        3
#define OVT_ALT2        4
#define OVT_HOME        5
#define OVT_THER        6	// last thermal
#define OVT_MATE        7	// team mate
#define OVT_XC          8	// target XC Closing triangle point
#define OVT_FLARM       9	// target flarm in LK mode
#define OVT_MARK        10	// marked location
#define OVT_PASS        11	// closer mountain pass
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
#define CKI_CENTERSCREEN	6

// LK8000 run modes, BYTE format 0x00 - 0xff
#define RUN_WELCOME		0x00
#define RUN_DUALPROF		0x01
#define RUN_PROFILE		0x02
#define RUN_AIRCRAFT		0x03
#define RUN_PILOT		0x04
#define RUN_DEVICE		0x05
#define RUN_EXIT		0x08
#define RUN_SHUTDOWN		0x09
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

#define LKINFOFONT      LK8SmallFont            // was InfoWindowFont
// km for distance, kmh for speed etc.  in map overlay

#define MAXFONTRESIZE 10 // -10 .. -5 -4 -3 -2 -1 [0] +1 +2 +3 +4 +5 .. +10
#define MAXSNAILRESIZE 10 // -10 .. -5 -4 -3 -2 -1 [0] +1 +2 +3 +4 +5 .. +10

// McReady

#define MAXSAFETYSPEED  100     // 360kmh
#define MAXSPEED        100     // 360kmh = 100ms
#define MAX_FLAPS	10	// max flaps positions count

//
// Math
//
#ifndef PI
static const double PI = (4*atan(1));
#endif
#define EARTH_DIAMETER		12733426.0		// Diameter of earth in meters
#define SQUARED_EARTH_DIAMETER	162140137697476.0	// Diameter of earth in meters (EARTH_DIAMETER*EARTH_DIAMETER)
#ifndef DEG_TO_RAD
#define DEG_TO_RAD  (PI / 180)
#define RAD_TO_DEG  (180 / PI)
#endif

#define NAUTICALMILESTOMETRES (double)1851.96
#define KNOTSTOMETRESSECONDS (double)0.5144

#define TOKNOTS (double)1.944
#define TOFEETPERMINUTE (double)196.9
#define TOMPH   (double)2.237
#define TOKPH   (double)3.6

// meters to.. conversion
#define TONAUTICALMILES (1.0 / 1852.0)
#define TOMILES         (1.0 / 1609.344)
#define TOKILOMETER     (0.001)
#define TOFEET          (1.0 / 0.3048)
#define TOMETER         (1.0)

// Pressures
#define PRESSURE_STANDARD	1013.25		// hPa


//
// MAP LABELS ROUND ROBIN CHOICES
//
#define MAPLABELS_START         0       // mark limit
#define MAPLABELS_ALLON         0
#define MAPLABELS_ONLYWPS       1
#define MAPLABELS_ONLYTOPO      2
#define MAPLABELS_ALLOFF        3
#define MAPLABELS_END           3       // mark limit



//
// TextInBox and LKDrawWaypoint
#define WPCIRCLESIZE        2


//
// SHORTCUTS
//

#define ISGLIDER (AircraftCategory == (AircraftCategory_t)umGlider)
#define ISPARAGLIDER (AircraftCategory == (AircraftCategory_t)umParaglider)
#define ISCAR (AircraftCategory == (AircraftCategory_t)umCar)
#define ISGAAIRCRAFT (AircraftCategory == (AircraftCategory_t)umGAaircraft)

#define CURTYPE ModeType[ModeIndex]
// CURMODE is the MSM_xxx page, independent from Type, any place. Example: check if we are in MSM_TRAFFIC
// Once set, MapSpaceMode becomes CURMODE
#define CURMODE ModeTable[ModeIndex][CURTYPE]
#define INVERTCOLORS  (Appearance.InverseInfoBox)
#define TASKINDEX       Task[ActiveTaskPoint].Index

#define ACTIVE_WP_IS_AAT_AREA (UseAATTarget() && (ActiveTaskPoint > 0) \
                              && ValidTaskPoint(ActiveTaskPoint + 1))

#define DONTDRAWTHEMAP  (!MapWindow::mode.AnyPan()&&MapSpaceMode!=MSM_MAP)
#define MAPMODE8000     (!MapWindow::mode.AnyPan()&&MapSpaceMode==MSM_MAP)
#define QUICKDRAW	(FastZoom || MapWindow::zoom.BigZoom())
#define NOTANYPAN	(!MapWindow::mode.AnyPan())
#define INPAN		(MapWindow::mode.Is(MapWindow::Mode::MODE_PAN))

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef NO_AS_FILES
#define NO_AS_FILES 9
#endif
#ifndef NO_WP_FILES
#define NO_WP_FILES 9
#endif


#endif
