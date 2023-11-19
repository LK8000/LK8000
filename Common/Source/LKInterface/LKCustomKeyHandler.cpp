/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "InputEvents.h"
#include "LKInterface.h"
#include "Calculations2.h"
#include "Multimap.h"
#include "Dialogs.h"
#include "Sideview.h"
#include "Sound/Sound.h"
#include "WindowControls.h"
#include "Dialogs/dlgMacCready.h"

extern void ShowMenu();

// handle custom keys. Input: key pressed (center, left etc.)
// Returns true if handled successfully, false if not
//
// Passthrough mode for keys>=1000 (custom menu keys)
//
bool CustomKeyHandler(unsigned key) {

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
		DoStatusMessage(_T("ERR-725 UNKNOWN CUSTOMKEY"));
		return false;
		break;
  }

passthrough:

  switch(ckeymode) {
	case ckDisabled:
		break;
	case ckZoomIn:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		MapWindow::zoom.EventScaleZoom(1);
		return true;
		break;
	case ckZoomInMore:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		MapWindow::zoom.EventScaleZoom(2);
		return true;
		break;
	case ckZoomOut:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		MapWindow::zoom.EventScaleZoom(-1);
		return true;
		break;
	case ckZoomOutMore:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		MapWindow::zoom.EventScaleZoom(-2);
		return true;
		break;
	case ckMenu:
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
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::setMode(_T("TrueWind"));
		return true;

	case ckTeamCode:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::eventSetup(_T("Teamcode"));
		return true;

	case ckToggleOverlays:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		ToggleMultimapOverlays();
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
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::eventInvertColor(NULL);
		return true;
	case ckTimeGates:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::eventTimeGates(NULL);
		return true;
	case ckMarkLocation:
		InputEvents::eventMarkLocation(_T(""));
		return true;
	case ckAutoZoom:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::eventZoom(_T("auto toggle"));
		InputEvents::eventZoom(_T("auto show"));
		return true;
	case ckActiveMap:
                // NO MORE USED (reserved)
		return true;
	case ckBooster:
		DoStatusMessage(_T("FEEL THE THERMAL"));
		LKSound(_T("LK_BOOSTER.WAV"));
		return true;
	case ckGoHome:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		if (ValidWayPoint(HomeWaypoint)) {
			if ( (ValidTaskPoint(ActiveTaskPoint)) && (Task[ActiveTaskPoint].Index == HomeWaypoint )) {
	// LKTOKEN  _@M82_ = "Already going home"
				DoStatusMessage(MsgToken<82>());
			} else {
				GotoWaypoint(HomeWaypoint);
			}
		} else
	// LKTOKEN  _@M465_ = "No Home to go!"
			DoStatusMessage(MsgToken<465>());
		return true;
	case ckPanorama:
		if (PGZoomTrigger==false)
			PGZoomTrigger=true;
		else
			LastZoomTrigger=0;
		PlayResource(TEXT("IDR_WAV_CLICK"));
		return true;

	case ckMultitargetRotate:
		RotateOvertarget();
		MapWindow::RefreshMap();
		return true;

	case ckMultitargetMenu:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::setMode(_T("MTarget"));
		return true;
	case ckBaroToggle:
		ToggleBaroAltitude();
		return true;
	case ckBasicSetup:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::eventSetup(_T("Basic"));
		return true;
	case ckSimMenu:
		PlayResource(TEXT("IDR_WAV_CLICK"));
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
		SetModeType(LKMODE_MAP,MP_MAPASP);
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;
	case ckOptimizeRoute:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		TskOptimizeRoute=!TskOptimizeRoute;
		if(gTaskType==TSK_GP) {
            ClearOptimizedTargetPos();
		}
		return true;
	case ckLockScreen:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::eventService(_T("LOCKMODE"));
		return true;
	case ckWhereAmI:
		// no sound here, chime is played by service event
		InputEvents::eventService(_T("ORACLE"));
		return true;
	case ckUseTotalEnergy:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::eventService(_T("TOTALEN"));
		return true;
	case ckNotepad:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::eventChecklist(_T(""));
		return true;
	case ckTerrainColors:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::eventService(_T("TERRCOL"));
		return true;
	case ckNearestAirspace:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::eventNearestAirspaceDetails(NULL);
		return true;
	case ckOlcAnalysis:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::eventSetup(_T("OlcAnalysis"));
		return true;
	case ckTerrainColorsBack:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::eventService(_T("TERRCOLBACK"));
		return true;
	case ckForceFreeFlightRestart:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		if (!CALCULATED_INFO.Flying) {
			DoStatusMessage(MsgToken<922>()); // NOT FLYING
		} else {
			if (MessageBoxX(MsgToken<1754>(), _T(""), mbYesNo) == IdYes) {
				LKSW_ForceFreeFlightRestart=true;
			}
		}
		return true;
	case ckCustomMenu1:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::eventMode(_T("MYMODE"));
		return true;
	case ckTaskCalc:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::eventCalculator(NULL);
		return true;
	case ckTaskTarget:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::eventSetup(_T("Target"));
		return true;
	case ckArmAdvance:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::eventArmAdvance(_T("toggle"));
		InputEvents::eventArmAdvance(_T("show"));
		return true;

	case ckMessageRepeat:
		InputEvents::eventRepeatStatusMessage(NULL);
                return true;

	case ckWaypointLookup:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::eventWaypointDetails(_T("select"));
		return true;

	case ckPan:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::eventPan(_T("toggle"));
		return true;

	case ckWindRose:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		UseWindRose=!UseWindRose;
		return true;

	case ckFlarmRadar:
		SetModeType(LKMODE_MAP,MP_RADAR);
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;
	case ckDeviceA:
		return devConfig<0>();
	case ckDeviceB:
		return devConfig<1>();
	case ckDeviceC:
		return devConfig<2>();
	case ckDeviceD:
		return devConfig<3>();
	case ckDeviceE:
		return devConfig<4>();
	case ckDeviceF:
		return devConfig<5>();
	case ckResetOdometer:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		if (MessageBoxX(MsgToken<2229>(), _T(""), mbYesNo) == IdYes) {
			LKSW_ResetOdometer=true;
		}
		return true;
	case ckForceLanding:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		if ( !CALCULATED_INFO.Flying ) {
			DoStatusMessage(MsgToken<922>()); // NOT FLYING
		} else {
			if ( (GPS_INFO.Speed > TakeOffSpeedThreshold) && (!GPS_INFO.NAVWarning) ) {
				DoStatusMessage(MsgToken<1799>()); // STOP MOVING!
			} else {
				if (MessageBoxX(MsgToken<2230>(), _T(""), mbYesNo) == IdYes) {
					LKSW_ForceLanding=true;
				}
			}
		}
		return true;
	case ckResetTripComputer:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		if (MessageBoxX(MsgToken<2236>(), _T(""), mbYesNo) == IdYes) {
			LKSW_ResetTripComputer=true;
		}
		return true;
	case ckSonarToggle:
		SonarWarning = !SonarWarning;
		TCHAR sonarmsg[60];
		_stprintf(sonarmsg,_T("%s "),MsgToken<1293>()); // SONAR
		if (SonarWarning)
			_tcscat(sonarmsg,MsgToken<1643>()); // ENABLED
		else
			_tcscat(sonarmsg,MsgToken<1600>()); // DISABLED
		DoStatusMessage(sonarmsg,NULL,false);
        if (SonarWarning)
            LKSound(TEXT("LK_TONEUP.WAV"));
        else
            LKSound(TEXT("LK_TONEDOWN.WAV"));
		return true;
    case ckDrawXCToggle:
      Flags_DrawXC = !Flags_DrawXC;
      if (EnableSoundModes) {
        if (!Flags_DrawXC)
          LKSound(TEXT("LK_TONEUP.WAV"));
        else
          LKSound(TEXT("LK_TONEDOWN.WAV"));
      }
      return true;
	case ckResetView:
		ModeType[LKMODE_MAP]    =       MP_MOVING;
		ModeType[LKMODE_INFOMODE]=      IM_CRUISE;
		ModeType[LKMODE_WP]     =       WP_AIRPORTS;
		ModeType[LKMODE_NAV]    =       NV_COMMONS;
		ModeType[LKMODE_TRF]    =       TF_LIST;

		SetModeType(LKMODE_MAP,MP_MOVING);
		MapWindow::RefreshMap();
		SoundModeIndex();

		return true;

	case  ckMapOrient:
		PlayResource(TEXT("IDR_WAV_CLICK"));

		TCHAR MapOrientMsg[60];

	    if  (MapSpaceMode==MSM_MAP)
	    {
	      DisplayOrientation++;
	      if(DisplayOrientation > TARGETUP)
		DisplayOrientation = 0;
	      MapWindow::SetAutoOrientation(); // 101008 reset it
	      switch(DisplayOrientation)
	      {
            case TRACKUP     : _stprintf(MapOrientMsg,_T("%s"),MsgToken<737>()) ; break;  // _@M737_ "Track up"
            case NORTHUP     : _stprintf(MapOrientMsg,_T("%s"),MsgToken<483>()) ; break;  // _@M483_ "North up"
            case NORTHCIRCLE : _stprintf(MapOrientMsg,_T("%s"),MsgToken<482>()) ; break;  // _@M482_ "North circling"
            case TARGETCIRCLE: _stprintf(MapOrientMsg,_T("%s"),MsgToken<682>()) ; break;  // _@M682_ "Target circling"  _@M485_ "NorthUp above "
            case NORTHTRACK  : _stprintf(MapOrientMsg,_T("%s"),MsgToken<484>()) ; break;  // _@M484_ "North/track"
            case NORTHSMART  : _stprintf(MapOrientMsg,_T("%s"),MsgToken<481>()) ; break;  // _@M481_ "North Smart"
            case TARGETUP    : _stprintf(MapOrientMsg,_T("%s"),MsgToken<2349>()); break;  // _@M2349_"Target up"

	      }
	      DoStatusMessage(MapOrientMsg,NULL,false);
	    }
	    else
	    {
		  SetMMNorthUp(GetSideviewPage(), (GetMMNorthUp(GetSideviewPage())+1)%2);
	    }

		return true;
    case ckResetComm:
		PlayResource(TEXT("IDR_WAV_CLICK"));
        InputEvents::eventRestartCommPorts(NULL);
        return true;

    case ckDspMode:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		InputEvents::setMode(_T("Display3"));
		return true;

    case ckAirspaceLookup:
        PlayResource(TEXT("IDR_WAV_CLICK"));
        dlgSelectAirspace();
        return true;
    case  ckRadioDlg:
        PlayResource(TEXT("IDR_WAV_CLICK"));
        dlgRadioSettingsShowModal();
        return true;
	case ckMCSetting:
        PlayResource(TEXT("IDR_WAV_CLICK"));
        dlgMacCready::DoModal();
        return true;
	default:
		DoStatusMessage(_T("ERR-726 INVALID CUSTOMKEY"));
		StartupStore(_T("... ERR-726 INVALID CUSTOMKEY=%d\n"),ckeymode);
		break;
  }

  return false;

}

