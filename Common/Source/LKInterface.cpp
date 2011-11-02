/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "InputEvents.h"

extern void ShowMenu();


void SetModeType(short modeindex, short modetype) {

	UnselectMapSpace( ModeTable[ModeIndex][CURTYPE] );
	// TODO: make safe checks
	ModeIndex=modeindex;
	CURTYPE=modetype;
	SelectMapSpace( ModeTable[ModeIndex][CURTYPE] );

}

// Advance through types inside current mode
//
void NextModeType() {

	UnselectMapSpace( ModeTable[ModeIndex][CURTYPE] );
	short curtype_entry=CURTYPE;
redo:
	if ( CURTYPE >= ModeTableTop[ModeIndex] ) {
		// point to first
		CURTYPE=0; 
	} else {
		CURTYPE++;
	}

	// if we are at the beginning point, we keep it
	if (CURTYPE == curtype_entry) goto finish;

	if (!ConfIP[ModeIndex][CURTYPE]) goto redo;

	if (CURTYPE == IM_CONTEST) {
		if (!UseContestEngine()) goto redo;
	}
finish:
	SelectMapSpace( ModeTable[ModeIndex][CURTYPE] );
}


void PreviousModeType() {
// usare ifcircling per decidere se 0 o 1
	UnselectMapSpace( ModeTable[ModeIndex][CURTYPE] );
	short curtype_entry=CURTYPE;
redo:
	if ( CURTYPE <= 0 ) {
		// point to last
		CURTYPE=ModeTableTop[ModeIndex]; 
	} else {
		CURTYPE--;

	}
	// if we are at the beginning point, we keep it
	if (CURTYPE == curtype_entry) goto finish;

	if (!ConfIP[ModeIndex][CURTYPE]) goto redo;

	if (!UseContestEngine()) {
		if (CURTYPE == IM_CONTEST) goto redo;
	}
finish:
	SelectMapSpace( ModeTable[ModeIndex][CURTYPE] );
}


// Advance inside Mode Table between map, wp, infopages 
// and reselect previous state for that mode
// Notice: does NOT advance inside modes through types
//
void NextModeIndex() {
	UnselectMapSpace(ModeTable[ModeIndex][CURTYPE]);
	InfoPageChange(true);
	SelectMapSpace(ModeTable[ModeIndex][CURTYPE]);
}

void SoundModeIndex() {
#ifndef DISABLEAUDIO
	if (EnableSoundModes) {
		switch(ModeIndex) {
			case LKMODE_MAP:
				PlayResource(TEXT("IDR_WAV_TONE7"));
				break;
			case LKMODE_INFOMODE:
				PlayResource(TEXT("IDR_WAV_TONE1"));
				break;
			case LKMODE_WP:
				PlayResource(TEXT("IDR_WAV_TONE2"));
				break;
			case LKMODE_NAV:
				PlayResource(TEXT("IDR_WAV_TONE3"));
				break;
			case LKMODE_TRF:
				PlayResource(TEXT("IDR_WAV_TONE4"));
				break;
		}
	}
#endif
}

void BottomSounds() {
#ifndef DISABLEAUDIO
   if (EnableSoundModes) {
	switch(BottomMode) {
		case 1:
			PlayResource(TEXT("IDR_WAV_HIGHCLICK"));
			break;
		case 2:
			PlayResource(TEXT("IDR_WAV_BTONE2"));
			break;
		case 3:
			PlayResource(TEXT("IDR_WAV_BTONE2"));
			break;
		case 4:
			PlayResource(TEXT("IDR_WAV_BTONE4"));
			break;
		case 5:
			PlayResource(TEXT("IDR_WAV_BTONE4"));
			break;
		case 6:
			PlayResource(TEXT("IDR_WAV_BTONE5"));
			break;
		case 7:
			PlayResource(TEXT("IDR_WAV_BTONE5"));
			break;
		case 8:
			PlayResource(TEXT("IDR_WAV_BTONE6"));
			break;
		case 9:
			PlayResource(TEXT("IDR_WAV_BTONE6"));
			break;
		default:
			PlayResource(TEXT("IDR_WAV_CLICK"));
			break;
	}
  }
#endif
}

void PreviousModeIndex() {
  UnselectMapSpace(ModeTable[ModeIndex][CURTYPE]);
  InfoPageChange(false);
  SelectMapSpace(ModeTable[ModeIndex][CURTYPE]);
}

// This will set mapspace directly, and set ModeIndex accordingly. So we can do a "goto" mapspace now.
void SetModeIndex(short i) {
  UnselectMapSpace(ModeTable[ModeIndex][CURTYPE]);
  if (i<0 || i>LKMODE_TOP) {
	DoStatusMessage(_T("ERR-137 INVALID MODEINDEX"));
	return;
  }
  ModeIndex=i;
  SelectMapSpace(ModeTable[ModeIndex][CURTYPE]);

}


