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
#include "Sideview.h"
#include "Message.h"
#include "LKInterface.h"
#include "InputEvents.h"
#include "Multimap.h"

extern int XstartScreen, YstartScreen;
extern bool IsMultimapConfigShown;

extern bool Sonar_IsEnabled;


void MapWindow::LKDrawMultimap_Asp(HDC hdc, const RECT rc)
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
		ActiveMap=false;
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
			Sonar_IsEnabled = !Sonar_IsEnabled;
			if (EnableSoundModes) {
				if (Sonar_IsEnabled)
					LKSound(TEXT("LK_TONEUP.WAV"));
				else
					LKSound(TEXT("LK_TONEDOWN.WAV"));
			}
		}
		// ACTIVE is available only when there is a topview shown!
		if ( (MapSpaceMode==MSM_MAPTRK || MapSpaceMode==MSM_MAPWPT) && (Current_Multimap_TopRect.bottom>0)) {
			ActiveMap = !ActiveMap;
			if (EnableSoundModes) {
				if (ActiveMap)
					LKSound(TEXT("LK_TONEUP.WAV"));
				else
					LKSound(TEXT("LK_TONEDOWN.WAV"));
			}
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
	if ( YstartScreen < Current_Multimap_TopRect.bottom)
	{
	  POINT Pos={XstartScreen, YstartScreen};
	  if( PtInRect(&rc, Pos))
	  {

		double Xstart, Ystart;
		SideviewScreen2LatLon(XstartScreen, YstartScreen, Xstart, Ystart);
//		StartupStore(_T("...... LKDrawMultimap_Asp lon:%f  lat:%f  \n"),Xstart,Ystart);
		MapWindow::Event_NearestWaypointDetails(Xstart, Ystart, 500*zoom.RealScale(), false);
		LKevent=LKEVENT_NONE;
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

  RenderAirspace(hdc, rci);

#ifdef ENABLE_ALL_AS_FOR_SIDEVIEW
  AltitudeMode = oldAltMode;
#endif


 // LKevent=LKEVENT_NONE;

}

