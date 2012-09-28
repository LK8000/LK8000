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

 // New topology OPTIMIZE options
 #define TOPOFAST	1
 #define TOPOFASTLABEL	1
 #define TOPOFASTCACHE	1

 #define IBLSCALE(x) (   (ScreenIntScale) ? ((x)*ScreenScale) : ((int)((x)*ScreenDScale)))
 #define NIBLSCALE(x) (LKIBLSCALE[x])
 #define MAXIBLSCALE	100	// CAREFUL! NIBLSCALE can be used only UP TO MAXIBLSCALE!

 //
 // TEMPORARY FIXES THAT REQUIRE EXTENSIVE TESTING - KEEP #ifdef until expiring date
 // When expire date is reached, unnecessary old stuff can be removed, even if commented
 //

 #define ALPHADEBUG		1	// DEBUG STARTUPSTORE MESSAGES FOR ALPHA AND BETA VERSIONS
					// IN FINAL VERSIONS MAY BE DISABLED.. MAYBE.

 #define TESTBENCH	1	// THIS IS USING MORE MESSAGES, AND IT IS GOOD FOR DEVELOPMENT VERSIONS
				// For example, all dev.vers have CPUSTATS activated by this define, 
				// automatically.
				// COMMENT BEFORE PUBLIC RELEASE, NO EXCEPTIONS: INTERNAL USE ONLY!

 #define USELKASSERT	1	// Enable LKASSERT checks and diagnostic messages at runtime.
				// For TESTBENCH and beta versions this should be always ENABLED.
				// Only in official stable versions it should be disabled.


 #if TESTBENCH
 #define CPUSTATS		1	// Show cpu stats in inverted text mode inside map mode
 #endif

 #if (WINDOWSPC>0)
 #define WINE
 #endif

// #define LKCOMPETITION	1	// Will disable TRI 

 // This stuff will be permanently removed shortly
 // #define JP2000		1	// use JPG2000 terrain files .jp2 and jasper library
 #define LKMTERRAIN		1	// allow DEM terrain inside dat and lkm topology files



 //#define TOW_CRUISE // keep climb mode from engaging while on tow (unless turning steeply
                      // enough to warrant detection of the start of free flight)
                      // Eric Carden, 6/28/12
#define GTL2 // Draws a glide terrain line around the next (active) task waypoint.
             // Includes addition of MSG tokens "Line+NextWP" & "Shade+NextWP"
             // and change in HELP message 194 (for "Glide Terrain Line" setting).
             // Eric Carden, September 13, 2012

 // Modify best cruise track calculation to assume goal arrival
 // altitude of safety altitude (not current flight altitude).
 // Eric Carden, April 21, 2012
 // #define BCT_ALT_FIX

// Dynamic screen resizing. If NOT using alphablending for bottom bar, or using 100% opacity,
// then terrain and generally the moving map will not be drawn under the bottombar.
// On 480x272 this is saving around 12% CPU. This is also needed to allow DrawTerrain choosing any
// screen portion, on demand.
#define DYNASCREEN		1	






/*
 * Incomplete work, or stuff that never got into production versions but still interesting
 *

// #define RASTERCACHE		1	// fallback to cached dem if it does not fit in memory entirely. 
					// Does not work. 

// #define NEWSMARTZOOM		1	// stretch bitmap for fast zoom, uncompleted work (almost working)
					// Needs USEBIGZOOM
// #define USEBIGZOOM		1	// will fast redraw only terrain, with no topology &c.

// #define USESWITCHES	1	// External device switch support / to be completed because unused
				// Do not remove, it can be worked out with no problems if we want to
				// support external switches through NMEA input.

// #define DSX			// only an experimental test feature for sms reception

// TopologyWriter for adding topo labels dynamically, previously used for markers
// May be sill used for custom user's topology, so let's keep it for a while.
// #define USETOPOMARKS	1	

 *
 */

#include "Debug.h"	// DEBUG OPTIONS FOR EVERYONE, depending also on TESTBENCH


#endif
