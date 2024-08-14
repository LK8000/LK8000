/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKInterface.h"
#include "InputEvents.h"
#include "Multimap.h"
#include "Sound/Sound.h"

extern bool IsMultimapConfigShown;

void SetModeType(short modeindex, short modetype) {

	UnselectMapSpace( ModeTable[ModeIndex][CURTYPE] );
	// TODO: make safe checks
	ModeIndex=modeindex;
	CURTYPE=modetype;
	SelectMapSpace( ModeTable[ModeIndex][CURTYPE] );

}

bool IsActiveModeType(short modeindex, short modetype) {
	if (ModeIndex != modeindex) {
		return false;
	}
	if (CURTYPE != modetype) {
		return false;
	}
	return true;
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

	if (CURMODE == MSM_INFO_CONTEST) {
		if (!UseContestEngine()) goto redo;
	}
#ifdef LKCOMPETITION
	if (CURMODE == MSM_INFO_TRI) goto redo;
#endif
	if (CURMODE == MSM_WELCOME) goto redo;
#ifndef OWN_FLARM_TRACES
	if (CURMODE == MSM_MAPRADAR && !GPS_INFO.FLARM_Available) goto redo;
#endif
	if ( (CURMODE == MSM_MAPTRK) && (Multimap1==mm_disabled) ) goto redo;
	if ( (CURMODE == MSM_MAPWPT) && (Multimap2==mm_disabled) ) goto redo;
	if ( (CURMODE == MSM_MAPASP) && (Multimap3==mm_disabled) ) goto redo;
	if ( (CURMODE == MSM_VISUALGLIDE) && (Multimap4==mm_disabled) ) goto redo;
	if ( (CURMODE == MSM_MAPRADAR) && (Multimap5==mm_disabled) ) goto redo;

	//if (CURMODE==MSM_MAPWPT && GetOvertargetIndex()<0) goto redo;
finish:
	SelectMapSpace( ModeTable[ModeIndex][CURTYPE] );
	if (ModeIndex==LKMODE_MAP) MultiMapSound();
	// Multimaps can change zoom, and when we are back to moving map we may have missing topology items
	if (MapSpaceMode==MSM_MAP) {
		MapWindow::ForceVisibilityScan=true;
		MapWindow::RefreshMap();
	}
	// 120919 If we have just selected MSM_MAP, request a fast refresh. Probably from a multimap.
	// Not sure it is really needed. Just in case, this is the right place to do it.
	// if (MapSpaceMode == MSM_MAP) MapWindow::RefreshMap();
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
		if (CURMODE == MSM_INFO_CONTEST) goto redo;
	}
#ifdef LKCOMPETITION
	if (CURMODE == MSM_INFO_TRI) goto redo;
#endif
	if (CURMODE == MSM_WELCOME) goto redo;

#ifndef OWN_FLARM_TRACES
	if (CURMODE == MSM_MAPRADAR && !GPS_INFO.FLARM_Available) goto redo;
#endif
	if ( (CURMODE == MSM_MAPTRK) && (Multimap1==mm_disabled) ) goto redo;
	if ( (CURMODE == MSM_MAPWPT) && (Multimap2==mm_disabled) ) goto redo;
	if ( (CURMODE == MSM_MAPASP) && (Multimap3==mm_disabled) ) goto redo;
	if ( (CURMODE == MSM_VISUALGLIDE) && (Multimap4==mm_disabled) ) goto redo;
	if ( (CURMODE == MSM_MAPRADAR) && (Multimap5==mm_disabled) ) goto redo;

	//if (CURMODE==MSM_MAPWPT && GetOvertargetIndex()<0) goto redo;
