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
#include "Sideview.h"
#include "LKInterface.h"
#include "InputEvents.h"
#include "Multimap.h"
#include "Sound/Sound.h"

extern POINT startScreen;
extern bool IsMultimapConfigShown;



void MapWindow::LKDrawMultimap_Asp(LKSurface& Surface, const RECT& rc)
{


  RECT rci = rc;
  rci.bottom -= BottomSize;

  #if 0
  if (DoInit[MDI_MAPASP]) {
	DoInit[MDI_MAPASP]=false;
  }
  #endif

  switch(LKevent) {
	//
	// USABLE EVENTS
	//
	case LKEVENT_NEWRUN:
		// Upon entering a new multimap, Active is forced reset. It should not be necessary
		if (MapSpaceMode==MSM_VISUALGLIDE) {
			GetVisualGlidePoints(0); // reset upon entering!
		}
		break;

	case LKEVENT_TOPLEFT:
		IsMultimapConfigShown=true;
		InputEvents::setMode(_T("MMCONF"));
		break;

	case LKEVENT_TOPRIGHT:
		if (MapSpaceMode==MSM_MAPASP) {
			SonarWarning = !SonarWarning;
            if (SonarWarning)
                LKSound(TEXT("LK_TONEUP.WAV"));
            else
                LKSound(TEXT("LK_TONEDOWN.WAV"));
		}
		// ACTIVE is available only when there is a topview shown!
		if ( (MapSpaceMode==MSM_MAPTRK || MapSpaceMode==MSM_MAPWPT) && (Current_Multimap_TopRect.bottom>0)) {
			LKSound(TEXT("LK_TONEDOWN.WAV"));
		}
		break;
	default:
		// THIS SHOULD NEVER HAPPEN, but always CHECK FOR IT!
		break;
  }

  //
  // If the map is active in the proper mapspace, we shall manage here the action
  //
  if (LKevent==LKEVENT_LONGCLICK /*&& ActiveMap && (MapSpaceMode==MSM_MAPTRK || MapSpaceMode==MSM_MAPWPT)*/) {
		//
		// It would be a GOOD IDEA to keep this as a global, updated of course.
		// We need to know very often how is the screen splitted, and where!
		// It should be made global somewhere else, not here.
		//
	if ( startScreen.y < Current_Multimap_TopRect.bottom)
	{
	  if( PtInRect(&rc, startScreen))
	  {
        /*
         * we can't show dialog from Draw thread
         * instead, new event is queued, dialog will be popup by main thread
         */
        InputEvents::processGlideComputer(GCE_WAYPOINT_DETAILS_SCREEN);

	//	LKevent=LKEVENT_NONE;
	  }
	}
  }

  //
  // This is doing all rendering, including terrain and topology, which is not good.
  //
#ifdef ENABLE_ALL_AS_FOR_SIDEVIEW
  int oldAltMode = AltitudeMode ;

  if (GetSideviewPage() == IM_NEAR_AS)
   AltitudeMode = ALLON;
#endif

  RenderAirspace(Surface, rci);

#ifdef ENABLE_ALL_AS_FOR_SIDEVIEW
  AltitudeMode = oldAltMode;
#endif


 // LKevent=LKEVENT_NONE;

}