namespace {
//
// Some labels already exist for buttons. Some other are missing.
// We assign msg tokens in index array, since they are not in order.
// Order is strictly the one in Enums.h for customkeys
//
struct KeyLabel_t {
	MsgToken_t Name; // used by Config dialog
	MsgToken_t MenuLabel;
};

template<unsigned name_id, unsigned label_id>
KeyLabel_t KeyLabel() {
	return { MsgToken<name_id>, MsgToken<label_id> };
}

// Careful, order must respect the enum list in LKInterface.h CustomKeyMode_t

const KeyLabel_t _CustomKeyLabel[] = {
	KeyLabel< 239, 2200>(),	// { Disabled, ---- }  - note: never shown since label not printed at all
	KeyLabel< 435, 2201>(),	// { Menu, Menu }
	KeyLabel< 517, 2202>(),	// { Page Back, Page\nBack }
	KeyLabel< 725, 2203>(),	// { Toggle Map<>current page, Toggle\nMap }
	KeyLabel< 723, 2204>(),	// { Toggle Map<>Landables, Toggle\nLandb }
	KeyLabel< 385, 2205>(),	// { Landables, Landables }
	KeyLabel< 722, 2206>(),	// { Toggle Map<>Commons, Toggle\nCommon }
	KeyLabel< 192, 2207>(),	// { Commons, Commons }
	KeyLabel< 724, 2208>(),	// { Toggle Map<>Traffic, Toggle\nTraffic }
	KeyLabel< 738, 2209>(),	// { Traffic, Traffic }
	KeyLabel< 363, 2036>(),	// { Invert colors, invert text }
	KeyLabel<1532, 2071>(),	// { TrueWind, TrueWind\nCalc }
	KeyLabel< 726, 2079>(),	// { Overlays toggle, Overlays }
	KeyLabel<1533, 2210>(),	// { AutoZoom On/Off, Auto\nZoom }
	KeyLabel< 982,  982>(),	// (reserved)
	KeyLabel< 426, 2070>(),	// { Mark Location, Location\nMarker }
	KeyLabel< 513, 2024>(),	// { Time Gates, Time\nGates }
	KeyLabel<1535, 2211>(),	// { Thermal Booster, Therm\nSound }
	KeyLabel< 329, 2212>(),	// { Goto Home, Goto\nHome }
	KeyLabel< 519, 2213>(),	// { Panorama trigger, Zoom\nOut20 }
	KeyLabel< 448, 2214>(),	// { Multitarget rotate, Multi\nTarg+ } 
	KeyLabel< 447, 2025>(),	// { Multitarget menu, Multi\nTarget }
	KeyLabel< 700, 2021>(),	// { Team code, Team\nCode }
	KeyLabel< 767, 2215>(),	// { Use HBar on/off, Use\nHBAR }
	KeyLabel< 130, 2042>(),	// { Basic Setup menu, Setup\nBasic } 
	KeyLabel<1536, 2216>(),	// { SIMulation menu, SIM\nMENU }
	KeyLabel<1652, 2217>(),	// { Airspace Analysis, Airsp\nAnalys }
	KeyLabel<1653, 2218>(),	// { Toggle Map<>Airspace infopage, Toggle\nAirsp }
	KeyLabel<1657, 2001>(),	// { Zoom In, Zoom\nin }
	KeyLabel<1658, 2002>(),	// { Zoom Out, Zoom\nout }
	KeyLabel<1659, 2219>(),	// { Zoom In More, Zoom\nin++ }
	KeyLabel<1660, 2220>(),	// { Zoom Out More, Zoom\nout++ }
	KeyLabel<1687, 2221>(),	// { Optimized route toggle, Route\nOptim }
	KeyLabel<1688,  966>(),	// { Screen Lock, LOCK\nSCREEN }
	KeyLabel<1689, 2058>(),	// { Oracle, Oracle }
	KeyLabel<1666, 2115>(),	// { TotalEnergy toggle, TEnergy }
	KeyLabel<2063, 2063>(),	// Notepad
	KeyLabel<1693, 2223>(),	// { Change+ Terrain Colors, Color+\nterrain }
	KeyLabel< 871, 2060>(),	// { Nearest airspace, Nearest\nAirspace }
	KeyLabel<1740, 2222>(),	// { OLC analysis, OLC\nAnalys }
	KeyLabel<1774, 2224>(),	// { Change- Terrain Colors, Color-\nterrain }
	KeyLabel<1754, 2225>(),	// { Free Flight start, Free\nFlight }
	KeyLabel<1787, 2131>(),	// { Custom Menu, Custom\nMenu }
	KeyLabel< 685, 2013>(),	// { Task Calculator, Task\nCalc }
	KeyLabel< 684, 2020>(),	// { Target, Target }
	KeyLabel<1791, 2226>(),	// { Arm toggle advance, Arm toggle }
	KeyLabel<2064, 2064>(),	// Message Repeat
	KeyLabel<2015, 2015>(),	// Waypoint lookup
	KeyLabel<2082, 2082>(),	// PAN
	KeyLabel<2227, 2227>(),	// Toggle windrose
	KeyLabel<2228, 2228>(),	// Flarm radar
	KeyLabel<2143, 2143>(),	// Device A Config
	KeyLabel<2144, 2144>(),	// Device B Config
	KeyLabel<2229, 2229>(),	// Reset Odometer
	KeyLabel<2230, 2230>(),	// Force landing
	KeyLabel<2236, 2236>(),	// ResetTripComputer
	KeyLabel<2237, 2237>(),	// Sonar toggle
	KeyLabel<2246, 2246>(),	// Reset view
	KeyLabel<2038, 2038>(),	// Map Orientation
	KeyLabel< 928,  928>(),	// Restarting Comm Ports
	KeyLabel<2249, 2249>(),	// DspMode
	KeyLabel<2390, 2390>(),	// Toggle Draw XC
	KeyLabel<2337, 2337>(),	// Airspace lookup
	KeyLabel<2393, 2393>(),	// Device C Config
	KeyLabel<2394, 2394>(),	// Device D Config
	KeyLabel<2395, 2395>(),	// Device E Config
	KeyLabel<2396, 2396>(),	// Device F Config
	KeyLabel<2307, 2307>(),	// Radio Settings
	KeyLabel< 844,  844>()    // MacCready setting
};

static_assert(ckTOP == std::size(_CustomKeyLabel), "invalid _CustomKeyLabel array size");

} // namespace


