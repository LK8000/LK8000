/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKMapWindow.h"
#include "LKObjects.h"
#include "RGB.h"
#include "DoInits.h"
#include "Terrain.h"
#include "Multimap.h"
#include "../ScreenProjection.h"
extern POINT startScreen;
extern long VKtime;

#if TESTBENCH

void MapWindow::LKDrawMultimap_Test(LKSurface& Surface, const RECT& rc)
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
  int X=startScreen.x;
  int Y=startScreen.y;

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
  PanLatitude  = DrawInfo.Latitude;
  PanLongitude = DrawInfo.Longitude;

  // Current position  is in center map
  POINT Orig = { (rct.right-rct.left)/2,(rct.bottom-rct.top)/2};
  POINT Orig_Aircraft= {0,0};

  //zoom.ModifyMapScale();
  //zoom.UpdateMapScale();

  const ScreenProjection _Proj = CalculateScreenPositions( Orig,  rct, &Orig_Aircraft);
  CalculateScreenPositionsAirspace(rct, _Proj);

  double sunelevation = 40.0;
  double sunazimuth=GetAzimuth(DrawInfo, DerivedDrawInfo);
  if (IsMultimapTerrain() && DerivedDrawInfo.TerrainValid) {
	LockTerrainDataGraphics();
	DrawTerrain(Surface, rct, _Proj, sunazimuth, sunelevation);
	UnlockTerrainDataGraphics();
  }

  ResetLabelDeclutter();	// This is needed to reset at each run the declutter, for topology and waypoints!
  // SaturateLabelDeclutter();	// Use this to force no labels be printed, from now on.

  DrawTopology(Surface, rct, _Proj);
  DrawAirSpace(Surface, rct, _Proj);

  // ResetLabelDeclutter();	// If you saturated labels for topology, now you can reset the declutter to allow
				// printing only waypoints,

  DrawWaypointsNew(Surface,rct,_Proj);

  const auto oldpen = Surface.SelectObject(LKPen_White_N1);
  const auto oldbrush = Surface.SelectObject(LKBrush_LightGrey);

  LKWriteBoxedText(Surface, rct, _T("MULTIMAP PAGE EXAMPLE"), 1, 1, WTALIGN_LEFT, RGB_BLACK, RGB_WHITE);


  TCHAR ttext[100];

  switch(LKevent) {
	//
	// USABLE EVENTS
	//
	case LKEVENT_NEWRUN:
		// CALLED ON ENTRY: when we select this page coming from another mapspace
		lk::strcpy(ttext,_T("Event = NEW RUN"));
		break;
	case LKEVENT_UP:
		// click on upper part of screen, excluding center
		lk::strcpy(ttext,_T("Event = UP"));
		break;
	case LKEVENT_DOWN:
		// click on lower part of screen,  excluding center
		lk::strcpy(ttext,_T("Event = DOWN"));
		break;
	case LKEVENT_LONGCLICK:
		_stprintf(ttext,_T("Event = LONG CLICK"));
		break;
	case LKEVENT_PAGEUP:
		lk::strcpy(ttext,_T("Event = PAGE UP"));
		break;
	case LKEVENT_PAGEDOWN:
		lk::strcpy(ttext,_T("Event = PAGE DOWN"));
		break;
	case LKEVENT_TOPLEFT:
		lk::strcpy(ttext,_T("Event = TOP LEFT"));
		break;
	case LKEVENT_TOPRIGHT:
		lk::strcpy(ttext,_T("Event = TOP RIGHT"));
		break;
	case LKEVENT_SHORTCLICK:
		lk::strcpy(ttext,_T("Event = SHORT CLICK"));
		break;


	//
	// THESE EVENTS ARE NOT AVAILABLE IN MULTIMAPS!
	//
	case LKEVENT_ENTER:
		// click longer on center, like to confirm a selection
		lk::strcpy(ttext,_T("Event = ENTER"));
		break;
	case LKEVENT_NEWPAGE:
		// swipe gesture up/down produces NEW PAGE in this case
		lk::strcpy(ttext,_T("Event = NEW PAGE"));
		break;
	case LKEVENT_NONE:
		// Normally no event.
		lk::strcpy(ttext,_T("Event = NONE"));
		break;
	//


	default:
		// THIS SHOULD NEVER HAPPEN, but always CHECK FOR IT!
		lk::strcpy(ttext,_T("Event = unknown"));
		break;
  }

  LKWriteBoxedText(Surface, rct, ttext, 1, 50 , WTALIGN_LEFT, RGB_BLACK, RGB_WHITE);

  //
  // Be sure to check that an EVENT was generated, otherwise you are checking even bottombar key presses.
  //
  if (LKevent!=LKEVENT_NONE) {
	_stprintf(ttext,_T("Last coords: X=%d Y=%d  , duration=%ld ms"),X,Y,VKtime);
	LKWriteBoxedText(Surface, rct, ttext, 1, 100 , WTALIGN_LEFT, RGB_BLACK, RGB_WHITE);
  }


  // After using the event, WE MUST CLEAR IT, otherwise it will survive for next run.
  // This can be good for something, though, like automatic redo of last action.
  // You can also clear this event at the end of this function, to know during execution which was
  // the key pressed, but remember to clear it.
  LKevent=LKEVENT_NONE;

Surface.SelectObject(oldbrush);
Surface.SelectObject(oldpen);


}


#else
void MapWindow::LKDrawMultimap_Test(LKSurface& Surface, const RECT& rc) {
}
#endif // NOT TESTBENCH