// Selecting MapSpaceMode need also ModeIndex and ModeType to be updated!
// Do not use these functions directly..
// Toggling pages will force intermediate activations, so careful here
//
void UnselectMapSpace(short i) {

	return;
}


void SelectMapSpace(short i) {
	
	LKForceDoNearest=false;
	LKForceDoCommon=false;
	LKForceDoRecent=false;
	// Particular care not to leave pending events
	LKevent=LKEVENT_NONE;

	switch(i) {
		case MSM_LANDABLE:
		case MSM_AIRPORTS:
		case MSM_NEARTPS:
			// force DoNearest to run at once
			LKForceDoNearest=true;
			LKevent=LKEVENT_NEWRUN;
			SelectedPage[MapSpaceMode]=0;
			SelectedRaw[MapSpaceMode]=0;
			break;
		case MSM_AIRSPACES:
			LKevent=LKEVENT_NEWRUN;
			SelectedPage[MapSpaceMode]=0;
			SelectedRaw[MapSpaceMode]=0;
			break;
		case MSM_COMMON:
			LKForceDoCommon=true;
			LKevent=LKEVENT_NEWRUN;
			SelectedPage[MapSpaceMode]=0;
			SelectedRaw[MapSpaceMode]=0;
			break;
		case MSM_RECENT:
			LKForceDoRecent=true;
			LKevent=LKEVENT_NEWRUN;
			SelectedPage[MapSpaceMode]=0;
			SelectedRaw[MapSpaceMode]=0;
			break;
		default:
			LKevent=LKEVENT_NEWRUN;
			break;
	}
	MapSpaceMode=i;
}


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
	default:
		DoStatusMessage(_T("ERR-726 INVALID CUSTOMKEY"));
		FailStore(_T("ERR-726 INVALID CUSTOMKEY=%d"),ckeymode);
		break;
  }

  return false;

}


//
void ToggleOverlays() {
  static int oldLook8000;
  static bool doinit=true;

  if (doinit) {
	if (Look8000 == (Look8000_t)lxcNoOverlay)
		oldLook8000=(Look8000_t)lxcAdvanced;
	else
		oldLook8000=Look8000;
	doinit=false;
  }

  if (Look8000>lxcNoOverlay)
	Look8000=lxcNoOverlay;
  else
	Look8000=oldLook8000;

}


// We assume that at least ConfBB[1] will be ON!
// We cannot have all OFF!
void BottomBarChange(bool advance) {

  short wanted;
  if (!advance) goto bbc_previous;

  wanted=BottomMode+1;
  while (true) {
    if (wanted >BM_LAST) {
	if ( MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
		wanted=BM_TRM;
		break;
	} else {
		wanted=BM_FIRST;
		continue;
	}
    }
    if (ConfBB[wanted]) break;
    wanted++;
  }
  BottomMode=wanted;
  return;

bbc_previous:
  wanted=BottomMode-1;
  while (true) {
    if (wanted == BM_TRM) {
	if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
		break;
	} else {
		wanted=BM_LAST;
		continue;
	}
    }
    if (wanted<0) {
	wanted=BM_LAST;
	continue;
    }
    if (ConfBB[wanted]) break;
    wanted--;
  }
  BottomMode=wanted;
  return;

}


void InfoPageChange(bool advance) {

  short wanted;
  if (!advance) goto ipc_previous;

  wanted=ModeIndex+1;
  while (true) {
    if (wanted >LKMODE_TOP) {
	wanted=LKMODE_MAP;
	break;
    }
    if (wanted == LKMODE_TRF) {
	if ( GPS_INFO.FLARM_Available ) break; // always ON if available
	wanted++;
	continue;
    }
    if (ConfMP[wanted]) break;
    wanted++;
  }
  ModeIndex=wanted;
  // Here we set the correct subpage initially. Only DrawInfoPage is checking CURTYPE, while
  // DrawNearest and commons use MapSpaceMode. It's a bit confusing, but both things are good.
  if (!ConfIP[ModeIndex][CURTYPE]) NextModeType();

  return;

ipc_previous:

  wanted=ModeIndex-1;
  while (true) {
    if (wanted <LKMODE_MAP) {
	wanted=LKMODE_TOP;
	continue;
    }
    if (wanted == LKMODE_TRF) {
	if ( GPS_INFO.FLARM_Available ) break; // always ON if available
	wanted--;
	continue;
    }
    if (ConfMP[wanted]) break;
    wanted--;
  }

  ModeIndex=wanted;
  if (!ConfIP[ModeIndex][CURTYPE]) PreviousModeType();
  return;

}




