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

 #define NOWINDREGISTRY	1	// Configurable: load and save wind from registry

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

 // #define RASTERCACHE		1	// fallback to cached dem if it does not fit in memory entirely. 
					// Does not work. 

 // Both can be enabled, so careful while testing
 #define NEWPROFILES		1	// new lk profile system with no windows registry
 // #define OLDPROFILES		1	// old lk profile with registry

 // #define OLDLOGGER		1	// old DLL Grecord logging system

 //#define NEWSMARTZOOM		1	// stretch bitmap for fast zoom, uncompleted work (almost working)
 //#define USEBIGZOOM		1	// will fast redraw only terrain, with no topology &c.
 #define FASTPAN		1	// bitblt panning 

 #define HIRESB			1	// HIRES bitmaps

/* BB_CHANGES makes some Bottom Bar-related changes:
 * 1. 3/11/2012: If user disables all non-TRM0 bottom bars, then (1) give a
 *    warning & (2) make it as if the user enabled NAV1.
 *    adds MSG token 16 (replacing unused one).
 *    changes HELP token 1220.
 * Other BB changes are to follow soon in later commits.
 * Eric Carden, March 11, 2012
 */
 #define BB_CHANGES
 
 //#define ASCIILOGBOOK // old logbook format encoded in ASCII (new is UTF8)

/*
 * Incomplete work, or stuff that never got into production versions but still interesting
 *

    #define USESWITCHES	1	// External device switch support / to be completed because unused
				// Do not remove, it can be worked out with no problems if we want to
				// support external switches through NMEA input.

    #define FIXGDI	        // todo, work for further optimization of GDIs. 
			        // To check GDI memory leaks, use the freeware GDIView.exe 

    #define LKCLIP		// replace old Sutherland Hodgman clipping algo
    #define NEWUTM		// New full UTM support INCOMPLETED
    #define DSX			// only an experimental test feature for sms reception

   // TopologyWriter for adding topo labels dynamically, previously used for markers
   // May be sill used for custom user's topology, so let's keep it for a while.
   #define USETOPOMARKS	1	

 *
 */

#include "Debug.h"	// DEBUG OPTIONS FOR EVERYONE, depending also on TESTBENCH

#endif
