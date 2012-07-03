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

CustomKeyLabel_t CustomKeyLabel[(CustomKeyMode_t)ckTOP];

// handle custom keys. Input: key pressed (center, left etc.)
// Returns true if handled successfully, false if not
//
// Passthrough mode for keys>=1000 (custom menu keys)
//
bool CustomKeyHandler(const int key) {

  int ckeymode;
  static bool doinit=true;
  static int oldModeIndex;

  if (doinit) {
	oldModeIndex=LKMODE_INFOMODE;;
	doinit=false;
  }

  if (key>=1000) {
	ckeymode=key-1000;
	LKASSERT((ckeymode>=0 && ckeymode<ckTOP));
	goto passthrough;
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

passthrough:

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
		if (CALCULATED_INFO.Flying == FALSE) {
			DoStatusMessage(_T("NO TAKEOFF!"));
		} else {
			if (MessageBoxX(hWndMapWindow, MsgToken(1754), _T(""), MB_YESNO|MB_ICONQUESTION) == IDYES) {
				LKSW_ForceFreeFlightRestart=true;
			}
		}
		return true;
	case ckCustomMenu1:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventMode(_T("MYMODE"));
		return true;
	case ckTaskCalc:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventCalculator(NULL);
		return true;
	case ckTaskTarget:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventSetup(_T("Target"));
		return true;
	case ckArmAdvance:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventArmAdvance(_T("toggle"));
		InputEvents::eventArmAdvance(_T("show"));
		return true;

	case ckMessageRepeat:
		InputEvents::eventRepeatStatusMessage(NULL);
                return true;
		
	case ckWaypointLookup:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventWaypointDetails(_T("select"));
		return true;

	case ckPan:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventPan(_T("toggle"));
		return true;

	case ckWindRose:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		UseWindRose=!UseWindRose;
		return true;

	case ckFlarmRadar:
		#ifndef DISABLEAUDIO
		//if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		if (ModeIndex==LKMODE_MAP)
			SetModeType(LKMODE_TRF,IM_RADAR);
		else
			SetModeIndex(LKMODE_MAP);
		MapWindow::RefreshMap();
		SoundModeIndex();

		return true;
	case ckDeviceA:
		if(devA() && devA()->Config) {
			devA()->Config();
		}
		return true;
	case ckDeviceB:
		if(devB() && devB()->Config) {
			devB()->Config();
		}
		return true;
	default:
		DoStatusMessage(_T("ERR-726 INVALID CUSTOMKEY"));
		FailStore(_T("ERR-726 INVALID CUSTOMKEY=%d"),ckeymode);
		break;
  }

  return false;

}


