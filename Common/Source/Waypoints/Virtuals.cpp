/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "LKStyle.h"


void AddReservedWaypoints()
{
    WayPointList.resize(NUMRESWP);
    WayPointCalc.resize(NUMRESWP);
        
	WayPointList[RESWP_TAKEOFF].Number=RESWP_TAKEOFF+1;
	WayPointList[RESWP_TAKEOFF].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_TAKEOFF].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_TAKEOFF].Altitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_TAKEOFF].Flags=TURNPOINT;
	_tcscpy(WayPointList[RESWP_TAKEOFF].Name, gettext(TEXT(RESWP_TAKEOFF_NAME)) ); // 100227
	if ( WayPointList[RESWP_TAKEOFF].Comment == NULL)
		WayPointList[RESWP_TAKEOFF].Comment = (TCHAR*)malloc((COMMENT_SIZE+1)*sizeof(TCHAR));
	if (WayPointList[RESWP_TAKEOFF].Comment!=NULL)
		_tcscpy(WayPointList[RESWP_TAKEOFF].Comment,_T("WAITING FOR GPS POSITION"));
	WayPointList[RESWP_TAKEOFF].Reachable=FALSE;
	WayPointList[RESWP_TAKEOFF].AltArivalAGL=0.0;
	WayPointList[RESWP_TAKEOFF].Visible=FALSE;
	WayPointList[RESWP_TAKEOFF].InTask=false;
	WayPointList[RESWP_TAKEOFF].Details=(TCHAR *)NULL;
	
	WayPointList[RESWP_TAKEOFF].FarVisible=false;
	WayPointList[RESWP_TAKEOFF].FileNum=-1;  // 100219  so it cannot be saved
	WayPointList[RESWP_TAKEOFF].Format= LKW_VIRTUAL;  //@ bugfix 101110
	
	WayPointList[RESWP_LASTTHERMAL].Number=RESWP_LASTTHERMAL+1;
	WayPointList[RESWP_LASTTHERMAL].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_LASTTHERMAL].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_LASTTHERMAL].Altitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_LASTTHERMAL].Flags=TURNPOINT;
	_tcscpy(WayPointList[RESWP_LASTTHERMAL].Name, gettext(TEXT(RESWP_LASTTHERMAL_NAME)) );
	if ( WayPointList[RESWP_LASTTHERMAL].Comment == NULL)
		WayPointList[RESWP_LASTTHERMAL].Comment = (TCHAR*)malloc((COMMENT_SIZE+1)*sizeof(TCHAR));
	// LKTOKEN _@M1320_ "LAST GOOD THERMAL"
	if (WayPointList[RESWP_LASTTHERMAL].Comment!=NULL) 
		_tcscpy(WayPointList[RESWP_LASTTHERMAL].Comment, gettext(TEXT("_@M1320_")));		
	WayPointList[RESWP_LASTTHERMAL].Reachable=FALSE;
	WayPointList[RESWP_LASTTHERMAL].AltArivalAGL=0.0;
	WayPointList[RESWP_LASTTHERMAL].Visible=TRUE; // careful! 100929
	WayPointList[RESWP_LASTTHERMAL].InTask=false;
	WayPointList[RESWP_LASTTHERMAL].Details=(TCHAR *)NULL;
	WayPointList[RESWP_LASTTHERMAL].FarVisible=TRUE; // careful! 100929
	WayPointList[RESWP_LASTTHERMAL].FileNum=-1;

	WayPointCalc[RESWP_LASTTHERMAL].WpType = WPT_TURNPOINT;
	WayPointList[RESWP_LASTTHERMAL].Style = STYLE_THERMAL;
	WayPointCalc[RESWP_LASTTHERMAL].IsLandable = false;
	WayPointCalc[RESWP_LASTTHERMAL].IsAirport = false;
	WayPointCalc[RESWP_LASTTHERMAL].IsOutlanding = false;
	WayPointList[RESWP_LASTTHERMAL].Format= LKW_VIRTUAL;  //@ bugfix 101110

	WayPointList[RESWP_TEAMMATE].Number=RESWP_TEAMMATE+1;
	WayPointList[RESWP_TEAMMATE].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_TEAMMATE].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_TEAMMATE].Altitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_TEAMMATE].Flags=TURNPOINT;
	_tcscpy(WayPointList[RESWP_TEAMMATE].Name, gettext(TEXT(RESWP_TEAMMATE_NAME)) );
	if ( WayPointList[RESWP_TEAMMATE].Comment == NULL)
		WayPointList[RESWP_TEAMMATE].Comment = (TCHAR*)malloc((COMMENT_SIZE+1)*sizeof(TCHAR));
	// LKTOKEN _@M1321_ "TEAM MATE"
	if (WayPointList[RESWP_TEAMMATE].Comment!=NULL) 
		_tcscpy(WayPointList[RESWP_TEAMMATE].Comment, gettext(TEXT("_@M1321_")));
	WayPointList[RESWP_TEAMMATE].Reachable=FALSE;
	WayPointList[RESWP_TEAMMATE].AltArivalAGL=0.0;
	WayPointList[RESWP_TEAMMATE].Visible=FALSE;
	WayPointList[RESWP_TEAMMATE].InTask=false;
	WayPointList[RESWP_TEAMMATE].Details=(TCHAR *)NULL;
	WayPointList[RESWP_TEAMMATE].FarVisible=false;
	WayPointList[RESWP_TEAMMATE].FileNum=-1;

	WayPointCalc[RESWP_TEAMMATE].WpType = WPT_TURNPOINT;
	WayPointCalc[RESWP_TEAMMATE].IsLandable = false;
	WayPointCalc[RESWP_TEAMMATE].IsAirport = false;
	WayPointCalc[RESWP_TEAMMATE].IsOutlanding = false;
	WayPointList[RESWP_TEAMMATE].Format= LKW_VIRTUAL;  //@ bugfix 101110

	WayPointList[RESWP_FLARMTARGET].Number=RESWP_FLARMTARGET+1;
	WayPointList[RESWP_FLARMTARGET].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_FLARMTARGET].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_FLARMTARGET].Altitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_FLARMTARGET].Flags=TURNPOINT;
	_tcscpy(WayPointList[RESWP_FLARMTARGET].Name, gettext(TEXT(RESWP_FLARMTARGET_NAME)) );
	if ( WayPointList[RESWP_FLARMTARGET].Comment == NULL)
		WayPointList[RESWP_FLARMTARGET].Comment = (TCHAR*)malloc((COMMENT_SIZE+1)*sizeof(TCHAR));
	// LKTOKEN _@M1322_ "FLARM TARGET"
	if (WayPointList[RESWP_FLARMTARGET].Comment!=NULL) 
		_tcscpy(WayPointList[RESWP_FLARMTARGET].Comment, gettext(TEXT("_@M1322_")));
	WayPointList[RESWP_FLARMTARGET].Reachable=FALSE;
	WayPointList[RESWP_FLARMTARGET].AltArivalAGL=0.0;
	WayPointList[RESWP_FLARMTARGET].Visible=FALSE;
	WayPointList[RESWP_FLARMTARGET].InTask=false;
	WayPointList[RESWP_FLARMTARGET].Details=(TCHAR *)NULL;
	WayPointList[RESWP_FLARMTARGET].FarVisible=false;
	WayPointList[RESWP_FLARMTARGET].FileNum=-1;

	WayPointCalc[RESWP_FLARMTARGET].WpType = WPT_TURNPOINT;
	WayPointCalc[RESWP_FLARMTARGET].IsLandable = false;
	WayPointCalc[RESWP_FLARMTARGET].IsAirport = false;
	WayPointCalc[RESWP_FLARMTARGET].IsOutlanding = false;
	WayPointList[RESWP_FLARMTARGET].Format= LKW_VIRTUAL;  //@ bugfix 101110

	WayPointList[RESWP_OPTIMIZED].Number=RESWP_OPTIMIZED+1;
	WayPointList[RESWP_OPTIMIZED].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_OPTIMIZED].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_OPTIMIZED].Altitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_OPTIMIZED].Flags=TURNPOINT;
	// name will be assigned by function dynamically
	_tcscpy(WayPointList[RESWP_OPTIMIZED].Name, _T("OPTIMIZED") );
	if ( WayPointList[RESWP_OPTIMIZED].Comment == NULL)
		WayPointList[RESWP_OPTIMIZED].Comment = (TCHAR*)malloc((COMMENT_SIZE+1)*sizeof(TCHAR));
	if (WayPointList[RESWP_OPTIMIZED].Comment!=NULL) 
	       _tcscpy(WayPointList[RESWP_OPTIMIZED].Comment, _T("OPTIMIZED") );

	WayPointList[RESWP_OPTIMIZED].Reachable=FALSE;
	WayPointList[RESWP_OPTIMIZED].AltArivalAGL=0.0;
	WayPointList[RESWP_OPTIMIZED].Visible=FALSE;
	WayPointList[RESWP_OPTIMIZED].InTask=false;
	WayPointList[RESWP_OPTIMIZED].Details=(TCHAR *)NULL;
	WayPointList[RESWP_OPTIMIZED].FarVisible=false;
	WayPointList[RESWP_OPTIMIZED].FileNum=-1;

	WayPointCalc[RESWP_OPTIMIZED].WpType = WPT_TURNPOINT;
	WayPointCalc[RESWP_OPTIMIZED].IsLandable = false;
	WayPointCalc[RESWP_OPTIMIZED].IsAirport = false;
	WayPointCalc[RESWP_OPTIMIZED].IsOutlanding = false;
	WayPointList[RESWP_OPTIMIZED].Format= LKW_VIRTUAL;

	//
	// VIRTUAL FAIOPTIMIZED
	//
	WayPointList[RESWP_FAIOPTIMIZED].Number=RESWP_FAIOPTIMIZED+1;
	WayPointList[RESWP_FAIOPTIMIZED].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_FAIOPTIMIZED].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_FAIOPTIMIZED].Altitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_FAIOPTIMIZED].Flags=TURNPOINT;
	_tcscpy(WayPointList[RESWP_FAIOPTIMIZED].Name, gettext(TEXT(RESWP_FAIOPTIMIZED_NAME)) );
	if ( WayPointList[RESWP_FAIOPTIMIZED].Comment == NULL)
		WayPointList[RESWP_FAIOPTIMIZED].Comment = (TCHAR*)malloc((COMMENT_SIZE+1)*sizeof(TCHAR));
	if (WayPointList[RESWP_FAIOPTIMIZED].Comment!=NULL)
		_tcscpy(WayPointList[RESWP_FAIOPTIMIZED].Comment,_T("FAI OPTIMIZED VIRTUAL TURNPOINT"));
	WayPointList[RESWP_FAIOPTIMIZED].Reachable=FALSE;
	WayPointList[RESWP_FAIOPTIMIZED].AltArivalAGL=0.0;
	WayPointList[RESWP_FAIOPTIMIZED].Visible=FALSE;
	WayPointList[RESWP_FAIOPTIMIZED].InTask=false;
	WayPointList[RESWP_FAIOPTIMIZED].Details=(TCHAR *)NULL;
	
	WayPointList[RESWP_FAIOPTIMIZED].FarVisible=false;
	WayPointList[RESWP_FAIOPTIMIZED].FileNum=-1;
	WayPointList[RESWP_FAIOPTIMIZED].Format= LKW_VIRTUAL;

	//
	// VIRTUAL FREEFLY START
	//
	WayPointList[RESWP_FREEFLY].Number=RESWP_FREEFLY+1;
	WayPointList[RESWP_FREEFLY].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_FREEFLY].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_FREEFLY].Altitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_FREEFLY].Flags=TURNPOINT;
	_tcscpy(WayPointList[RESWP_FREEFLY].Name, gettext(TEXT(RESWP_FREEFLY_NAME)) );
	if ( WayPointList[RESWP_FREEFLY].Comment == NULL)
		WayPointList[RESWP_FREEFLY].Comment = (TCHAR*)malloc((COMMENT_SIZE+1)*sizeof(TCHAR));
	if (WayPointList[RESWP_FREEFLY].Comment!=NULL)
		_tcscpy(WayPointList[RESWP_FREEFLY].Comment,_T("START OF FREEFLIGHT VIRTUAL TURNPOINT"));
	WayPointList[RESWP_FREEFLY].Reachable=FALSE;
	WayPointList[RESWP_FREEFLY].AltArivalAGL=0.0;
	WayPointList[RESWP_FREEFLY].Visible=FALSE;
	WayPointList[RESWP_FREEFLY].InTask=false;
	WayPointList[RESWP_FREEFLY].Details=(TCHAR *)NULL;
	
	WayPointList[RESWP_FREEFLY].FarVisible=false;
	WayPointList[RESWP_FREEFLY].FileNum=-1;
	WayPointList[RESWP_FREEFLY].Format= LKW_VIRTUAL;

	//
	// VIRTUAL PANPOS
	// This is used as a temporary position while moving a task point
	//
	WayPointList[RESWP_PANPOS].Number=RESWP_PANPOS+1;
	WayPointList[RESWP_PANPOS].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_PANPOS].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_PANPOS].Altitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_PANPOS].Flags=TURNPOINT;
	_tcscpy(WayPointList[RESWP_PANPOS].Name, _T("")); 
	if ( WayPointList[RESWP_PANPOS].Comment == NULL)
		WayPointList[RESWP_PANPOS].Comment = (TCHAR*)malloc((COMMENT_SIZE+1)*sizeof(TCHAR));
	if (WayPointList[RESWP_PANPOS].Comment!=NULL)
		_tcscpy(WayPointList[RESWP_PANPOS].Comment,_T("PANPOS VIRTUAL TURNPOINT"));
	WayPointList[RESWP_PANPOS].Reachable=FALSE;
	WayPointList[RESWP_PANPOS].AltArivalAGL=0.0;
	WayPointList[RESWP_PANPOS].Visible=FALSE;
	WayPointList[RESWP_PANPOS].InTask=false;
	WayPointList[RESWP_PANPOS].Details=(TCHAR *)NULL;
	
	WayPointList[RESWP_PANPOS].FarVisible=false;
	WayPointList[RESWP_PANPOS].FileNum=-1;
	WayPointList[RESWP_PANPOS].Format= LKW_VIRTUAL;

	//
	// VIRTUAL UNUSED (RESERVED FOR NEXT TIME..)
	//
	WayPointList[RESWP_UNUSED].Number=RESWP_UNUSED+1;
	WayPointList[RESWP_UNUSED].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_UNUSED].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_UNUSED].Altitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_UNUSED].Flags=TURNPOINT;
	_tcscpy(WayPointList[RESWP_UNUSED].Name, gettext(TEXT(RESWP_UNUSED_NAME)) );
	if ( WayPointList[RESWP_UNUSED].Comment == NULL)
		WayPointList[RESWP_UNUSED].Comment = (TCHAR*)malloc((COMMENT_SIZE+1)*sizeof(TCHAR));
	if (WayPointList[RESWP_UNUSED].Comment!=NULL)
		_tcscpy(WayPointList[RESWP_UNUSED].Comment,_T("UNUSED VIRTUAL TURNPOINT"));
	WayPointList[RESWP_UNUSED].Reachable=FALSE;
	WayPointList[RESWP_UNUSED].AltArivalAGL=0.0;
	WayPointList[RESWP_UNUSED].Visible=FALSE;
	WayPointList[RESWP_UNUSED].InTask=false;
	WayPointList[RESWP_UNUSED].Details=(TCHAR *)NULL;
	
	WayPointList[RESWP_UNUSED].FarVisible=false;
	WayPointList[RESWP_UNUSED].FileNum=-1;
	WayPointList[RESWP_UNUSED].Format= LKW_VIRTUAL;

   for (short i=RESWP_FIRST_MARKER; i<=RESWP_LAST_MARKER; i++) {
	WayPointList[i].Number=i+1;
	WayPointList[i].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[i].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[i].Altitude=RESWP_INVALIDNUMBER;
	WayPointList[i].Flags=TURNPOINT;
	_tcscpy(WayPointList[i].Name, _T("LKMARKER"));
	if ( WayPointList[i].Comment == NULL)
		WayPointList[i].Comment = (TCHAR*)malloc((COMMENT_SIZE+1)*sizeof(TCHAR));
	if (WayPointList[i].Comment!=NULL) 
		_tcscpy(WayPointList[i].Comment, _T(""));
	WayPointList[i].Reachable=FALSE;
	WayPointList[i].AltArivalAGL=0.0;
	WayPointList[i].Visible=FALSE;
	WayPointList[i].InTask=false;
	WayPointList[i].Details=(TCHAR *)NULL;
	WayPointList[i].FarVisible=FALSE;
	WayPointList[i].FileNum=-1;
	WayPointList[i].Style = STYLE_MARKER;

	WayPointCalc[i].WpType = WPT_UNKNOWN;
	WayPointCalc[i].IsLandable = false;
	WayPointCalc[i].IsAirport = false;
	WayPointCalc[i].IsOutlanding = false;
	WayPointList[i].Format= LKW_VIRTUAL;
   }

}


/**
 *  Must be called BEFORE ReadWaypoints()!! 
 *  #LockTaskData() Needed for call this.
 */
void InitVirtualWaypoints()	// 091102
{
  #if TESTBENCH
  StartupStore(_T(". InitVirtualWaypoints: start%s"),NEWLINE);
  #endif

  // if first load, reserve space
  if (WayPointList.size()<=NUMRESWP) {
	AddReservedWaypoints();
	#if TESTBENCH
	StartupStore(_T(". InitVirtualWaypoints: done (%d vwp)%s"),NUMRESWP,NEWLINE);
	#endif
  } else {
	#if TESTBENCH
	StartupStore(_T(".. InitVirtualWaypoints: already done, skipping.%s"),NEWLINE);
        #endif
  }
}
