/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: options.h,v 8.2 2010/12/15 12:25:23 root Exp root $
*/
#ifndef OPTIONS_H
#define OPTIONS_H

#include "Debug.h"				// DEBUG OPTIONS FOR EVERYONE

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

 // NEWIBLSCALE  to be used only for NUMBERS between 0 and MAXIBLSCALE !!!!
 #define NEWIBLSCALE	1

 // New topology OPTIMIZE options
 #define TOPOFAST	1
 #define TOPOFASTLABEL	1
 #define TOPOFASTCACHE	1

// No instrument thread. However it is here ready to be used for anything.
#define NOINSTHREAD	1

#ifdef NEWIBLSCALE
 #define IBLSCALE(x) (   (InfoBoxLayout::IntScaleFlag) ? ((x)*InfoBoxLayout::scale) : ((int)((x)*InfoBoxLayout::dscale)))
 #define NIBLSCALE(x) (LKIBLSCALE[x])
 #define MAXIBLSCALE	100
#else
 #define IBLSCALE(x) (   (InfoBoxLayout::IntScaleFlag) ? ((x)*InfoBoxLayout::scale) : ((int)((x)*InfoBoxLayout::dscale)))
 #define NIBLSCALE(x) (   (InfoBoxLayout::IntScaleFlag) ? ((x)*InfoBoxLayout::scale) : ((int)((x)*InfoBoxLayout::dscale)))
#endif

#define NOWINDREGISTRY	1	// Configurable: load and save wind from registry

// TEMPORARY FIXES THAT REQUIRE EXTENSIVE TESTING - KEEP #ifdef until expiring date
// When expire date is reached, unnecessary old stuff can be removed, even if commented
//

#define ALPHADEBUG		1	// DEBUG STARTUPSTORE MESSAGES FOR ALPHA AND BETA VERSIONS
					// IN FINAL VERSIONS WILL BE DISABLED

#if (WINDOWSPC>0)
#define WINE
#endif

#define USEGOINIT		1	// expire 1.7.2011, it is disabled and should be removed

//#define USEWEATHER		1	// we dont use rasp weather - hangarware
//#define USEOLDASPWARNINGS	1	// we dont use old airspace warning system

#define USEIBOX			1	// Use old IBOX mode, work in progress for complete removal 


/*
 * Incomplete work, or stuff that never got into production versions but still interesting

#define FIXGDI		  // todo, work for further optimization of GDIs. 
			  // To check GDI memory leaks, use the freeware GDIView.exe 

#define LKCLIP		  // replace old Sutherland Hodgman clipping algo
#define NEWUTM		  // New full UTM support INCOMPLETED
#define NOIBOX		 1 // Ibox mode no more available, an important TODO
#define DSX		  // only an experimental test feature for sms reception

// Very old stuff probably we can clean and remove
#define   EXPERIMENTAL          0             // unused stuff, such like SMS send
#define   AIRSPACEUSEBINFILE    0             // use and maintain binary airspace file

 */

/*
 * All debug options should be put inside Debug.h
 */

#endif
