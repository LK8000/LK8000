/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: options.h,v 8.2 2010/12/15 12:25:23 root Exp root $
*/
#ifndef OPTIONS_H
#define OPTIONS_H

#include "Debug.h"				// DEBUG OPTIONS FOR EVERYONE
#define   MONOCHROME_SCREEN     1             // optimize for monochrom screen
#define   EXPERIMENTAL          0             // ????

#define   LOGGDEVICEINSTREAM    0             // log device in stream
#define   LOGGDEVCOMMANDLINE    NULL          // device in-stream logger command line
                                              // ie TEXT("-logA=\\Speicherkarte\\logA.log ""-logB=\\SD Card\\logB.log""")
#define   AIRSPACEUSEBINFILE    0             // use and maintain binary airspace file

// define this to be true for windows PC port
#if !defined(WINDOWSPC)
#define   WINDOWSPC             0
#endif

#define   FONTQUALITY           NONANTIALIASED_QUALITY

#if (WINDOWSPC>0)
#if _DEBUG
// leak checking
#define CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif

#define DISABLEAUDIOVARIO

// disable internally generated sounds
//#define DISABLEAUDIO

// no vegavoice support
#undef VEGAVOICE

#ifndef BIGDISPLAY
#define BIGDISPLAY
#endif

#ifdef PNA
#define NOLINETO
#endif

// General OPTIMIZATION for LK8000. Needed for all sub optimization definitions
#define LK8000_OPTIMIZE 	1


// -------------------------------------------------------------
// Activate cache on all calculations defined below:
#ifdef LK8000_OPTIMIZE

 // NEWIBLSCALE  to be used only for NUMBERS between 0 and MAXIBLSCALE !!!!
 #define NEWIBLSCALE	1

 // CACHE CALCULATIONS: IT IS IMPERATIVE THAT THIS OPTION CAN BE DISABLED ANYTIME!
 #define LK_CACHECALC 1
 // Notice:  These are only used if above is active!
 // MacCreadyAltitude and its statistics
 #define LK_CACHECALC_MCA 60
 // #define LK_CACHECALC_MCA_STAT 1

 #define NOFLARMGAUGE	1
 // Old vario gauge for landscape geometry 6
 #define NOVARIOGAUGE	1
 #define NOCDIGAUGE	1
 // no instrument thread
 #define NOINSTHREAD	1

 #define FLIPFLOP	1	// obsoleted by MULTICALC. REMOVE WHEN MULTICALC IS PERMANENT
#endif
// -------------------------------------------------------------

#ifdef NEWIBLSCALE
 #define IBLSCALE(x) (   (InfoBoxLayout::IntScaleFlag) ? ((x)*InfoBoxLayout::scale) : ((int)((x)*InfoBoxLayout::dscale)))
 #define NIBLSCALE(x) (LKIBLSCALE[x])
 #define MAXIBLSCALE	100
#else
 #define IBLSCALE(x) (   (InfoBoxLayout::IntScaleFlag) ? ((x)*InfoBoxLayout::scale) : ((int)((x)*InfoBoxLayout::dscale)))
 #define NIBLSCALE(x) (   (InfoBoxLayout::IntScaleFlag) ? ((x)*InfoBoxLayout::scale) : ((int)((x)*InfoBoxLayout::dscale)))
#endif

#define NOWINDREGISTRY	1	// Configurable: load and save wind from registry

// New topology OPTIMIZE options
#ifdef LK8000_OPTIMIZE
 #define TOPOFAST	1
 #define TOPOFASTLABEL	1
 #define TOPOFASTCACHE	1
#endif

// ew terrain manager, for shading and altitudes ex LKTEST
#define NEWRASTER	1	// expire 110601

#define ORBITER		1	// thermal orbiter

#define LKSTARTUP	1	// new startup screen

// 101020
// TEMPORARY FIXES THAT REQUIRE EXTENSIVE TESTING - KEEP #ifdef until expiring date
// When expire date is reached, unnecessary old stuff can be removed, even if commented
//
#define FIXDC			1	// expire 110601

// Will speed up Range search, and also BestAlternate search
#define UNSORTEDRANGE		1	// 101120 expire 110601


#define LKOBJ			1	// expire 110401

#define GASUPPORT		1	// oren's GA work

#define MULTICALC		1	// Use multicalc approach, splitting calculation inside MapWindow
					// thread into multiple instances, 0.5 or 0.33 Hz recommended
					// Extensive checking required expire 110601

#define MAP_ZOOM                1       // Applied 110204 expire 110601



#define ALPHADEBUG		1	// DEBUG STARTUPSTORE MESSAGES FOR ALPHA AND BETA VERSIONS
					// IN FINAL VERSIONS WILL BE DISABLED

#if (WINDOWSPC>0)
#define WINE
#endif

// ------------ NEW OLC ENGINE -------------------------------------------------
// Using NEW_OLC requires to change lk8000.rc and activate NEWOLC xmls. 
// To do so, comment non-NEWOLC and uncomment NEWOLC. They are 3 files in total.
// No need to change any XML filename. Just comment/uncomment.
//
// #define NEW_OLC                 1     // Applied 110313
// -----------------------------------------------------------------------------

/*
 * Incomplete work, or stuff that never got into production versions but still interesting

#define NEWTRIGGERGPS	  // Parser approach for triggering a quantum data completed
			  // The idea was correct, since I could verify later that also the real
			  // LX8000 has the same approach, although simplified!!

#define FIXGDI		  // todo, work for further optimization of GDIs. 
			  // To check GDI memory leaks, use the freeware GDIView.exe 

#define LKCLIP		  // replace old Sutherland Hodgman clipping algo
#define NEWUTM		  // New full UTM support INCOMPLETED
#define NOIBOX		  // Ibox mode no more available, an important TODO
#define DSX		  // only an experimental test feature for sms reception

 */

/*
 * Put here debug defines, so that other developers can activate them if needed.

#define DRAWLOAD          // show cpu load (set by DEBUG mode)
#define DEBUG_DBLCLK      // show double click is being pressed
#define VENTA_DEBUG_EVENT // show key events, actually very few.
#define VENTA_DEBUG_KEY   // activates scan key codes, so you know what an hardware key is mapped to
			  // the HW key scan code is displayed on the screen!
#define DEBUG_ROTARY      // write in DEBUG.TXT located in the same place of .exe , append mode
#define CPUSTATS          // activate cpu thread profiling stuff also in Cpustat.h
#define DEBUG_DEV_COM     // log device communication through DevBase class methods
#define DEBUG_GPS	  //  Parser.cpp will show more debug messages about wrong fixes
#define DEBUGNPM	  // port monitor and hearthbeats in Parser
#define DEBUG_BESTALTERNATE	// full bestalternate messages inside DEBUG.TXT in home directory
 */

#endif