const TCHAR* CustomKeyLabel(unsigned key) {
	if (key < std::size(_CustomKeyLabel)) {
		return _CustomKeyLabel[key].MenuLabel();
	}
	return nullptr;
}

void AddCustomKeyList(WndForm* pForm, const TCHAR* WndName, unsigned value) {
	auto pWnd = static_cast<WndProperty*>(pForm->FindByName(WndName));
	if (pWnd) {
		DataField* dfe = pWnd->GetDataField();
		if (dfe->getCount() == 0) {
			for (auto& item : _CustomKeyLabel) {
				dfe->addEnumText(item.Name());
			}
			dfe->Sort();
		}
		dfe->Set(value);
		pWnd->RefreshDisplay();
	}
}

template<typename TypeT>
static void GetCustomKey(WndForm* pForm, const TCHAR* WndName, TypeT& value) {
	auto pWnd = static_cast<WndProperty*>(pForm->FindByName(WndName));
	if (pWnd) {
		value = pWnd->GetDataField()->GetAsInteger();
	}
}

void GetCustomKey(WndForm* pForm, const TCHAR* WndName, unsigned short& value) {
	GetCustomKey<unsigned short>(pForm, WndName, value);
}

void GetCustomKey(WndForm* pForm, const TCHAR* WndName, int& value) {
	GetCustomKey<int>(pForm, WndName, value);
}
