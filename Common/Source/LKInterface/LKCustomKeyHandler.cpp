/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "InputEvents.h"
#include "LKInterface.h"

extern void ShowMenu();


// handle custom keys. Input: key pressed (center, left etc.)
// Returns true if handled successfully, false if not
bool CustomKeyHandler(const int key) {

  int ckeymode;
  static bool doinit=true;
  static int oldModeIndex;

  if (doinit) {
	oldModeIndex=LKMODE_INFOMODE;;
	doinit=false;
  }

  switch(key) {
	case CKI_BOTTOMCENTER:
		ckeymode=CustomKeyModeCenter;
		break;
	case CKI_BOTTOMLEFT:	
		ckeymode=CustomKeyModeLeft;
		break;
	case CKI_BOTTOMRIGHT:	
		ckeymode=CustomKeyModeRight;
		break;
	case CKI_BOTTOMICON:	
		ckeymode=CustomKeyModeAircraftIcon;
		break;
	case CKI_TOPLEFT:
		ckeymode=CustomKeyModeLeftUpCorner;
		break;
	case CKI_TOPRIGHT:
		ckeymode=CustomKeyModeRightUpCorner;
		break;
	case CKI_CENTERSCREEN:
		ckeymode=CustomKeyModeCenterScreen;
		break;
	default:
		DoStatusMessage(_T("ERR-725 UNKNWOWN CUSTOMKEY"));
		return false;
		break;
  }

  switch(ckeymode) {
	case ckDisabled:
		break;
	case ckZoomIn:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		MapWindow::zoom.EventScaleZoom(1);
		return true;
		break;
	case ckZoomInMore:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		MapWindow::zoom.EventScaleZoom(2);
		return true;
		break;
	case ckZoomOut:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		MapWindow::zoom.EventScaleZoom(-1);
		return true;
		break;
	case ckZoomOutMore:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		MapWindow::zoom.EventScaleZoom(-2);
		return true;
		break;
	case ckMenu:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		ShowMenu();
		return true;
	case ckBackMode:
		PreviousModeIndex();
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;
	case ckToggleMap: //TODO
		if (ModeIndex==LKMODE_MAP)
			SetModeIndex(oldModeIndex);
		else {
			oldModeIndex=ModeIndex;
			SetModeIndex(LKMODE_MAP);
		}
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;

	case ckTrueWind:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::setMode(_T("TrueWind"));
		return true;

	case ckTeamCode:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventSetup(_T("Teamcode"));
		return true;

	case ckToggleOverlays:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		ToggleOverlays();
		return true;

	case ckToggleMapLandable:
		if (ModeIndex==LKMODE_MAP)
			SetModeIndex(LKMODE_WP);
		else
			SetModeIndex(LKMODE_MAP);
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;
	case ckLandables:
		SetModeIndex(LKMODE_WP);
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;
	case ckToggleMapCommons:
		if (ModeIndex==LKMODE_MAP)
			SetModeIndex(LKMODE_NAV);
		else
			SetModeIndex(LKMODE_MAP);
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;
	case ckCommons:
		SetModeIndex(LKMODE_NAV);
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;
	case ckToggleMapTraffic:
		if (ModeIndex==LKMODE_MAP)
			SetModeIndex(LKMODE_TRF);
		else
			SetModeIndex(LKMODE_MAP);
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;
	case ckTraffic:
		SetModeIndex(LKMODE_TRF);
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;
	case ckInvertColors:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventInvertColor(NULL);
		return true;
	case ckTimeGates:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventTimeGates(NULL);
		return true;
	case ckMarkLocation:
		InputEvents::eventMarkLocation(_T(""));
		return true;
	case ckAutoZoom:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventZoom(_T("auto toggle"));
		InputEvents::eventZoom(_T("auto show"));
		return true;
	case ckActiveMap:
		InputEvents::eventActiveMap(_T("toggle"));
		InputEvents::eventActiveMap(_T("show"));
		return true;
	case ckBooster:
		DoStatusMessage(_T("FEEL THE THERMAL"));
		if (EnableSoundModes) LKSound(_T("LK_BOOSTER.WAV"));
		return true;
	case ckGoHome:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		if (ValidWayPoint(HomeWaypoint)) {
			if ( (ValidTaskPoint(ActiveWayPoint)) && (Task[ActiveWayPoint].Index == HomeWaypoint )) {
	// LKTOKEN  _@M82_ = "Already going home" 
				DoStatusMessage(gettext(TEXT("_@M82_")));
			} else {
				GotoWaypoint(HomeWaypoint);
			}
		} else
	// LKTOKEN  _@M465_ = "No Home to go!" 
			DoStatusMessage(gettext(TEXT("_@M465_")));
		return true;
	case ckPanorama:
		if (PGZoomTrigger==false)
			PGZoomTrigger=true;
		else
			LastZoomTrigger=0;
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		return true;

	case ckMultitargetRotate:
		RotateOvertarget();
		return true;

	case ckMultitargetMenu:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::setMode(_T("MTarget"));
		return true;
	case ckBaroToggle:
		ToggleBaroAltitude();
		return true;
	case ckBasicSetup:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventSetup(_T("Basic"));
		return true;
	case ckSimMenu:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::setMode(_T("SIMMENU"));
		return true;
	case ckToggleMapAirspace:
		if (ModeIndex==LKMODE_MAP)
			SetModeType(LKMODE_WP,WP_AIRSPACES);
		else
			SetModeIndex(LKMODE_MAP);
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;
	case ckAirspaceAnalysis:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventSetup(_T("AspAnalysis"));
		return true;
	case ckOptimizeRoute:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		PGOptimizeRoute=!PGOptimizeRoute;
		if (ISPARAGLIDER && PGOptimizeRoute) {
			AATEnabled = true;
		}
		return true;
	case ckLockScreen:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventService(_T("LOCKMODE"));
		return true;
	case ckWhereAmI:
		// no sound here, chime is played by service event
		InputEvents::eventService(_T("ORACLE"));
		return true;
	case ckUseTotalEnergy:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventService(_T("TOTALEN"));
		return true;
	case ckNotepad:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventChecklist(_T(""));
		return true;
	case ckTerrainColors:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventService(_T("TERRCOL"));
		return true;
	case ckNearestAirspace:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventNearestAirspaceDetails(NULL);
		return true;
	case ckOlcAnalysis:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventSetup(_T("OlcAnalysis"));
		return true;
	case ckTerrainColorsBack:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventService(_T("TERRCOLBACK"));
		return true;
	case ckForceFreeFlightRestart:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		LKSW_ForceFreeFlightRestart=true;
		return true;
	default:
		DoStatusMessage(_T("ERR-726 INVALID CUSTOMKEY"));
		FailStore(_T("ERR-726 INVALID CUSTOMKEY=%d"),ckeymode);
		break;
  }

  return false;

}