finish:
	SelectMapSpace( ModeTable[ModeIndex][CURTYPE] );
	if (ModeIndex==LKMODE_MAP) MultiMapSound();
	if (MapSpaceMode==MSM_MAP) {
		MapWindow::ForceVisibilityScan=true;
		MapWindow::RefreshMap();
	}

	// 120919 If we have just selected MSM_MAP, request a fast refresh. Probably from a multimap.
	// Not sure it is really needed. Just in case, this is the right place to do it.
	// if (MapSpaceMode == MSM_MAP) MapWindow::RefreshMap();
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

void BottomSounds() {
	switch(BottomMode) {
		case BM_CRU:
			PlayResource(TEXT("IDR_WAV_BTONE7"));
			break;
		case BM_HGH:
			PlayResource(TEXT("IDR_WAV_BTONE2"));
			break;
		case BM_AUX:
			PlayResource(TEXT("IDR_WAV_BTONE2"));
			break;
		case BM_TSK:
			PlayResource(TEXT("IDR_WAV_BTONE4"));
			break;
		case BM_ALT:
			PlayResource(TEXT("IDR_WAV_BTONE4"));
			break;
		case BM_SYS:
			PlayResource(TEXT("IDR_WAV_BTONE5"));
			break;
		case BM_CUS2:
			PlayResource(TEXT("IDR_WAV_BTONE5"));
			break;
		case BM_CUS3:
			PlayResource(TEXT("IDR_WAV_BTONE6"));
			break;
		case BM_CUS:
			PlayResource(TEXT("IDR_WAV_BTONE6"));
			break;
		case BM_TRM:
		default:
			PlayResource(TEXT("IDR_WAV_CLICK"));
			break;
	}
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
			break;
		case MSM_AIRSPACES:
			LKevent=LKEVENT_NEWRUN;
			break;
		case MSM_COMMON:
			LKForceDoCommon=true;
			LKevent=LKEVENT_NEWRUN;
			break;
		case MSM_RECENT:
			LKForceDoRecent=true;
			LKevent=LKEVENT_NEWRUN;
			break;
		case MSM_VISUALGLIDE:
			// direction sorting! It is set by LKInit, also.
			// It should not be necessary, unless someone changes it after init.
			SortedMode[MapSpaceMode]=2;
			LKForceDoNearest=true;
			LKevent=LKEVENT_NEWRUN;
			break;
		default:
			LKevent=LKEVENT_NEWRUN;
			break;
	}
	// Resetting Selected are needed if we want that each nearest page open
	// with page 1 row 1. Otherwise it is remembered last status, which can be
	// a problem maybe. Resetting Selected for non-nearest is not a problem.
	SelectedPage[MapSpaceMode]=0;
	SelectedRaw[MapSpaceMode]=0;
	MapSpaceMode=i;

	//
	// If we are showing special menus for special pages, here we disable them.
	//
	if (IsMultimapConfigShown &&
		MapSpaceMode!=MSM_MAPTRK &&
		MapSpaceMode!=MSM_MAPWPT &&
		MapSpaceMode!=MSM_MAPASP &&
		MapSpaceMode!=MSM_VISUALGLIDE) {
		InputEvents::setMode(TEXT("default"));
	}
    main_window->UnGhost();
}


// We assume that at least ConfBB[1] will be ON!
// We cannot have all OFF!
void BottomBarChange(bool advance) {

  short wanted;
  if (!advance) goto bbc_previous;

  wanted=BottomMode+1;
  while (true) {
	if (wanted >BM_LAST) {
		if ( ConfBB[BM_TRM] || (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING) && ConfBB0Auto!=BBSM_MANUAL)) {
			wanted=BM_TRM;
			break;
		} else {
			wanted=BM_FIRST;
			continue;
		}
	}
	if (ConfBB[wanted]) break;
	wanted++;
  } // while

  BottomMode=wanted;
  return;

bbc_previous:
  wanted=BottomMode-1;
  while (true) {
	if (wanted == BM_TRM) {
		if ( ConfBB[BM_TRM] || (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING) && ConfBB0Auto!=BBSM_MANUAL)) {
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
  } // while

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
