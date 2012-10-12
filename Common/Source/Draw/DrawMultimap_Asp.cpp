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

extern long  iSonarLevel;
bool Sonar_IsEnabled = true;

// Active map can be triggered only for a mapspace run. Changing mapspace will
// disable active map automatically. Entering a mapspace will make active map off.
bool ActiveMap_IsEnabled = false;

extern AirSpaceSonarLevelStruct sSonarLevel[];
extern TCHAR Sideview_szNearAS[];
extern RECT Sideview_TopRect_InUse;


AirSpaceSonarLevelStruct sSonarLevel[10] = {
    /* horizontal sonar levels */
    /* Dist , Delay *0.5s, V/H,      soundfile */
    {  150,     3,         true, TEXT("LK_SONAR_H1.WAV")},
    {  330,     3,         true, TEXT("LK_SONAR_H2.WAV")},
    {  500,     5,         true, TEXT("LK_SONAR_H3.WAV")},
    {  650,     5,         true, TEXT("LK_SONAR_H4.WAV")},
    {  850,     7,         true, TEXT("LK_SONAR_H5.WAV")},
    /* vertical sonar levels */
    {  30 ,     3,         false, TEXT("LK_SONAR_H1.WAV")},
    {  50 ,     3,         false, TEXT("LK_SONAR_H2.WAV")},
    {  70,      5,         false, TEXT("LK_SONAR_H3.WAV")},
    {  90,      5,         false, TEXT("LK_SONAR_H4.WAV")},
    {  110,     7,         false, TEXT("LK_SONAR_H5.WAV")}
   };



int SonarNotify(void)
{
  static unsigned long lSonarCnt = 0;
  lSonarCnt++;

  if((iSonarLevel >=0) && (iSonarLevel < 10))
	if( lSonarCnt > (unsigned)sSonarLevel[iSonarLevel].iSoundDelay)
	{
	  lSonarCnt = 0;
          // StartupStore(_T("... level=%d PLAY <%s>\n"),iSonarLevel,&sSonarLevel[iSonarLevel].szSoundFilename);
	  LKSound((TCHAR*) &(sSonarLevel[iSonarLevel].szSoundFilename));
	}
  return 0;
}



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
		// Upon entering a new multimap, Active is reset.
		ActiveMap_IsEnabled=false;
		break;

#if NEWMULTIMAPS
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
		if ( (MapSpaceMode==MSM_MAPTRK || MapSpaceMode==MSM_MAPWPT) && (Sideview_TopRect_InUse.bottom>0)) {
			ActiveMap_IsEnabled = !ActiveMap_IsEnabled;
			if (EnableSoundModes) {
				if (ActiveMap_IsEnabled)
					LKSound(TEXT("LK_TONEUP.WAV"));
				else
					LKSound(TEXT("LK_TONEDOWN.WAV"));
			}
		}
		break;
#endif
	default:
		// THIS SHOULD NEVER HAPPEN, but always CHECK FOR IT!
		break;
  }

#if NEWMULTIMAPS
  //
  // If the map is active in the proper mapspace, we shall manage here the action
  //
  if (LKevent==LKEVENT_SHORTCLICK && ActiveMap_IsEnabled && (MapSpaceMode==MSM_MAPTRK || MapSpaceMode==MSM_MAPWPT)) {
		//
		// It would be a GOOD IDEA to keep this as a global, updated of course.
		// We need to know very often how is the screen splitted, and where!
		// It should be made global somewhere else, not here.
		//
		if ( YstartScreen < Sideview_TopRect_InUse.bottom) {
			double Xstart, Ystart;
			SideviewScreen2LatLon(XstartScreen, YstartScreen, Xstart, Ystart);
			MapWindow::Event_NearestWaypointDetails(Xstart, Ystart, 1.0e5, false);
			LKevent=LKEVENT_NONE;
			ActiveMap_IsEnabled=false;
			return;
		}
  }
#endif

  //
  // This is doing all rendering, including terrain and topology, which is not good.
  //
#ifdef ENABLE_ALL_AS_FOR_SIDEVIEW
  int oldAltMode = AltitudeMode ;

  AltitudeMode = ALLON;
#endif

  RenderAirspace(hdc, rci);

#ifdef ENABLE_ALL_AS_FOR_SIDEVIEW
  AltitudeMode = oldAltMode;
#endif


#if NEWMULTIMAPS
#else
  DrawMultimap_Topleft(hdc, rci);
  DrawMultimap_Topright(hdc, rci);
  DrawMultimap_DynaLabel(hdc, rci);
#endif

  if(GetSideviewPage()== IM_NEAR_AS) SonarNotify();


  LKevent=LKEVENT_NONE;

}

