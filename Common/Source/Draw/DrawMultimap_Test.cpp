/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKMapWindow.h"
#include "LKObjects.h"
#include "RGB.h"
#include "DoInits.h"
#include "Terrain.h"
extern int XstartScreen, YstartScreen;
extern long VKtime;

#if TESTBENCH

void MapWindow::LKDrawMultimap_Test(HDC hdc, const RECT rc)
{

  if (DoInit[MDI_MAPTEST]) {
	// init statics here and then clear init to false
	DoInit[MDI_MAPTEST]=false;
  }

  // 
  // X,Y coordinates of last clicked point on screen
  // These coordinates are related to any point clicked, even for a page flip, for bottom bar etc.
  // In some cases, you will read old coordinates because for example after clicking in the center of 
  // bottom bar, the page changed out of multimap and entered nearest pages.  
  // 
  int X=XstartScreen;
  int Y=YstartScreen;

  //
  // Duration of key is inside long VKtime, in milliseconds.
  //


  //
  // Draw a boxed terrain/topology  example
  // --------------------------------------
  //

  RECT rct=rc;		// desired area is 400x180, topleft is (20,30)
  rct.top=20;
  rct.bottom=rct.top+180;
  rct.left=30;
  rct.right=rct.left+400;

  MapWindow::ChangeDrawRect(rct);	// set new area for terrain and topology
  PanLatitude  = GPS_INFO.Latitude;
  PanLongitude = GPS_INFO.Longitude;

  // Current position  is in center map
  POINT Orig = { (rct.right-rct.left)/2,(rct.bottom-rct.top)/2};
  POINT Orig_Aircraft= {0,0};

  //zoom.ModifyMapScale();
  //zoom.UpdateMapScale();

  CalculateScreenPositions( Orig,  rct, &Orig_Aircraft);
  CalculateScreenPositionsAirspace();

  double sunelevation = 40.0;
  double sunazimuth=GetAzimuth();
  if (EnableTerrain && DerivedDrawInfo.TerrainValid) {
	DrawTerrain(hdc, rct, sunazimuth, sunelevation);
  }

  ResetLabelDeclutter();	// This is needed to reset at each run the declutter, for topology and waypoints!
  // SaturateLabelDeclutter();	// Use this to force no labels be printed, from now on.

  DrawTopology(hdc, rct);
  DrawAirSpace(hdc, rct);

  // ResetLabelDeclutter();	// If you saturated labels for topology, now you can reset the declutter to allow
				// printing only waypoints, 

  DrawWaypointsNew(hdc,rct);

  LKWriteBoxedText(hdc, _T("MULTIMAP PAGE EXAMPLE"), 1, 1 , 0, WTALIGN_LEFT);


  TCHAR ttext[100];
  
  switch(LKevent) {
	//
	// USABLE EVENTS
	// 
	case LKEVENT_NEWRUN:
		// CALLED ON ENTRY: when we select this page coming from another mapspace
		_tcscpy(ttext,_T("Event = NEW RUN"));
		break;
	case LKEVENT_UP:
		// click on upper part of screen, excluding center
		_tcscpy(ttext,_T("Event = UP"));
		break;
	case LKEVENT_DOWN:
		// click on lower part of screen,  excluding center
		_tcscpy(ttext,_T("Event = DOWN"));
		break;
	case LKEVENT_LONGCLICK:
		_stprintf(ttext,_T("Event = LONG CLICK"));
		break;
	case LKEVENT_PAGEUP:
		_tcscpy(ttext,_T("Event = PAGE UP"));
		break;
	case LKEVENT_PAGEDOWN:
		_tcscpy(ttext,_T("Event = PAGE DOWN"));
		break;
	case LKEVENT_TOPLEFT:
		_tcscpy(ttext,_T("Event = TOP LEFT"));
		break;
	case LKEVENT_TOPRIGHT:
		_tcscpy(ttext,_T("Event = TOP RIGHT"));
		break;


	//
	// THESE EVENTS ARE NOT AVAILABLE IN MULTIMAPS!
	//	
	case LKEVENT_ENTER:
		// click longer on center, like to confirm a selection
		_tcscpy(ttext,_T("Event = ENTER"));
		break;
	case LKEVENT_NEWPAGE:
		// swipe gesture up/down produces NEW PAGE in this case
		_tcscpy(ttext,_T("Event = NEW PAGE"));
		break;
	case LKEVENT_NONE:
		// Normally no event.
		_tcscpy(ttext,_T("Event = NONE"));
		break;
	//


	default:
		// THIS SHOULD NEVER HAPPEN, but always CHECK FOR IT!
		_tcscpy(ttext,_T("Event = unknown"));
		break;
  }

  LKWriteBoxedText(hdc, ttext, 1, 50 , 0, WTALIGN_LEFT);

  //
  // Be sure to check that an EVENT was generated, otherwise you are checking even bottombar key presses.
  //
  if (LKevent!=LKEVENT_NONE) {
	_stprintf(ttext,_T("Last coords: X=%d Y=%d  , duration=%ld ms"),X,Y,VKtime);
	LKWriteBoxedText(hdc, ttext, 1, 100 , 0, WTALIGN_LEFT);
  }


  // After using the event, WE MUST CLEAR IT, otherwise it will survive for next run.
  // This can be good for something, though, like automatic redo of last action.
  // You can also clear this event at the end of this function, to know during execution which was
  // the key pressed, but remember to clear it.
  LKevent=LKEVENT_NONE;



}


#else 
void MapWindow::LKDrawMultimap_Test(HDC hdc, const RECT rc) {
}
#endif // NOT TESTBENCH
