/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
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

extern void ShowMenu();

unsigned int CustomKeyLabel[(CustomKeyMode_t)ckTOP];

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
				DoStatusMessage(MsgToken(82));
			} else {
				GotoWaypoint(HomeWaypoint);
			}
		} else
	// LKTOKEN  _@M465_ = "No Home to go!"
			DoStatusMessage(MsgToken(465));
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
			DoStatusMessage(MsgToken(922)); // NOT FLYING
		} else {
			if (MessageBoxX(MsgToken(1754), _T(""), mbYesNo) == IdYes) {
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
		if(devA() && devA()->Config) {
			devA()->Config(devA());
		}
		return true;
	case ckDeviceB:
		if(devB() && devB()->Config) {
			devB()->Config(devB());
		}
		return true;
        case ckDeviceC:
                if(devC() && devC()->Config) {
                        devC()->Config(devC());
                }
                return true;
        case ckDeviceD:
                if(devD() && devD()->Config) {
                        devD()->Config(devD());
                }
                return true;
        case ckDeviceE:
                if(devE() && devE()->Config) {
                        devE()->Config(devE());
                }
                return true;
        case ckDeviceF:
                if(devF() && devF()->Config) {
                        devF()->Config(devF());
                }
                return true;
	case ckResetOdometer:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		if (MessageBoxX(MsgToken(2229), _T(""), mbYesNo) == IdYes) {
			LKSW_ResetOdometer=true;
		}
		return true;
	case ckForceLanding:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		if ( !CALCULATED_INFO.Flying ) {
			DoStatusMessage(MsgToken(922)); // NOT FLYING
		} else {
			if ( (GPS_INFO.Speed > TakeOffSpeedThreshold) && (!GPS_INFO.NAVWarning) ) {
				DoStatusMessage(MsgToken(1799)); // STOP MOVING!
			} else {
				if (MessageBoxX(MsgToken(2230), _T(""), mbYesNo) == IdYes) {
					LKSW_ForceLanding=true;
				}
			}
		}
		return true;
	case ckResetTripComputer:
		PlayResource(TEXT("IDR_WAV_CLICK"));
		if (MessageBoxX(MsgToken(2236), _T(""), mbYesNo) == IdYes) {
			LKSW_ResetTripComputer=true;
		}
		return true;
	case ckSonarToggle:
		SonarWarning = !SonarWarning;
		TCHAR sonarmsg[60];
		_stprintf(sonarmsg,_T("%s "),MsgToken(1293)); // SONAR
		if (SonarWarning)
			_tcscat(sonarmsg,MsgToken(1643)); // ENABLED
		else
			_tcscat(sonarmsg,MsgToken(1600)); // DISABLED
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
            case TRACKUP     : _stprintf(MapOrientMsg,_T("%s"),MsgToken(737)) ; break;  // _@M737_ "Track up"
            case NORTHUP     : _stprintf(MapOrientMsg,_T("%s"),MsgToken(483)) ; break;  // _@M483_ "North up"
            case NORTHCIRCLE : _stprintf(MapOrientMsg,_T("%s"),MsgToken(482)) ; break;  // _@M482_ "North circling"
            case TARGETCIRCLE: _stprintf(MapOrientMsg,_T("%s"),MsgToken(682)) ; break;  // _@M682_ "Target circling"  _@M485_ "NorthUp above "
            case NORTHTRACK  : _stprintf(MapOrientMsg,_T("%s"),MsgToken(484)) ; break;  // _@M484_ "North/track"
            case NORTHSMART  : _stprintf(MapOrientMsg,_T("%s"),MsgToken(481)) ; break;  // _@M481_ "North Smart"
            case TARGETUP    : _stprintf(MapOrientMsg,_T("%s"),MsgToken(2349)); break;  // _@M2349_"Target up"

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
        dlgAirspaceSelect();
        return true;
    case  ckRadioDlg:
        PlayResource(TEXT("IDR_WAV_CLICK"));
        dlgRadioSettingsShowModal();
        return true;
	default:
		DoStatusMessage(_T("ERR-726 INVALID CUSTOMKEY"));
		StartupStore(_T("... ERR-726 INVALID CUSTOMKEY=%d\n"),ckeymode);
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
CustomKeyLabel[0]=2200;		// Disabled  - note: never shown since label not printed at all
CustomKeyLabel[1]=2201;		// Menu
CustomKeyLabel[2]=2202;		// Page Back
CustomKeyLabel[3]=2203;		// Toggle Map<>current page
CustomKeyLabel[4]=2204;		// Toggle Map<>Landables
CustomKeyLabel[5]=2205;		// Landables
CustomKeyLabel[6]=2206;		// Toggle Map<>Commons
CustomKeyLabel[7]=2207;		// Commons
CustomKeyLabel[8]=2208;		// Toggle Map<>Traffic
CustomKeyLabel[9]=2209;		// "Traffic"
CustomKeyLabel[10]=2036;	// invert text
CustomKeyLabel[11]=2071;	// truewind calc
CustomKeyLabel[12]=2079;	// overlays (on/off missing)
CustomKeyLabel[13]=2210;	// auto zoom
CustomKeyLabel[14]=982; 	// (reserved) ActiveMap On/Off (UNUSED 2044)
CustomKeyLabel[15]=2070;	// Location marker
CustomKeyLabel[16]=2024;	// Time gates
CustomKeyLabel[17]=2211;	// Thermal booster
CustomKeyLabel[18]=2212;	// goto home
CustomKeyLabel[19]=2213;	// zoom out 20  panorama trigger
CustomKeyLabel[20]=2214;	// multitarget rotate
CustomKeyLabel[21]=2025;	// multitarget menu
CustomKeyLabel[22]=2021;	// team code
CustomKeyLabel[23]=2215;	// use hbar
CustomKeyLabel[24]=2042;	// basic setup
CustomKeyLabel[25]=2216;	// SIM MENU
CustomKeyLabel[26]=2217;	// airspace analysis
CustomKeyLabel[27]=2218;	// toggle map Airspace
CustomKeyLabel[28]=2001;	// zoom in
CustomKeyLabel[29]=2002;	// zoom out
CustomKeyLabel[30]=2219;	// zoom in more
CustomKeyLabel[31]=2220;	// zoom out more
CustomKeyLabel[32]=2221;	// route optimize
CustomKeyLabel[33]=966;		// LOCK SCREEN
CustomKeyLabel[34]=2058;	// Oracle
CustomKeyLabel[35]=2115;	// TEnergy
CustomKeyLabel[36]=2063;	// Notepad
CustomKeyLabel[37]=2223;	// Change+ terrain colors
CustomKeyLabel[38]=2060;	// Nearest airspace
CustomKeyLabel[39]=2222;	// OLC analysis
CustomKeyLabel[40]=2224;	// Change- terrain colors
CustomKeyLabel[41]=2225;	// free flight
CustomKeyLabel[42]=2131;	// custom menu
CustomKeyLabel[43]=2013;	// task calc
CustomKeyLabel[44]=2020;	// task target
CustomKeyLabel[45]=2226;	// Arm toggle
CustomKeyLabel[46]=2064;	// Message Repeat
CustomKeyLabel[47]=2015;	// Waypoint lookup
CustomKeyLabel[48]=2082;	// PAN
CustomKeyLabel[49]=2227;	// Toggle windrose
CustomKeyLabel[50]=2228;	// Flarm radar
CustomKeyLabel[51]=2143;	// Device A Config
CustomKeyLabel[52]=2144;	// Device B Config
CustomKeyLabel[53]=2229;	// Reset Odometer
CustomKeyLabel[54]=2230;	// Force landing
CustomKeyLabel[55]=2236;	// ResetTripComputer
CustomKeyLabel[56]=2237;	// Sonar toggle
CustomKeyLabel[57]=2246;	// Reset view
CustomKeyLabel[58]=2038;	// Map Orientation
CustomKeyLabel[59]=928;		// Restarting Comm Ports
CustomKeyLabel[60]=2249;	// DspMode
CustomKeyLabel[61]=2390;	// Toggle Draw XC
CustomKeyLabel[62]=2337;	// Airspace lookup
CustomKeyLabel[63]=2393;        // Device C Config
CustomKeyLabel[64]=2394;        // Device D Config
CustomKeyLabel[65]=2395;        // Device E Config
CustomKeyLabel[66]=2396;        // Device F Config
CustomKeyLabel[67]=2307;        // Radio Settings _@M2307_ "Radio Settings"

static_assert(67 < std::size(CustomKeyLabel), "invalid CustomKeyLabel array size");
}
