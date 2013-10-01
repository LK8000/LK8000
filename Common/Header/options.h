/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: options.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/
#ifndef OPTIONS_H
#define OPTIONS_H


// define this to be true for windows PC port
#if !defined(WINDOWSPC)
#define   WINDOWSPC             0
#endif

/////////////////////////////////////////////////////////
// This stuff need checking. At 110614 unknown if working
#if (WINDOWSPC>0)
#if _DEBUG
// leak checking
#define CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif
/////////////////////////////////////////////////////////

#include "compatibility.h"


// Disable internally generated sounds
// #define DISABLEAUDIO

#ifdef PNA
#define NOLINETO
#endif

 // CACHE CALCULATIONS: IT IS IMPERATIVE THAT THIS OPTION CAN BE DISABLED ANYTIME!
 #define LK_CACHECALC 1
 // Notice:  These are only used if above is active!
 // MacCreadyAltitude and its statistics
 #define LK_CACHECALC_MCA 60
 // #define LK_CACHECALC_MCA_STAT 1

 //
 // TEMPORARY FIXES THAT REQUIRE EXTENSIVE TESTING - KEEP #ifdef until expiring date
 // When expire date is reached, unnecessary old stuff can be removed, even if commented
 //

 #define BUGSTOP	1	// STOP by LKASSERT even if the condition was managed, to show
				// the problem during alpha stages. 
				// Normally these conditions should not exist

 #define TESTBENCH	1	// THIS IS USING MORE MESSAGES, DOING MORE CHECKS, AND IT IS GOOD 
				// FOR DEVELOPMENT VERSIONS. CAN BE USED FOR SPECIAL BETAs,
				// but normally should not be used for public releases.

 #define USELKASSERT	1	// Enable LKASSERT checks and diagnostic messages at runtime.
				// For TESTBENCH and beta versions this should be always ENABLED.
				// Only in official stable versions it should be disabled.


 #if TESTBENCH
 #define CPUSTATS		1	// Show cpu stats in inverted text mode inside map mode
 #endif

 #if (WINDOWSPC>0)
 #define WINE
 #endif

#define PICTORIALS            // enables pictorials in dialogs

#define MOVE_WP_PAN           // moving a task waypoint in PAN mode
#define GEAR_WARNING          // Gear warning switch
#define MAX_NO_GEAR_WARN 10   // max. numbers of Gerwarnings before disabled
//#define GOTO_AS_SIMPLETASK  // even a singel goto will be listed in multiselect
#define BEARING_INDICATOR             // show QDM arrow in pictorials

#ifdef PICTORIALS
  #define WAYPOINT_ICONS   // waypoint ICONS in waypoint select dialog
  #define WAYPOINT_QUICK_PICTO
  #define MULTISEL_PICTORIALS
  #define BACKGROUND_PICTORIAL
  #define ASP_WARNING_PICTO
#endif

#define SHOW_FOUND_WAYPOINT    // show best found waypoint on active keyboard
#define SELECT_FOUND           // preselect best fit waypoint in waypoint list.

/*
 * MULTISELECT OPTIONS 
 */
//
// Team, Own and Oracle are introducing buttons for actions inside dialog.
// They must be first enabled with BUTTONS_MS
//
//#define BUTTONS_MS		// this is REQUIRED to enable one of the following:
//#define TEAM_CODE_MS		// button to trigger team code
//#define OWN_POS_MS		// button to trigger basic settings
//#define ORACLE_MS		// button to trigger oracle

//Keep the dialog list updated every second
#define AUTOUPDATE_MS

// Feedback is giving acoustic sound when long click has triggered the multiselection.
// It is introducing a TIMER in even processed inside MapWndProc.
//#define LONGCLICK_FEEDBACK

/* END OF MULTISELECT OPTIONS */


//#define OUTLINE_2ND    // double outline for airspaces

// #define LKCOMPETITION	1	// Will disable TRI 

// This stuff will be permanently removed shortly
// #define JP2000		1	// use JPG2000 terrain files .jp2 and jasper library
#define LKMTERRAIN		1	// allow DEM terrain inside dat and lkm topology files


// Draws a glide terrain line around the next (active) task waypoint.
// Includes addition of MSG tokens "Line+NextWP" & "Shade+NextWP"
// and change in HELP message 194 (for "Glide Terrain Line" setting).
// Eric Carden, September 13, 2012
#define GTL2 

// Activate FastZoom and QUICKDRAW conditions, for fast paint of map the first time after zoom request
#define USEBIGZOOM	1	


// do not add screen points if closer than 5pix to the previous
//#define LKASP_REMOVE_NEAR_POINTS		1

// MapDraw OPTIM , SetTopologyBounds limited to zoom change and map draw out of current rect 
#define TOPOBOUND_OPTIM	1

// Limit some functions in Draw thread to run only at 1hz max.
// Otherwise they are called each time the Thread_Draw is looping again.
#define USEONEHZLIMITER 1


// ----------------------------------------------------------------------------------------
//
// OPTIONALs not in use within official LK versions, but still available for custom version
//
// ----------------------------------------------------------------------------------------

// Use F Record in IGC log files- not needed really
// #define LOGFRECORD	1

// Bottombar TRM0 automatically triggered upon entering a thermal.
// This will be soon removed entirely - 121002
// #define AUTO_BBTRM		1

// Modify best cruise track calculation to assume goal arrival
// altitude of safety altitude (not current flight altitude).
// Eric Carden, April 21, 2012
// #define BCT_ALT_FIX

// PWC Scoring use WGS84 earth model, but LK8000 use FAISphere
// PG optimise can Work with WGS84 but Wapoint validation not..
// we need change Waypoint validation before use it
// #define _WGS84

// #define TOW_CRUISE // keep climb mode from engaging while on tow (unless turning steeply
                      // enough to warrant detection of the start of free flight)
                      // Eric Carden, 6/28/12

// #define ASPWAVEOFF	1 // Airspace wave areas disabled by default on startup, and set to Fly-In


// ----------------------------------------------------------------------------------------
//
// UNCOMPLETED WORK, or stuff that never got into production versions but still interesting
//
// ----------------------------------------------------------------------------------------

// #define RASTERCACHE		1	// fallback to cached dem if it does not fit in memory entirely. 
					// Does not work. 

// #define NEWSMARTZOOM		1	// stretch bitmap for fast zoom, uncompleted work (almost working)
					// Needs USEBIGZOOM

// #define USESWITCHES	1	// External device switch support / to be completed because unused
				// Do not remove, it can be worked out with no problems if we want to
				// support external switches through NMEA input.

// #define DSX			// only an experimental test feature for sms reception

// TopologyWriter for adding topo labels dynamically, previously used for markers
// May be sill used for custom user's topology, so let's keep it for a while.
// #define USETOPOMARKS	1	

// LKAIRSPACE OPTIM STUFF 
// Recalculate airspace positions only if the draw area has changed by more than 2 pixels
// There is a problem: enabling/disabling/ack/etc will not show up until a zoom is performed.
// So there is no more instant view of what is happening, and for this reason the option is disabled.
// #define LKASP_CALC_ON_CHANGE_ONLY		1

#include "Debug.h"	// DEBUG OPTIONS FOR EVERYONE, depending also on TESTBENCH


#endif
