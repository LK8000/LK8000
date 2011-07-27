/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Defines.h,v 8.2 2010/12/15 12:27:55 root Exp root $
*/

#ifndef LK8000_DEFINES_H
#define LK8000_DEFINES_H

// The StartupStore line separator, normally CRLF but previously it was only LF
#define SNEWLINE        "\r\n"
#define NEWLINE         _T(SNEWLINE)

//#ifdef PPC2002
//#define LKFONT_QUALITY	ANTIALIASED_QUALITY
//#else
//#define LKFONT_QUALITY	CLEARTYPE_COMPAT_QUALITY
//#endif

/* 
 * General defines for LK8000
 */

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
#define AIRSPACECLICK 1000 // interval to look only for airspace and not WP (IF NOT USING VK)


#define BESTALTERNATEINTERVAL 60.0 // interval in seconds between BA search (slow)

/*
 * Global defines
 */

#define LKNAME		"XCSoar"
#define LKFORK		"LK8000"
#define LKVERSION	"2"
#define LKRELEASE	"2e"

#define REGKEYNAME	"Software\\COOL\\LK8000"
#define XCSDATADIR	"LK8000"
#define XCSPROFILE	"DEFAULT_PROFILE.prf"

/*
 * LK8000 directories
 */
#define LKD_HOME	XCSDATADIR
#define LKD_LOGS	"_Logger"
#define LKD_SYSTEM	"_System"
#define LKD_SOUNDS	"_System\\_Sounds"
#define LKD_BITMAPS	"_System\\_Bitmaps"
#define LKD_CONF	"_Configuration"
#define LKD_TASKS	"_Tasks"
#define LKD_WAYPOINTS	"_Waypoints"
#define LKD_POLARS	"_Polars"
#define LKD_AIRSPACES	"_Airspaces"
#define LKD_MAPS	"_Maps"
#define LKD_LANGUAGE	"_Language"
// #define LKD_TEMP	"_Tmp"
// #define LKD_DEBUG	"_Debug"

/*
 * LK8000 suffixes - Do not change them, uncompleted work 091001
 */
#define LKS_TSK		".tsk"
#define LKS_LOG		".log"
#define LKS_TXT		".txt"
#define LKS_PRF		".prf"
#define LKS_AIRSPACES	".txt"
#define LKS_POLARS	".plr"
#define LKS_WP_WINPILOT	".dat"
#define LKS_WP_XCSOAR	".xcw"
#define LKS_WP_CUP	".cup"
#define LKS_WP_COMPE	".wpt"
#define XCS_MAPS	".xcm"
#define LKS_MAPS	".lkm"
#define LKS_TERRAINDAT	".dat"
#define LKS_TERRAINDEM	".dem"
#define LKS_TERRAINJP2	".jp2"
#define LKS_TOPOLOGY	".tpl"
#define LKS_AIRFIELDS	".txt"
#define LKS_LANGUAGE	".LNG"
#define LKS_STATUS	".xcs"
#define LKS_INPUT	".xci"
#define LKS_IGC		".igc"


/*
 * LK8000 files (keep original suffixes)
 */
#define LKF_RECENTS	"History.txt"
#define LKF_DEFAULTASK	"Default.tsk"
#define LKF_MARKS	"Markers.txt"
#define LKF_SMARKS	"Markers"
#define LKF_FLARMIDS	"IDFLARM.txt"
#define LKF_RUNLOG	"RUNTIME.log"
#define LKF_FAILLOG	"FAILURES.log"
#define LKF_AIRFIELDS	"WAYNOTES.txt"
#define LKF_DEBUG	"DEBUG.log"
#define LKF_PERSIST	"Persist.log"
#define LKF_FLARMNET	"flarmnet.fln"
#define LKF_CHECKLIST	"NOTEPAD.txt"
#define LKF_WAYPOINTS1	"waypoints1.dat"
#define LKF_WAYPOINTS2	"waypoints2.dat"
#define LKF_AIRSPACES	"airspace.txt"


// Rotary buffers and filters, except TrueWind

#define MAXITERFILTER 10 // Max number of iterations during a filter convergence search
			 // just for safety, it should stop at 3-4 automatically

// Size of internal rotary buffer for TrueWind
#define WCALC_ROTARYSIZE        90

#define MAXLDROTARYSIZE 180 // max size of rotary buffer for LD calculation

#define MAXEFFICIENCYSHOW 200  // over this, show INVALID_GR

#endif

// Optimization preprocessing for LK8000:  We want ALL landables within dst range.
// Distance is in Km possibly GREATER THAN DSTRANGETURNPOINT but not needed really.
#define MAXRANGELANDABLE        500
#define DSTRANGELANDABLE        150
// Same for nearest turnpoints . Since tps are much more than landables, lets try to reduce the number
// by selecting only those within 75km. It is critical that the number of found tps within 75km is
// below 500 !
#define MAXRANGETURNPOINT	500
#define DSTRANGETURNPOINT	75

// Nearest calculations are made on this list
// if we enlarge, resize also MAXNUMPAGES
#define MAXNEAREST		50
// Commons are both 1 page of commons and HISTORY as well! HISTORY is sized MAXCOMMON!
#define MAXCOMMON		50
#define MAXTRAFFIC		FLARM_MAX_TRAFFIC

// Max number of pages in mapspacemode. 
// Large enough to contain MAXNEAREST/numrows (numraws min 5 on some devices)
// Counting only real pages, 1-x , add 1 for safety
// It only matters dimensional space, so you can enlarge at a memory cost. 
// If it is too low, visible waypoints will be limited to this number automatically.
#define MAXNUMPAGES		9	// the nearest
#define MAXCOMMONNUMPAGES	9
#define MAXTRAFFICNUMPAGES	9

// Nearest Update time: wait for some seconds before updating nearest pages with
// new calculations. 5 seconds is far enough, could even be more..
#define NEARESTUPDATETIME	5.0	// seconds, double

// If a user has pressed up or down in nearest/common/etc pages, let this time pass
// before a DoCommon DoNearest etc. are performed again, in order to avoid selection on
// screen changes (for example, if sorted by direction..).
#define PAGINGTIMEOUT		2.0	// seconds, double

// Nearest hold time: when asking for example waypoint details, put the nearest on hold for..
#define	NEARESTONHOLD		60.0	// seconds, double

// Number of reserved waypoints at the beginning of WP struct. 
// All loops with NumberOfWayPoints should start from NUMRESWP in place of the old 0
// Position 0 is takeoff
// Position 1 is for testing
// moving waypoints will have assigned fixed slots and positions
// LKUtils.h defines contents for RESWP_ 
#define NUMRESWP		4

// Number of background colors available in MapWindow.h for NON-terrain maps
#define LKMAXBACKGROUNDS        10

// Task format version
#define LKTASKVERSION	'3'
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
