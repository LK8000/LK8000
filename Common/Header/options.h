/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: options.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/
#ifndef OPTIONS_H
#define OPTIONS_H

#ifdef MSOFT
#if !defined(WINDOWSPC)
#define   WINDOWSPC             0
#else
#define   WINDOWSPC             1
#define   WINE	1
#endif
#endif

#if !defined(UNDER_CE)
#define NO_BLUETOOTH
#endif


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
 #define LK_CACHECALC_MCA 60U
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



#define DEVICE_SERIAL          // switch for external Hardware/Firmware Revision informations

//#define OLD_TIME_MODIFY // retore old TimeModify(...) Funtion

// Feedback is giving acoustic sound when long click has triggered the multiselection.
// It is introducing a TIMER in event processed inside MapWndProc.
//#define LONGCLICK_FEEDBACK




// #define LKCOMPETITION	1	// Will disable TRI 

// This stuff will be permanently removed shortly
#define LKMTERRAIN		1	// allow DEM terrain inside dat and lkm topology files


// Draws a glide terrain line around the next (active) task waypoint.
// Includes addition of MSG tokens "Line+NextWP" & "Shade+NextWP"
// and change in HELP message 194 (for "Glide Terrain Line" setting).
// Eric Carden, September 13, 2012
#define GTL2 

// Activate FastZoom and QUICKDRAW conditions, for fast paint of map the first time after zoom request
// In v5 this is practically unused. Should be removed asap.
#define USEBIGZOOM	1	

// Long trail size
#define LONGSNAIL  1

// ----------------------------------------------------------------------------------------
//
// OPTIONALs not in use within official LK versions, but still available for custom version
//
// ----------------------------------------------------------------------------------------

//#define ULLIS_PRIVATE_FEATURES  // Ulli's individual features
#ifdef ULLIS_PRIVATE_FEATURES
//#define GOTO_AS_SIMPLETASK  // even a singel goto will be listed in multiselect
  #define BUTTONS_MS
  #define OWN_POS_MS
  #define OWN_FLARM_TRACES
  #define FLARM_MS
//#define OUTLINE_2ND		// double outline airspaces
#endif

/*
 * MULTISELECT OPTIONS 
 */
//
// Team, Own and Oracle are introducing buttons for actions inside dialog.
// They must be first enabled with BUTTONS_MS
// WARNING THESE FUNCTIONS ARE NOT CHECKED FOR THREAD SAFETY AND CAN LEAD TO CRASHES
//
//#define BUTTONS_MS		// this is REQUIRED to enable one of the following:
//#define TEAM_CODE_MS		// button to trigger team code
//#define OWN_POS_MS		// button to trigger basic settings
//#define ORACLE_MS		// button to trigger oracle
//#define FLARM_MS

// Use F Record in IGC log files- not needed really
// #define LOGFRECORD	1

// do not add screen points if closer than 5pix to the previous
//#define LKASP_REMOVE_NEAR_POINTS		1

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

// LAST_TASKPOINT_QUESTION
#define LAST_TASKPOINT_QUESTION
// remove question for last turnpoint is good idea, but don't work
// Usability of task definition needed to be refactoring before remove that.

#include "Debug.h"	// DEBUG OPTIONS FOR EVERYONE, depending also on TESTBENCH


#endif
