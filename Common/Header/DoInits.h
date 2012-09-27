/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: DoInits.h,v 1.1 2011/12/21 10:35:29 root Exp root $

   PLEASE USE COMMENTS ALSO HERE TO DESCRIBE YOUR GLOBALS!
   YOU CAN INITIALIZE VALUES TO true,false,zero and NULL, 
   or you can do it also inside Globals_Init.

*/

#ifndef DOINITS_H
#define DOINITS_H


 // Master of Do Inits
 typedef enum {
	MDI_MASTER,			// #1
	MDI_FIRST_DOINIT,
	MDI_DORANGEWAYPOINTLIST,	// wp file change
	MDI_DOCALCULATIONSSLOW,		// force recalc bestalt, optimizer, validFix check
	MDI_HEADING,			// time change
	MDI_AVERAGE30S,			// time change
	MDI_DOAUTOQNH,			// force qnh search for 10 s
	MDI_DOAIRSPACES,		// asp file change
	MDI_DOCOMMON,			// generic reset
	MDI_DOTRAFFIC,			// FLARM trigger
	MDI_DETECTFREEFLYING,		// force detect FF 
	MDI_MCREADYCACHE,		// Reset cache for MC
	MDI_DOTHERMALHISTORY,		// Reset thermal history storage
	MDI_BATTERYMANAGER,		// Reset warnings on battery power
	MDI_EVENTINVERTCOLOR,		// OutlinedTp  , to be checked TODO
	MDI_GETLOCALPATH,		// system path change!
	MDI_LKSOUND,			// localpath changed
	MDI_DRAWTRI,			// screen change
	MDI_DRAWASPNEAREST,		// screen change, font change
	MDI_DRAWCOMMON,			// screen change, font change
	MDI_DRAWFLARMTRAFFIC,		// screen change
	MDI_DRAWINFOPAGE,		// screen change
	MDI_WRITEINFO,			// screen change
	MDI_DRAWLOOK8000,		// screen change, font change
	MDI_DRAWMAPSPACE,		// screen change
	MDI_DRAWNEAREST,		// screen change, font change
	MDI_DRAWTARGET,			// screen change
	MDI_DRAWTHERMALHISTORY,		// screen change, font change
	MDI_DRAWTRAFFIC,		// screen change, font change
	MDI_DRAWVARIO,			// screen change
	MDI_MAPWNDPROC,			// screen change
	MDI_GETOVERTARGETHEADER,	// language change
	MDI_PROCESSVIRTUALKEY,		// screen change
	MDI_ONPAINTLISTITEM,		// screen change, font change
	MDI_DRAWMAPSCALE,		// screen change
	MDI_MAPWPLABELADD,		// screen change
	MDI_CHECKLABELBLOCK,		// screen change
	MDI_LKPROCESS,			// unit change
	MDI_COMPASS,			// screen change
	MDI_LOOKABLEND,			// screen change, drawlook ablend
	MDI_MAPWPVECTORS,		// screen change
	MDI_READLANGUAGEFILE,		// full restart
	MDI_FLARMRADAR,			// screen change
	MDI_MAPASP,			// screen change?
	MDI_MAPRADAR,			// screen change?
	MDI_MAPTEST,			// testing purposes
	MDI_LAST_DOINIT,		
 } MDI_t;

#if defined(STATIC_DOINITS)
  // Careful, we MUST initialize here. Check that MDIs fit in this array.
  bool DoInit[100]={ 
	1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,
  };
#else
  extern void Init_DoInits(void);
  extern void Reset_All_DoInits(void);
  extern void Reset_Single_DoInits(int position);
  extern bool DoInit[100];
#endif

#endif // DOINITS_H