void InitCustomKeys(void) {

//
// Some labels already exist for buttons. Some other are missing.
// We assign msg tokens in index array, since they are not in order.
// Order is strictly the one in Enums.h for customkeys
//
CustomKeyLabel[0]= (CustomKeyLabel_t){2200, 239};	// Disabled  - note: never shown since label not printed at all
CustomKeyLabel[1]= (CustomKeyLabel_t){2201, 435};	// Menu
CustomKeyLabel[2]= (CustomKeyLabel_t){2202, 517};	// Page Back
CustomKeyLabel[3]= (CustomKeyLabel_t){2203, 725};	// Toggle Map<>current page
CustomKeyLabel[4]= (CustomKeyLabel_t){2204, 723};	// Toggle Map<>Landables
CustomKeyLabel[5]= (CustomKeyLabel_t){2205, 385};	// Landables
CustomKeyLabel[6]= (CustomKeyLabel_t){2206, 722};	// Toggle Map<>Commons
CustomKeyLabel[7]= (CustomKeyLabel_t){2207, 192};	// Commons
CustomKeyLabel[8]= (CustomKeyLabel_t){2208, 724};	// Toggle Map<>Traffic
CustomKeyLabel[9]= (CustomKeyLabel_t){2209, 738};	// "Traffic"
CustomKeyLabel[10]= (CustomKeyLabel_t){2036, 363};	// invert text
CustomKeyLabel[11]= (CustomKeyLabel_t){2071, 2137};	// truewind calc
CustomKeyLabel[12]= (CustomKeyLabel_t){2079, 726};	// overlays (on/off missing)
CustomKeyLabel[13]= (CustomKeyLabel_t){2210, 2138};	// auto zoom
CustomKeyLabel[14]= (CustomKeyLabel_t){2044, 2139};	// ActiveMap On/Off
CustomKeyLabel[15]= (CustomKeyLabel_t){2070, 426};	// Location marker
CustomKeyLabel[16]= (CustomKeyLabel_t){2024, 2140};	// Time gates
CustomKeyLabel[17]= (CustomKeyLabel_t){2211, 2141};	// Thermal booster
CustomKeyLabel[18]= (CustomKeyLabel_t){2212, 329};	// goto home
CustomKeyLabel[19]= (CustomKeyLabel_t){2213, 519};	// zoom out 20  panorama trigger
CustomKeyLabel[20]= (CustomKeyLabel_t){2214, 448};	// multitarget rotate
CustomKeyLabel[21]= (CustomKeyLabel_t){2025, 447};	// multitarget menu
CustomKeyLabel[22]= (CustomKeyLabel_t){2021, 700};	// team code
CustomKeyLabel[23]= (CustomKeyLabel_t){2215, 767};	// use hbar
CustomKeyLabel[24]= (CustomKeyLabel_t){2042, 130};	// basic setup
CustomKeyLabel[25]= (CustomKeyLabel_t){2216, 2142};	// SIM MENU
CustomKeyLabel[26]= (CustomKeyLabel_t){2217, 1652};	// airspace analysis
CustomKeyLabel[27]= (CustomKeyLabel_t){2218, 1653};	// toggle map Airspace
CustomKeyLabel[28]= (CustomKeyLabel_t){2001, 1657};	// zoom in
CustomKeyLabel[29]= (CustomKeyLabel_t){2002, 1658};	// zoom out
CustomKeyLabel[30]= (CustomKeyLabel_t){2219, 1659};	// zoom in more
CustomKeyLabel[31]= (CustomKeyLabel_t){2220, 1660};	// zoom out more
CustomKeyLabel[32]= (CustomKeyLabel_t){2221, 1687};	// route optimize
CustomKeyLabel[33]= (CustomKeyLabel_t){966, 1688};		// LOCK SCREEN
CustomKeyLabel[34]= (CustomKeyLabel_t){2058, 1689};	// Oracle
CustomKeyLabel[35]= (CustomKeyLabel_t){2115, 1666};	// TEnergy
CustomKeyLabel[36]= (CustomKeyLabel_t){2063, 2063};	// Notepad
CustomKeyLabel[37]= (CustomKeyLabel_t){2223, 1693};	// Change+ terrain colors
CustomKeyLabel[38]= (CustomKeyLabel_t){2060, 871};	// Nearest airspace
CustomKeyLabel[39]= (CustomKeyLabel_t){2222, 1740};	// OLC analysis
CustomKeyLabel[40]= (CustomKeyLabel_t){2224, 1774};	// Change- terrain colors
CustomKeyLabel[41]= (CustomKeyLabel_t){2225, 1754};	// free flight
CustomKeyLabel[42]= (CustomKeyLabel_t){2131, 1787};	// custom menu
CustomKeyLabel[43]= (CustomKeyLabel_t){2013, 685};	// task calc
CustomKeyLabel[44]= (CustomKeyLabel_t){2020, 684};	// task target
CustomKeyLabel[45]= (CustomKeyLabel_t){2226, 1791};	// Arm toggle
CustomKeyLabel[46]= (CustomKeyLabel_t){2064, 2064};	// Message Repeat
CustomKeyLabel[47]= (CustomKeyLabel_t){2015, 2015};	// Waypoint lookup
CustomKeyLabel[48]= (CustomKeyLabel_t){2082, 2082};	// PAN
CustomKeyLabel[49]= (CustomKeyLabel_t){2227, 2227};	// Toggle windrose
CustomKeyLabel[50]= (CustomKeyLabel_t){2228, 2228};	// Flarm radar
CustomKeyLabel[51]= (CustomKeyLabel_t){2134, 2143};	// Device A Config
CustomKeyLabel[52]= (CustomKeyLabel_t){2135, 2144};	// Device B Config
}


