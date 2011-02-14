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

// General OPTIMIZATION for LK8000. Needed for all sub optimization definitions
#define LK8000_OPTIMIZE 	1

#ifdef LK8000_OPTIMIZE
// NEWIBLSCALE  to be used only for NUMBERS between 0 and MAXIBLSCALE !!!!
#define NEWIBLSCALE	1
#endif

#ifdef BIGDISPLAY

#ifdef NEWIBLSCALE

#define IBLSCALE(x) (   (InfoBoxLayout::IntScaleFlag) ? ((x)*InfoBoxLayout::scale) : ((int)((x)*InfoBoxLayout::dscale)))
#define NIBLSCALE(x) (LKIBLSCALE[x])
#define MAXIBLSCALE	100

#else
#define IBLSCALE(x) (   (InfoBoxLayout::IntScaleFlag) ? ((x)*InfoBoxLayout::scale) : ((int)((x)*InfoBoxLayout::dscale)))
#define NIBLSCALE(x) (   (InfoBoxLayout::IntScaleFlag) ? ((x)*InfoBoxLayout::scale) : ((int)((x)*InfoBoxLayout::dscale)))
#endif

#else
#define IBLSCALE(x) (x)
#endif

#define FLARMNET	1
#define FLARM_AVERAGE	1

#ifdef PNA
#define NOLINETO
#endif


#endif


#define DSX

#define NEWTRAIL

// new nmea parser from december 2009
#define NEWGPS

// force polling mode
// #define NEWCOMM 1
// use configurable polling mode (do not mix with NEWCOMM !!
// Requires NEWGPS and a value like 1 to be defined
#define POLLINGMODE 1

// Parser.cpp will show more debug messages about wrong fixes
// #define DEBUG_GPS

// Use new terrain mode in RasterTerrain
#define NEWTERRAIN 1

// Use new QNH management
#define NEWQNH	1

// SeeYou cup waypoint format support
// Includes also COMPEgps support
#define CUPSUP	1

// activate cache on all calculations defined below
#ifdef LK8000_OPTIMIZE

 #define LK_CACHECALC 1
 // Notice:  These are only used if above is active!
 // MacCreadyAltitude and its statistics
 #define LK_CACHECALC_MCA 60
 // #define LK_CACHECALC_MCA_STAT 1

 // Optimize LKDraw functions with doinits (TO IMPROVE!)
 #define LKDRAW_OPTIMIZE		1

 // Optimize Terrain drawing, may be faulty and need more tests.
 // #define TERRAIN_OPTIMIZE	1

 // do not keep task abort code at all, no taskresume etc.
 // Task abort was already disabled. we are removing only unused code here
 #define NOTASKABORT	1
 #define NOFLARMGAUGE	1
 // Old vario gauge for landscape geometry 6
 #define NOVARIOGAUGE	1
 #define NOCDIGAUGE	1
 // no instrument thread
 #define NOINSTHREAD	1

 // use WaypointCalc.IsLandable &c. in place of WaypointList.Flags
 #define USEISLANDABLE	1

 // do calculations in map mode at 0.5hz, instead of 1hz
 #define FLIPFLOP	1

 // replace old Sutherland Hodgman clipping algo
 // #define LKCLIP		1

#endif

// New Move Icon on map function
#define NEWMOVEICON	1

// New task wp management
#define NEWTASKWP	1

// New full UTM support INCOMPLETED
// #define NEWUTM		1

// Don't load and save wind from registry
#define NOWINDREGISTRY	1

// New LK colors
#define LKCOLOR		1

// GetTickCount bugfix
#define GTCFIX	1

// WindowControl handling Width and Heigth negative
#define LKWINCONTROL	1

// new LK portrait mode interface
// #define LKPMODE	1

// New topology custom, and relative OPTIMIZE options
#define LKTOPO		1
#ifdef LK8000_OPTIMIZE
#ifdef LKTOPO
// topology optimization routines
#define TOPOFAST	1
#define TOPOFASTLABEL	1
#define TOPOFASTCACHE	1
#endif
#endif

// new terrain shadowing and highlighting
#define LKSHADING	1
// equivalent MC
#define EQMC		1
// auto orientation
#define AUTORIENT	1
// PC AlphaBlending option, unavailable on PNA and PDA
// #define ALPHABLENDING	1
// old splitting system for bottom bar, obsoleted
// #define OLDSPLITTER   1
// overtarget new system
#define OVERTARGET	1

// new virtual waypoints
#define NEWVIRTUALS	1

// new portrait navboxes
#define NEWPNAV		1

// ew terrain manager, for shading and altitudes ex LKTEST
#define NEWRASTER	1

// #define NOIBOX	1	// 101011 not working yet, not time to do it now

#define MOREDECLUTTER	1	// will declutter also airfields

#define CUPCOM		1	// CUP comments allocated dynamically

#define ORBITER		1	// thermal orbiter

#define LKSTARTUP	1	// new startup screen
#define NOSIM		1	// sim and fly in the same executable

// 101020
// TEMPORARY FIXES THAT REQUIRE EXTENSIVE TESTING - KEEP #ifdef until expiring date
// When expire date is reached, unnecessary old stuff can be removed, even if commented
//
#define FIX_RELOADCONFIG	1	// Applied: 101020 expires 110201
#define MATFIX1			1	// Applied: 101030 expires 110201
// #define FIX_MAPSIZE		1	// Applied: 101214 expires 110201
// #define FIXGDI			// todo
#define FIXDC			1
#define FIXISLANDABLE		1	// Applied: 101215 expire 110201
#define FIXNOLOADXML		1	// do not load xml from filesystem with tokenized XML

// Will speed up Range search, and also BestAlternate search
#define UNSORTEDRANGE		1	// 101120


#define LKOBJ			1

#define GASUPPORT		1	// oren's GA work

#define ABLEND			1	// Alpha Blending for airspaces and probably for many other
					// usages. Use AlphaBlendAvailable() to check, because the
					// function is inside coredll

#define MULTICALC		1	// Use multicalc approach, splitting calculation inside MapWindow
					// thread into multiple instances, 0.5 or 0.33 Hz recommended
					// Extensive checking required
#define MAP_ZOOM                1       // Applied 110204



#define ALPHADEBUG		1	// DEBUG STARTUPSTORE MESSAGES FOR ALPHA AND BETA VERSIONS
					// IN FINAL VERSIONS WILL BE DISABLED

/*
 * Put here debug defines, so that other developers can activate them if needed.

#define DRAWLOAD          // show cpu load (set by DEBUG mode)
#define DEBUG_DBLCLK      // show double click is being pressed
#define VENTA_DEBUG_EVENT // show key events, actually very few.
#define VENTA_DEBUG_KEY   // activates scan key codes, so you know what an hardware key is mapped to
#define DEBUG_ROTARY      // write in DEBUG.TXT located in the same place of .exe , append mode
#define CPUSTATS          // activate cpu thread profiling stuff also in Cpustat.h
#define DEBUG_DEV_COM     // log device communication through DevBase class methods
 */

